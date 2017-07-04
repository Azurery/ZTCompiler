#ifndef _ZT_TOKEN_H_
#define _ZT_TOKEN_H_
#include <set>
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "MemoryPool.h"

namespace ztCompiler {
	class token;
	using token_list = std::vector<const token*>;	//����һ��token��
	//���嵥�ʵ�����
	enum class TokenAttr {
		IDENTIFIER,		//��ʶ��

		//�����
		ADD = '+',
		SUB = '-',
		MUL = '*',
		DIV = '/',
		LESS = '<',
		GREATER = '>',
		HASH = '#',
		DOUBLE_HASH = '##',
		//��������
		POINTER,	//ָ��
		DEC,	//--
		INC,	//++
		SUB_ASSIGN,//-=
		ADD_ASSIGN,//+=
		LEFT_SHIFT,//<<
		LEFT_EQUAL,	//<=
		RIGHT_SHIFT,//>>
		RIGHT_EQUAL,//>=
		EQUAL,	//==
		NOT_EQUAL,//!=
		LOGICAL_AND,//&&
		AND_ASSIGN,//&=
		LOGICAL_OR,//||
		OR_ASSIGN,//|=
		LOGICAL_XOR,
		DIV_ASSIGN, // /=
		MUL_ASSIGN,//*=
		ELLIPSIS,//...
		VOID,
		INT,
		FLOAT,
		CHAR,
		BREAK,
		CASE,
		CHAR,
		CONST,
		CONTINUE,
		DEFAULT,
		DO,
		WHILE,
		DOUBLE,
		IF,
		ELSE,
		FOR,
		RETURN,
		STRUCT,
		SWITCH,
		UNION,
		TYPEDEF,
		VOID,
		END,
		NEW_LINE,
		NOTDEFINED	//������һ��keyword
	};
	//token����Ӧ�ð������غ�����ֵ
	class token {
		friend class scanner;
	public:
		static memory_pool token_pool;
		static token* new_token(const token& other) {
			return new(token_pool.allocate(sizeof(other))) token(other);
		}
		static token* new_token(TokenAttr type) {
			return new(token_pool.allocate(sizeof(type))) token(type);
		}
		token& operator = (const token& other) {
			str_ = other.str_;
			type_ = other.type_;
			return *this;
		}
		//����key�����type
		static TokenAttr get_keyword_type(const std::string& key) {
			auto iter = keyword_map.find(key);
			if (iter != keyword_map.end()) {
				return TokenAttr::NOTDEFINED;
			}
			return iter->second;
		}
		static bool is_keyword(const std::string& other);
		static bool is_keyword(TokenAttr type);
		bool is_keyword() const { return is_keyword(type_); }
		bool is_identifer() const { return type_ == TokenAttr::IDENTIFIER; }
		bool is_eof() const { return type_ == TokenAttr::END; }
		virtual ~token() {}
	private:
		token(std::string str, TokenAttr type) :str_(str), type_(type) {};
		token(const token& other) { *this = other; }
		explicit token(TokenAttr type) :type_(type) {}
		token(TokenAttr type, std::string str) :type_(type), str_(str) {}
		static const std::unordered_map<std::string, TokenAttr> keyword_map;

	public:
		std::string str_;	//Token�Ĵ���
		TokenAttr type_;		//Token������ֵ

	};

	////��classΪһ��token��������һ��token���
	class tokens {
		friend class token;
		friend class scanner;
	private:
		token_list* tokens_;
		token_list::iterator begin_;
		token_list::iterator end_;
	public:
		tokens() :tokens_(new token_list()), begin_(tokens_->begin()), end_(tokens_->end()) {}
		explicit tokens(token* t) {
			tokens();
			insert_back(t);
		}
		explicit tokens(token_list* token_list_)
			:tokens_(token_list_), begin_(token_list_->begin()), end_(token_list_->end()) {}
		tokens(token_list* token_list_,
			token_list::iterator begin,
			token_list::iterator end)
			:tokens_(token_list_),begin_(begin),end_(end){}
		~tokens() {};
		tokens(const tokens& other) { *this = other; }
		const tokens& operator=(const tokens& other) {
			tokens_ = other.tokens_;
			begin_ = other.begin_;
			end_ = other.end_;
			return *this;
		}

		void copy(const tokens& other) {
			auto ts= new token_list(other.begin_, other.end_);
			this->tokens_ = ts;
			this->begin_ = ts->begin();
			this->end_ = ts->end();
			for (auto iter = begin_; iter != end_; ++iter)
				*iter = token::new_token(**iter);
		}
		//�Ƿ����type���͵�token
		//bool contains(TokenAttr type) { get_next_token()->type_ == type; }
		//������һ��token(ֻ���Ը�token������ǰ�ƶ�token_list��ƫ��ָ��)
		const token* get_next_token() const {
			auto flag = token::new_token(TokenAttr::END);
			if (begin_ != end_ && (*begin_)->type_ == TokenAttr::NEW_LINE) {
				std::next(begin_);
				return get_next_token();
			}
			else if (begin_ == end_) {	//���token_listΪ��
				if (end_ != tokens_->begin()) {
					auto back_ = end_;
					*flag = **std::prev(back_);
					flag->type_ = TokenAttr::END;
					return flag;
				}
			}
			else if ((*begin_)->type_ == TokenAttr::IDENTIFIER) {

			}
			return *begin_;
		}
		const token* consume_next_token() {//������һ��Token
			auto ret = get_next_token();
			if (!ret->is_eof()) {
				++begin_;
				get_next_token();
			}
			return ret;
		}
		token_list::iterator get_insert_pos() {
			auto pos = begin_;
			if (pos == tokens_->begin())
				return begin_;
		}
		void insert_back(tokens& ts) {
			auto pos = tokens_->insert(end_, ts.begin_, ts.end_);
			if (begin_ == end_)
				begin_ = pos;
		}
		void insert_back(const token* token) {

		}
	};
}
#endif