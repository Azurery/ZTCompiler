#include "Code_generation.h"

namespace ztCompiler {
	/*
	|Microsoft		|							|
	|x64 calling	|							|
	|convention[14]	|Windows					|RCX/XMM0,		32 bytes shadow space on stack.
	|				|(Microsoft Visual C++,		|RDX/XMM1,		The specified 8 registers can only be
	|				|GCC,						|R8/XMM2,		used for parameters 1 through 4.
	|				|Intel C++ Compiler,		|R9/XMM3		For C++ classes, the hidden "this"
	|				|Delphi),					|				parameter is the first parameter,
	|				|UEFI						|				and is passed in RCX [22].
	|				|							|
	|x86-64		    |Windows			        |RCX/XMM0,
	|	vectorcall	|(Microsoft Visual C++)		|RDX/XMM1,
	|				|							|R8/XMM2,
	|				|							|R9/XMM3 +
	|				|							|XMM0-XMM5/YMM0-YMM5
	|				|							|
	|System V		|							|
	|AMD64 ABI[19]	|Solaris,					|RDI, RSI, RDX,		Caller	Stack aligned on 16 bytes boundary.
	|				|Linux,						|RCX, R8, R9,		128 bytes red zone below stack.
	|				|BSD, OS X					|XMM0�C7				The kernel interface uses RDI,
	|				|GCC, Intel C++ Compiler)	|					RSI, RDX, R10, R8 and R9
	*/


	static std::vector<const char*> registers = {
		"%rdi",	"%rsi",	"rdx",
		"rcx",	"r8",	"r9"
	};

	static std::vector<const char*> sse_registers{
		"xmm0",	"xmm1",	"xmm2",	"xmm3",
		"xmm4", "xmm5",	"xmm6",	"xmm7"
	};

	code_generator::Parameter_passing code_generator::classify(type* parameter_type_) {
		if (parameter_type_->is_integer() || parameter_type_->to_pointer_type()||parameter_type_->to_arithmetic_type()) {
			return Parameter_passing::INTEGER;
		}
		if (parameter_type_->to_arithmetic_type()) {
			auto type_ = parameter_type_->to_arithmetic_type();
			//SSE��INTEGERͬ����arithmetic type
			if (type_->tag_value() == static_cast<int>(TokenAttr::FLOAT) ||
				type_->tag_value() == static_cast<int>(TokenAttr::DOUBLE)) {
				return Parameter_passing::SSE;
			}
			if (type_->tag_value() == (static_cast<int>(TokenAttr::LONG) | static_cast<int>(TokenAttr::DOUBLE))) {
				return Parameter_passing::X87;
			}

			if (type_->tag_value()&static_cast<int>(TokenAttr::LONG) &&
				(type_->tag_value()&static_cast<int>(TokenAttr::DOUBLE))) {
				return Parameter_passing::COMPLEX_X87;
			}
			auto type_ = parameter_type_->to_struct_type();
			assert(type_);
			return Parameter_passing::MEMORY;

		}
	}

	void code_generator::emit(const std::string& instruction_) {
		fprintf(output_file_, "\t%s\n", instruction_.c_str());
	}

	void code_generator::emit(const std::string& instruction_, const labeled_statement* labeled_statement_) {
		emit(instruction_ + "\t" + labeled_statement_->labeled_wrapper());
	}
	
	/*
	�ú������ڻ�ȡװ��ָ���֧�ַ�����չ�����֣���64λ������
	movsbq		�����˷�����չ���ֽڴ��͵�����
	movswq		�����˷�����չ���ִ��͵�����
	movslq		�����˷�����չ��˫�ִ��͵�����
	*/
	std::string code_generator::get_load_instruction(int width_) {
		switch (width_) {
		case 1:	return "movsbq";
		case 2: return "movswq";
		case 4: return "movslq";
		case 8: return "mov";
		default:
			std::cerr<<"���ݴ�Сδ֪";
		}
	}
	//����ָ���width����ȡ��Ӧ�ļĴ���
	//%al %ax %eax %rax �Ĵ�����λ�����ε�������8λ��64λ
	std::string code_generator::get_register(int width_) {
		switch (width_) {
		case 1: return "%al";
		case 2: return "%ax";
		case 4: return "%eax";
		case 8: return "%rax";
		default:
			std::cerr << "���ݴ�Сδ֪";
		}
	}

