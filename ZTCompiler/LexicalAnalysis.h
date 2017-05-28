#ifndef _ZT_LECICAL_ANALYSIS_H_
#define _ZT_LECICAL_ANALYSIS_H_
#include "Token.h"
#include <string>
#include <vector>
#include <cassert>
#include <stack>
#include <unordered_map>
namespace ztCompiler {
	//�˷���������ʵ��һ��DNF
	class LexicalAnalysis {
	private:
		//״̬��������
		enum class StateType {
			START,	//START��Ϊ״̬���ĳ�ʼ״̬
			IDENTIFIER,	//��ʶ��״̬
			COMMENT,//��START״̬�¶���/*��ʹ״̬ת��ΪCOMMENT
			STRING,	//�ַ�״̬
			END		//״̬���Ľ���״̬
		};
	protected:
		void skip_white_space();
		void scan_string(std::string& out);//���ڴ����ַ���
		void scan_number(std::string& out);	//���ڴ�������
		void scan_identifier(std::string& out);//���ڴ����ʶ��
		bool contains();	 //�����ж��ַ��Ƿ�����ĳ������
	private:
		size_t line_num;	 //���ڼ�¼��ǰ��ȡ��token���к�
		std::string::const_iterator cur;	//���ڼ�¼��һ������ȡ��token��λ��
		StateType state_type;	 //״̬���ĸ���״̬
		TokenType token_type;//��������
		std::string str;
	public:
		token get_next_token();
		//�������е�tokens
		std::vector<token> get_tokens();
	};
}
#endif
