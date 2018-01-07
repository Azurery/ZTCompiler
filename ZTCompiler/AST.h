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
	class identifier :public expression {
		friend class translate_unit;
		friend class parser;
	public:
		static const int var = 0;

		~identifier() {}
		//identifier��Ȼ����ֵ
		virtual bool is_lvalue() const {
			return true;
		}

		bool operator==(const identifier& other) const {
			return other.offset_ == offset_ ;
		}

		bool operator!=(const identifier& other) const {
			return !(other == *this);
		}
	protected:
		identifier(qualifier_type* type, int offset = var)
			:expression(type), offset_(offset) {}
	private:
		int offset_;
	};

	//labeled-statement ������
	class labeled_statement :public statement {
	private:
		int label_;
		int generate_label() {
			int label = 0;
			return ++label;
		}

	public:
		labeled_statement * create();
		~labeled_statement() {}
		virtual void accept(visitor* visitor_);
	protected:
		labeled_statement() :label_(generate_label()) {}
	};

	//selection-statement ѡ�����
	/*	if ( expression ) statement
		if ( expression ) statement else statement
		switch ( expression ) statement
	*/
	class selection_statement :public statement {
	private:
		labeled_statement * select_label_;
	protected:
		selection_statement(labeled_statement* label_state){}
	public:
		virtual const void accpet() = 0;
		virtual const selection_statement* create() const = 0;
		virtual ~selection_statement(){}

	};
	//compound-statement �������
	/*(6.8.2) compound-statement:
				{ block-item-listopt }
		(6.8.2) block-item-list:
				block-item
				block-item-list block-item
		(6.8.2) block-item:
				declaration
				statement
	*/

	using statement_list = std::list<statement*>;
	class compound_statement :public statement {
	private:
		statement_list statements_;
	protected:
		compound_statement(const statement_list statements):
			statements_(statements){}
	public:
		compound_statement * create(statement_list& statements);
		virtual ~compound_statement(){}
		virtual void accept(visitor* visitor_);
		statement_list& statements_value(){ return statements_; }
	};

	/*(6.8.5) iteration-statement:
				while ( expression ) statement
				do statement while ( expression ) ;
				for ( expressionopt ; expressionopt ; expressionopt ) statement
				for ( declaration expressionopt ; expressionopt ) statement
	*/

	class iteration_statement : public statement {

	};


	//jump-statement ��ת���
	/*	jump-statement:
			goto identifier ;
			continue ;
			break ;
			return expressionopt ;
	*/
	class jump_statement :public statement {
	private:
		labeled_statement * label_;
	protected:
		jump_statement(labeled_statement* label) :
			label_(label){}
	public:
		jump_statement* create();
		~jump_statement();
		virtual void accept(visitor* visitor_);
		void set_label(labeled_statement* label) { this->label_ = label; }
	};

	//if statements
	class if_statement :public selection_statement {
	private:
		expression* condition_;
		statement* then_;
		statement* else_;
	protected:
		/*if_statement(expression* condition, statement* then, statement* elses = nullptr) :
			condition_(condition),then_(then),else_(elses){}
		*/
	public:
		virtual ~if_statement(){}
		if_statement* create(expression* condition, statement* then, statement* elses=nullptr);
		virtual void accept(visitor* vistor_);
	};

	//integer or float
	class constant :public expression {
		friend class translate_unit;
	public:
		~constant(){}
		virtual bool is_lvalue() const {
			return false;
		}
		static constant* create(const token* token_, int flag, long value);
		static constant* create(const token* token_, int flag, double value);
		static constant* create(const token* token_, int flag, const std::string& value);
	protected:
		constant(arithmetic_type* type, size_t value)
			:expression(type), value_(value) {}
		explicit constant(pointer_type* type)
			:expression(type), value_(0) {}
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
	class expression : public statement {
	public:
		virtual ~expression() {}
		/*virtual bool is_lvalue() = 0;
		virtual void check_type() = 0;*/
		virtual bool is_lvalue() const;
		qualifier_type* type_value() {
			return type_;
		}
		const qualifier_type* type_value() const {
			return type_;
		}
	protected:
		expression(qualifier_type* type):type_(type){}
		const token*  token_;
		qualifier_type* type_;
	};

	/*��Ԫ������
	*+ - *  / % < > << >> | & ^ =
	*== != <= >=
	*&& ||
	*.(��Ա�������
	*/
	class binary_operation :public expression {
		friend class translate_unit;
	public:
		static binary_operation* construct(const token* token_, expression* lhs, expression* rhs);
		virtual ~binary_operation() {}
		virtual void accept(visitor* visitor_);
		virtual bool is_lvalue() const {
			//TODO:switch(op_)
			//�������ʽ������ֵ����һЩoperator����ֵ����operator-
			return false;
		}
	protected:
		binary_operation(qualifier_type* type,int op, expression* lhs, expression* rhs)
			:expression(type),op_(op),lhs_(lhs), rhs_(rhs) {}
	protected:
		int op_;
		expression* lhs_;
		expression* rhs_;
	};

	class unary_operation :public expression {
		friend class translate_unit;
	public:
		~unary_operation(){}
		
		//TODO:��'++i'������ֵ��'~i'�㲻����ֵ
		virtual bool is_lvalue() const {
			//������deref('*') op����ֵ
			return (TokenAttr::DEREF == static_cast<TokenAttr>(op_));
		}
		static unary_operation* create(int flag, expression* operator_);
	protected:
		unary_operation(qualifier_type* type, int op, expression* expression)
			:expression(type), expression_(expression) {

		}
	private:
		int op_;
		expression* expression_;
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

	//return���
	class return_statement :public statement {
	public:
		virtual ~return_statement();
		virtual void accept(visitor* visitor_);
		static return_statement* construct();
	protected:
		return_statement(expression* expr):expr_(expr){}
	private:
		expression* expr_;
	};

	
	class function_call :public expression {
		friend class translate_unit;
	public:
		~function_call(){}
		//function call����lvalue
		virtual bool is_lvalue() const {
			return false;
		}

	protected:
		function_call(qualifier_type* type,expression* caller,std::list<expression*> args)
			:expression(type),caller_(caller),args_(args){}
	private:
		expression* caller_;
		std::list<expression*> args_;
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

		static binary_operation* new_binary_operation(qualifier_type* type,int op,expression* lhs,expression* rhs) {
			return new binary_operation(type,op,lhs,rhs);
		}

		static unary_operation* new_unary_operation(qualifier_type* type, int op, expression* expr) {
			return new unary_operation(type,op,expr);
		}

		static function_call* new_function_call(qualifier_type* type,expression* caller,const std::list<expression*>& args) {
			return new function_call(type,caller,args);
		}

		static identifier* new_identifier(qualifier_type* type,int offset=0) {
			return new identifier(type,offset);
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