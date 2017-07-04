#pragma once
#ifndef _ZT_PARSE_H_
#define _ZT_PARSE_H_
#include <string>
#include <vector>
#include <set>
#include <memory>
namespace ztCompiler {
	//statement��base class
	enum class statement_catagory {
		IF_STATEMENT,
		WHILE_STATEMENT,
		FOR_STATEMENT,
		ARITHMETIC_STATEMENT,
		DEFINE_STATEMENT,
		ASSGIN_STATEMENT
	};
	class statement_node {
	public:
		typedef std::shared_ptr<statement_node> ptr;
		std::vector<ptr> node;
		statement_catagory catagory;
	};
	class base_node{
		std::set<char> first_set;
		std::set<char> follow_set;
		std::vector<char> terminal;	//�ս��
		std::vector<char> nonterminal;//���ս��
	public:
		int get_terminal_index(char target);	//��ȡ���ս�������е��±�
		int get_nonterminal_index(char target); //��ȡ�ڷ��ս�������е��±�
		void get_first(char target);	//�õ�first����
		void get_follow(char target);	//�õ�follow����
		void resolve();		//����õ���first��follow����
	};

}
#endif
