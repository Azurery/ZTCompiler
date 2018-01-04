#pragma once
#ifndef _ZT_PARSE_H_
#define _ZT_PARSE_H_

#include "AST.h"
#include "Token.h"
#include "Scanner.h"

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <stack>
namespace ztCompiler {
	class parser {
	public:
		explicit parser(const tokens* tokens_) {

		}
		~parser() {
			delete scanner_;
		}

	private:
		//�����ǰtoken���ϣ�����true����consumeһ��token
		//�����token-tag��������򷵻�false�����Ҳ�consume token
		bool test_next_taken(int token_tag) {
			auto token=
		}

		////���ص�ǰtoken����ǰ��
		//token* get_next_token() {
		//	return scanner_->test_next_token();

		//}


		bool is_type(const token* token) {
			if (token->is_type_qualifier() || token->is_type_specifier()) {
				return true;
			}
			return false;
		}
	private:
		translate_unit* trans_unit_;
		scanner* scanner_;
		environment* enviroment_;
		std::stack<token*> token_buffer_;
	};

}
#endif
