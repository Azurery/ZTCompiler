#ifndef _ZT_AST_H_
#define _ZT_AST_H_

#include "MemoryPool.h"
#include "Token.h"
#include "Type.h"
#include "Parser.h"

#include <list>

namespace ztCompiler {
	class token;
	class visitor {
	public:
		virtual ~visitor() {}
		virtual void visit_binary_op(binary_operation* binary) = 0;
		/*virtual void visit_unary_op(unary_operation* binary) = 0;
		virtual void visit_binary_op(binary_operation* binary) = 0;
		virtual void visit_binary_op(binary_operation* binary) = 0;
		virtual void visit_binary_op(binary_operation* binary) = 0;
		virtual void visit_binary_op(binary_operation* binary) = 0;
		virtual void visit_binary_op(binary_operation* binary) = 0;
		virtual void visit_binary_op(binary_operation* binary) = 0;*/
	};
	//�����﷨����㣬���е��﷨��㶼ֱ�ӻ��߼�Ӽ̳�����
	class ast_node {
	public:
		virtual ~ast_node() {}
		//virtual void accept(visitor* vistor_) = 0;
	protected:
		ast_node() {}
		memory_pool* pool_ = nullptr;
	};

	//���
	class statement :public ast_node {
	public:
		virtual ~statement() {}
	protected:
		statement() {}
	};

	//identifier ��ʶ��
	class variable :public expresstion {
		friend class translate_unit;
		friend class parser;
	public:
		static const int TYPE = -1;
		static const int VAR = 0;

		~variable(){}
		//variable��Ȼ����ֵ
		virtual bool is_lvalue() const {
			return true;
		}

		bool operator==(const variable& other) const {
			return (other.offset_ == offset_&&(*other.type_ == *type_));
		}

		bool operator!=(const variable& other) const {
			return !(other == *this);
		}
	protected:
		variable(type* type,int offset=VAR)
			:expresstion(type),offset_(offset){}
	private:
		int offset_;
	};

	//integer or float
	class constant :public expresstion {
		friend class translate_unit;
	public:
		~constant(){}
		virtual bool is_lvalue() const {
			return false;
		}
	protected:
		constant(arithmetic_type* type, size_t value)
			:expresstion(type), value_(value) {}
		explicit constant(pointer_type* type)
			:expresstion(type), value_(0) {}
		size_t value_;
	};

	/*���ʽexpression��Ϊ��
	*1.��Ŀ������
	*2.˫Ŀ������
	*3.����������
	*4.��������
	*5.��ʶ��
	*6.����
	*/
	class expresstion : public statement {
	public:
		virtual ~expresstion() {}
		/*virtual bool is_lvalue() = 0;
		virtual void check_type() = 0;*/
		virtual bool is_lvalue() const = 0;
		type* type_value() {
			return type_;
		}
		const type* type_value() const {
			return type_;
		}
	protected:
		expresstion(type* type):type_(type){}
		type* type_;
	};

	/*��Ԫ������
	*+ - *  / % < > << >> | & ^ =
	*== != <= >=
	*&& ||
	*.(��Ա�������
	*/
	class binary_operation :public expresstion {
		friend class translate_unit;
	public:
		static binary_operation* construct(const token* token_, expresstion* lhs, expresstion* rhs);
		virtual ~binary_operation() {}
		virtual void accept(visitor* visitor_);
		virtual bool is_lvalue() const {
			//TODO:switch(op_)
			//�������ʽ������ֵ����һЩoperator����ֵ����operator-
			return false;
		}
	protected:
		binary_operation(type* type,int op, expresstion* lhs, expresstion* rhs)
			:expresstion(type),op_(op),lhs_(lhs), rhs_(rhs) {}
	protected:
		int op_;
		expresstion* lhs_;
		expresstion* rhs_;
	};

	class unary_operation :public expresstion {
		friend class translate_unit;
	public:
		~unary_operation(){}
		
		//TODO:��'++i'������ֵ��'~i'�㲻����ֵ
		virtual bool is_lvalue() const {
			//������deref('*') op����ֵ
			return (TokenAttr::DEREF == static_cast<TokenAttr>(op_));
		}
	protected:
		unary_operation(type* type, int op, expresstion* expresstion)
			:expresstion(type), expresstion_(expresstion) {

		}
	private:
		int op_;
		expresstion* expresstion_;
	};

	//empty���
	class empty_statement :public statement {
	public:
		virtual ~empty_statement() {}
		virtual void accept(visitor* vistor_);
		static empty_statement* construct();
	protected:
		empty_statement() {}
	};

	//if���
	class if_statement :public statement {
	public:
		virtual ~if_statement() {}
		virtual void accept(visitor* vistor_);
		static if_statement* construct();
	protected:
		if_statement(expresstion* condition, statement* t, statement* e)
			:condition_(condition), then_(t), else_(e) {}
	private:
		expresstion* condition_;
		statement* then_;
		statement* else_;
	};

	//return���
	class return_statement :public statement {
	public:
		virtual ~return_statement();
		virtual void accept(visitor* visitor_);
		static return_statement* construct();
	protected:
		return_statement(expresstion* expr):expr_(expr){}
	private:
		expresstion* expr_;
	};

	
	class function_call :public expresstion {
		friend class translate_unit;
	public:
		~function_call(){}
		//function call����lvalue
		virtual bool is_lvalue() const {
			return false;
		}

	protected:
		function_call(type* type,expresstion* caller,std::list<expresstion*> args)
			:expresstion(type),caller_(caller),args_(args){}
	private:
		expresstion* caller_;
		std::list<expresstion*> args_;
	};

	using declaration = ast_node;
	class translate_unit :public ast_node {
	public:
		virtual ~translate_unit() {
			auto iter = declaration_.begin();
			while (iter != declaration_.end()) {
				delete *iter;
				iter++;
			}
		}

		void add(declaration* declaration) {
			declaration_.push_back(declaration);
		}

		static translate_unit* new_translate_unit() {
			return new translate_unit();
		}

		static binary_operation* new_binary_operation(type* type,int op,expresstion* lhs,expresstion* rhs) {
			return new binary_operation(type,op,lhs,rhs);
		}

		static unary_operation* new_unary_operation(type* type, int op, expresstion* expr) {
			return new unary_operation(type,op,expr);
		}

		static function_call* new_function_call(type* type,expresstion* caller,const std::list<expresstion*>& args) {
			return new function_call(type,caller,args);
		}

		static variable* new_variable(type* type,int offset=0) {
			return new variable(type,offset);
		}

		static constant* new_constant(arithmetic_type* type,size_t value) {
			return new constant(type,value);
		}

		static constant* new_constant(pointer_type* type) {
			return new constant(type);
		}

	private:
		translate_unit(){}
		std::list<declaration*> declaration_;
	};
}
#endif