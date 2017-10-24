#ifndef _ZT_LECICAL_ANALYSIS_H_
#define _ZT_LECICAL_ANALYSIS_H_
#include "Token.h"
#include <string>
#include <type_traits>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <regex>
#include <vector>
namespace ztCompiler {
	class tokens;
	class token;
	class scanner {
	private:
		token token_;
		std::string::const_iterator cur_;
		const std::string text_;
	public:;
		//scanner(const std::string text):text_(text),token_(TokenAttr::END){}
		virtual ~scanner();
		scanner(const scanner& other) = delete;
		scanner& operator=(const scanner& other) = delete;
	public:
		/*template<typename T>
		typename std::enable_if<true,T>::value 
		print_err(std::string& message, const T err) {
			err = std::move(message);
			return err;
		}*/
		void tokenize(tokens tokens_) {}
		token* scan();
		int scan_esc();	
		token* create_token(int tag);
		token* create_token(TokenAttr type);
		void encode_utf8(uint32_t ch, std::string& out);	//���ڽ���utf-8�ַ�
		void skip_white_space();
		bool skip_comment();
		token* skip_number();
		token* skip_identifier();
		int scan_ucn(int length);
		int to_hex(int value);
		bool test_next_token(int tag);	//���ڲ�����һ��token
		void scan_string();//���ڴ����ַ���
		token* scan_number();	//���ڴ�������
		std::string scan_identifier();//���ڴ����ʶ��
		bool in_range(uint32_t target,uint32_t low,uint32_t high);	 //�����ж��ַ��Ƿ�����ĳ������
	};
}
#endif
