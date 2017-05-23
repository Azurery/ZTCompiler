#include <set>
#include <string>
#include <memory>
namespace ZTCompiler {
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
			LOMG,
			RETURN,
			SHORT,
			STRUCT,
			SWITCH,
			UNION,
			TYPEDEF,
			VOID
		};
		using keyset = std::set<std::string>;
		class keysetcase {
		private:
			static std::shared_ptr<keyset> ptr;
		public:
			static std::shared_ptr<keyset> get_instance();
		};


	//token����Ӧ�ð���
	class token {
		enum class type {
			KEYWORD,
			NUMBER,
			STRING,
			IDENTIFIER,
			SPACE
		};
		
		
	};
}
