#ifndef _ZT_LECICAL_ANALYSIS_H_
#define _ZT_LECICAL_ANALYSIS_H_
#include <string>

namespace ztCompiler {
	class LexicalAnalysis {
	private:
		enum class State {
			START,	//START��Ϊ״̬���ĳ�ʼ״̬
			IDENTIFIER,	
			SIGN,
			COMMENT,//��START״̬�¶���/*��ʹ״̬ת��ΪCOMMENT
			SPACE,
			STRING,
			REGEX
		};
	};
}
#endif
