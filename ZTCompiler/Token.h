#ifndef _ZT_TOKEN_H_
#define _ZT_TOKEN_H_
#include <set>
#include <string>
#include <memory>
#include <type_traits>
namespace ztCompiler {
	//����c���Ե�key word
	enum class keyword {
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
		INT,
		LONG,
		RETURN,
		SHORT,
		STRUCT,
		SWITCH,
		UNION,
		TYPEDEF,
		VOID
	};

	using keyset = std::set<std::string>;

	class keyset_instance {
	private:
		static std::shared_ptr<keyset> ptr;
	public:
		static std::shared_ptr<keyset> get_instance();
	};

	enum class TokenType {
		IDENTIFIER,//��ʶ������
		KEYWORD,//�ؼ�������
		NUMBER,	//��������
		SIGN,
		STRING,	//�ַ�������
		COMMENT	//ע������
	};
	//token����Ӧ�ð���
	class token {
		friend class LexicalAnalysis;
	public:
		token() = default;
		token(TokenType type, const std::string& value);
		token(TokenType type, const std::string&& value);
	private:
		const std::string value;
		TokenType type;
	};
}
#endif