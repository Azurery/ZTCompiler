#ifndef _ZT_CODE_GENERATION_H_
#define _ZT_CODE_GENERATION_H_
#include "AST.h"

class Parser;
class Token;
//System V�Ĳ������ݷ�ʽ
enum class Parameter_passing {
	INTEGER,
	SSE,
	SSE_UP,
	X87,
	X87_UP,
	COMPLEX_87,
	MEMORY,
	NO_CLASS
};


#endif