	//���ڻ�ȡָ��
	/*����instruction��width��Ϊ��ָ����Ϻ�׺
	b����8�ֽڣ������֣�d����16�ֽڣ���˫��
	l����32�ֽڣ�q����64�ֽ�
	*/
	std::string code_generator::get_instruction(const std::string& instruction_, int width_) {
		switch (width_) {
		case 1: return instruction_ + "b";
		case 2: return instruction_ + "d";
		case 4: return instruction_ + "l";
		case 8: return instruction_ + "q";
		default:
			std::cerr << "���ݴ�Сδ֪";
		}
		return instruction_;
	}

	std::string code_generator::get_instruction(const std::string& instruction_, type* type_) {
		assert(type_->is_scalar());
		return get_instruction(instruction_, type_->width());
	}

	std::string code_generator::get_destination(int width_) {
		return get_register(width_);
	}

	std::string code_generator::get_source(int width_) {
		switch (width_) {
			
		}
	}

	/*	MOVSB:����һ���ֽڣ�֮��SI��DI(����ESI��EDI)��/��1 
		MOVSW:����һ���֣�֮��SI��DI(����ESI��EDI)��/��2 
		MOVSD:����һ��˫�֣�֮��SI��DI(����ESI��EDI)��/��4
		������ָ��������ݴ���ָ��,���Ǵ�Դ��ַ��Ŀ�ĵ�ַ��������
	*/
	//register_����Ϊ8bits
	int code_generator::push(const std::string& register_) {
		stack_position_ += 8;
		//����Ĵ���register_��x��ͷ����xmm0~xmm9����Ϊ64λ�Ĵ�����ʹ��movsd����
		//����ָ��
		auto mov_ = ((register_[1] == 'x') ? "movsd" : "movq");
		emit(mov_, register_, "");
		return stack_position_;
	}

	int code_generator::push(type* type_) {
		if (type_->is_float()) {
			return push("%xmm0");
		}else if (type_->is_scalar()) {
			return push("%rax");
		}else {
			stack_position_ -= type_->width();
			return stack_position_;
		}
	}

	int code_generator::pop(const std::string& register_) {
		auto mov_ = ((register_[1] == 'x') ? "movsd" : "movq");
		emit(mov_, register_, "");
		stack_position_ -= 8;
		return stack_position_;
	}

	void code_generator::generate_comma_operator(binary_expression* comma_) {
		visit_expression(comma_->lhs_wrapper());
		visit_expression(comma_->rhs_wrapper());
	}

	void code_generator::generate_mul_operator(int width, bool sign) {
		//������ڷ���λ���򷵻�imul�����򷵻�mul
		auto instruction_ = (sign) ? "imul" : "mul";
	}

	void code_generator::generate_comparation_operator(int width_, const char* instruction_) {
		std::string ret;

	}

	void code_generator::generate_or_operator(binary_expression* or_opetator_) {
		visit_expression(or_opetator_->lhs_wrapper());
	}


	void code_generator::generate_assignment_operator(binary_expression* binary_expression_) {
		

	}

	void code_generator::visit_jump_statement(jump_statement* jump_statement_) {
		emit("jmp", jump_statement_->jump_wrapper());
	}

	void code_generator::visit_if_statement(if_statement* if_statement_) {
		visit_expression(if_statement_->if_statement_wrapper());

		auto else_label_ = labeled_statement::create();
		auto end_label_ = labeled_statement::create();
	}


	void code_generator::visit_binary_operation(binary_expression* binary_expression_) {
		auto operation_ = binary_expression_->operation_wrapper();

		switch (operation_) {
		case '=':
			return generate_assignment_operator(binary_expression_);
		case static_cast<int>(TokenAttr::LOGICAL_AND):
			return generate_and_operator(binary_expression_);
		case '.':
			return generate_member_reference_operator(binary_expression_);
		case ',':
			return generate_comma_operator(binary_expression_);
		}

		auto type_ = binary_expression_->lhs_wrapper()->type_value();
		auto 
	}

	void code_generator::visit_conditional_expression(conditional_expression* conditional_expression_) {
		auto if_statement_ = if_statement::create(conditional_expression_->condition_wrapper(),
			conditional_expression_->true_wrapper(), conditional_expression_->false_wrapper());
		visit_if_statement(if_statement_);
	}

	void code_generator::visit_identifier(identifier* identifier_) {
		emit("leaq", identifier_->indentifier_name(), "rax");
	}

}
