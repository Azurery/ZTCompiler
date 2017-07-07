#ifndef _ZT_STATEMENT_H_
#define _ZT_STATEMENT_H_
#include "MemoryPool.h"
#include "Token.h"
namespace ztCompiler {
	class visitor {
	public:
		virtual ~visitor(){}
		
	};
	//�����﷨����㣬���е��﷨��㶼ֱ�ӻ��߼�Ӽ̳�����
	class ast_node {
	public:
		virtual ~ast_node(){}
		virtual void visit(visitor* vistor_) = 0;
	protected:
		ast_node(){}
		memory_pool* pool_=nullptr;
	};

	//���
	class statement :public ast_node {
	public:
		virtual ~statement(){}
	protected:
		statement(){}
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
		virtual ~expresstion(){}
		virtual bool is_lvalue() = 0;
		virtual void check_type() = 0;
		const token* get_token() const { return ; }
	};


	class empty_statement :public statement {
	public:
		virtual ~empty_statement() {}
		virtual void visit(visitor* vistor_);
		static empty_statement* construct();
	protected:
		empty_statement(){}
	};

	//if���
	class if_statement :public statement {
	public:
		virtual ~if_statement(){}
		virtual void visit(visitor* vistor_);
		static if_statement* construct();
	protected:
	private:
		
	};
}
#endif