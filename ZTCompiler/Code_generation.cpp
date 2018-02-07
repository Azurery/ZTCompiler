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

	/*
	%rax	�����ۼ���
	%rcx	��ʱ�����Ĵ��� 
	%r10	������ַ��ѵ��
	%r11	����Դ�����Ĵ���
	%r12��%r13 	
			����%rdx��%rcx�Ĵ�������ʱ�Ĵ���

	%xmm0	���ڸ������ۼ���
	%xmm1~%xmm7 ���ڴ洢��������
	%xmm8   ����parameter passing��%xmm0������ʱ�Ĵ���
	%xmm9	����Դ�����Ĵ���
	%xmm10	��������������ʱ����ʱ�Ĵ���
	*/
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

	void code_generator::emit(const std::string& instruction_, const std::string& source_) {
		emit(instruction_ + source_+"\n");
	}

	void code_generator::emit(const std::string& instruction_, int immediate_) {
		emit(instruction_ + "$%d\n" + std::to_string(immediate_));
	}

	void code_generator::emit_label(const std::string& label_) {
		fprintf(output_file_, "%s:\n", label_);
	}

	void code_generator::emit_load(const std::string& sorce_address_, int width_, bool is_float_) {
		auto load_instruction_ = get_load_instruction(width_);
		auto destination_ = get_destination(width_ == 4 ? 4 : 8);
		//movsxq  src,dest
		emit(load_instruction_, sorce_address_, destination_);
	}

	void code_generator::emit_load(const std::string& sorce_address_, type* type_) {
		assert(type_->is_scalar());
		emit_load(sorce_address_, type_->width(), type_->is_float());
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
	//����ָ���width��������Ӧ�ļĴ������˴������ݼ��ص�%eax�Ĵ���
	//%al %ax %eax %rax �Ĵ�����λ�����ε�������8λ��64λ
	std::string code_generator::load_register(int width_) {
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
	std::string code_generator::get_instruction(const std::string& instruction_, int width_,bool is_float_) {
		if (is_float_) {
			return instruction_ + (width_ == 4 ? "ss" : "sd");
		}else {
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
		
	}

	std::string code_generator::get_instruction(const std::string& instruction_, type* type_) {
		assert(type_->is_scalar());
		return get_instruction(instruction_, type_->width(),type_->is_float());
	}

	std::string code_generator::get_destination(int width_) {
		return load_register(width_);
	}

	//���ڻ�ȡԴ������
	std::string code_generator::get_source(int width_,bool is_float_) {
		//���Դ��������һ�����������򷵻�%xmm0�Ĵ�����
		//��Ϊ%xmm9���ڴ洢Դ�������ĵ�ַ
		if (is_float_) {
			return "%xmm9";
		}
		switch (width_) {
		case 1:
			return "%r11b";
		case 2:
			return "r11w";
		case 4:
			return "r11d";
		case 8:
			return "r11";
		default:
			return "";
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

	void code_generator::save(bool is_float_) {
		//����Ǹ���������ʹ��movsd %xmm0,%xmm9
		if (is_float_)
			emit("movsd", "%xmm0", "%xmm9");
		else
			emit("movq", "%rax", "%r11");
	}

	void code_generator::generate_comma_operator(binary_expression* comma_) {
		visit_expression(comma_->lhs_wrapper());
		visit_expression(comma_->rhs_wrapper());
	}

	void code_generator::generate_mul_operator(int width_, bool sign_,bool is_float_) {
		const char* instruction_;
		//������ڷ���λ���򷵻�imul�����򷵻�mul
		if (is_float_&&sign_)
			instruction_ = "mul";
		else
			instruction_ = "imul";

		if (is_float_) {
			emit(get_instruction(instruction_, width_, is_float_));
		}
	}
		
	//movl	8(%ebp), %eax
	//cmpl	12(%ebp), %eax
	void code_generator::generate_comparation_operator(int width_, const char* instruction_,bool is_float_) {
		auto cmp_ = (is_float_) ? (width_ == 8 ? "ucomisd" : "ucomiss") : get_instruction(instruction_, width_, is_float_);
		emit(cmp_, get_source(width_, is_float_), get_destination(width_));
	}

	
	//	movl	%esp, %ebp
	//	cmpl	$0, 8(%ebp)
	//	jne	L2
	//	cmpl	$0, 12(%ebp)
	//	je	L3
	//	L2 :
	//	movl	$1, %eax
	//	jmp	L1
	//	L3 :
	//	L1:
	void code_generator::generate_or_operator(binary_expression* or_operator_) {
		visit_expression(or_operator_->lhs_wrapper());
		generate_comparation_zero(or_operator_->lhs_wrapper()->type());
		auto true_label_ = labeled_statement::create();
		emit("jne", true_label_);

		visit_expression(or_operator_->rhs_wrapper());
		generate_comparation_zero(or_operator_->rhs_wrapper()->type());
		emit("je", true_label_);
		
		auto false_label_ = labeled_statement::create();
		emit_label(true_label_);
		emit("movl", "$1", "%eax");
		emit("jmp", false_label_);
		emit_label(false_label_);
	}

	
	/*if(a&&b)
	pushl	%ebp
	movl	%esp, %ebp
	cmpl	$0, 8(%ebp)
	je	L2
	cmpl	$0, 12(%ebp)
	je	L2
	movl	$1, %eax
	jmp	L1
	L2:
	L1:
	popl	%ebp
	ret*/
	
	void code_generator::generate_and_operator(binary_expression* and_operator_) {
		visit_expression(and_operator_->lhs_wrapper());
		generate_comparation_zero(and_operator_->lhs_wrapper()->type());
	
		auto false_label_ = labeled_statement::create();
		emit("je", false_label_);

		visit_expression(and_operator_->rhs_wrapper());
		generate_comparation_zero(and_operator_->rhs_wrapper()->type());
		emit("je", false_label_);

		auto true_label_ = labeled_statement::create();
		emit("movl", "$1", "%eax");
		emit("jmp", true_label_);
		emit_label(false_label_);
		emit_label(false_label_);

	}

	void code_generator::generate_assignment_operator(binary_expression* assignment_) {
		auto left_address_ = assignment_->lhs_wrapper();

	}

	void code_generator::generate_div_operator(int width_, bool sign_, int operation_,bool is_float_) {
		if (is_float_) {
			//movss:��һ�������ȸ��������͵��ڴ���߼Ĵ���
			//movsd����һ��˫���ȸ��������͵��ڴ���߼Ĵ����ĵ�64λ*
			auto instruction_ = (width_ == 4 ? "movss" : "movsd");
			emit(instruction_, "%xmm9", "%xmm0");
			return;
		}
		//�����unsigned integers
		if (!sign_) {
			emit(get_instruction("div", width_, is_float_),get_source(width_,is_float_));
		}else {
			//cltd
			//idivl	12(%ebp)
			emit("cltd");
			emit(get_instruction("idiv", width_, is_float_));
		}
		//movl	%edx, %eax
		if (operation_ == '%')
			emit("movl", "edx", "eax");		
	}

	//�жϵ�ǰֵ�Ƿ�Ϊ0
	void code_generator::generate_comparation_zero(type* type_) {
		auto width_ = type_->width();
		auto is_float_ = type_->is_float();

		//������Ǹ���������ʹ��cmp $0,%eax(��%rax��%ax)
		if (!is_float_) {
			emit("cmpl", "$0", load_register(width_));
		}
		else {
			//���Ǹ���������ʹ��poxrָ���%xmm9����
			//pxor	%xmm9,%xmm9		��Ϊ������ͬ�����ֽ����������󽫷���0
			//ucomiss %xmm9��%xmm0	��%xmmo�Ĵ�����Ϊ0
			//ucomissָ���ܹ��Ƚϵ����ȸ���������������NaN������Чֵ��
			emit("pxor", "%xmm9", "%xmm9");
			auto cmp_ = (width_ == 8 ? "ucomisd" : "ucomiss");
			emit(cmp_, "%xmm9", "%xmm0");
		}
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

		auto type_ = binary_expression_->lhs_wrapper()->type();
		auto width_ = type_->width();
		auto is_float_ = type_->is_float();
		auto is_unsigned_ = type_->is_unsigned();

		visit(binary_expression_->lhs_wrapper());
		visit(binary_expression_->rhs_wrapper());

		const char* instruction_ = nullptr;
		switch (operation_) {
		case '*':
			return generate_mul_operator(width_, is_unsigned_,is_float_);
		case '%':case '/': {
			if (is_float_) {
				//divss	����������ѹ�������������32λ��
				//divsd ����������˫���ȸ����������64λ��
				auto instr = ((width_ == 4) ? "divss" : "divsd");
				emit(instr, "%xmm9", "xmm0");
				return;
			}
			if (!is_unsigned_) {
				emit("xor", "%rdx", "rdx");

			}
		}
		}
	}

	void code_generator::visit_conditional_expression(conditional_expression* conditional_expression_) {
		auto if_statement_ = if_statement::create(conditional_expression_->condition_wrapper(),
			conditional_expression_->true_wrapper(), conditional_expression_->false_wrapper());
		visit_if_statement(if_statement_);
	}

	void code_generator::visit_identifier(identifier* identifier_) {
		emit("leaq", identifier_->indentifier_name(), "rax");
	}
	//ȡ������
	void code_generator::generate_minus_operator(unary_expression* minus_operator_) {
		auto width_ = minus_operator_->type()->width();
		auto is_float_ = minus_operator_->type()->is_float();

		visit_expression(minus_operator_);
		//movss %xmm1, %xmm9
		//xorps %xmm9, %xmm0
		if (is_float_) {
			emit(get_instruction("mov", width_, is_float_), "%xmm9", "%xmm0");
			emit(get_instruction("xor", width_, is_float_),"%xmm0","%xmm9");
		}else {
		//movl %edi, %eax
		//negl %eax
			emit(get_instruction("mov", width_, is_float_), "%edi", "%eax");
			emit(get_instruction("neg", width_, is_float_), get_destination(width_));
		}
	}

	//(6.5.4) cast-expression:
	//			unary-expression
	//			( type-name ) cast-expression
	//
	void code_generator::generate_cast_operator(unary_expression* cast_operator_) {
		auto destination_type_ = cast_operator_->type();
		auto source_type_ = cast_operator_->unary_expression_wrapper()->type();

		//pxor %xmm0, %xmm0
		//cvtsi2ss %xmm0, %xmm0
		if (destination_type_->is_float() && source_type_->is_float()) {
			if (destination_type_->width() == source_type_->width())
				return;
			emit("pxor", "%xmm0", "%xmm0");
			auto instruction_ = (source_type_ ->width()== 4 ? "cvttss2si" : "cvtsi2ss");
			emit(instruction_, "%xmm9", "xmm0");
		}else if(source_type_->is_float()){

		}
	}


	
}
