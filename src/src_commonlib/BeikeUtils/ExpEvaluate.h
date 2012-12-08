#pragma once
#pragma warning( disable : 4786)
#include <string>
#include <map>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <ctime>
#include <vector>
#include "BeikeUtils.h"

class CExpEvaluate
{
public:
	typedef enum TokenType
	{
		TOKEN_ERROR = -1,
		TOKEN_NONE=0,
		TOKEN_NAME,
		TOKEN_STRING,
		TOKEN_NUMBER,
		TOKEN_END,
		TOKEN_PLUS='+',
		TOKEN_MINUS='-',
		TOKEN_MULTIPLY='*',
		TOKEN_DIVIDE='/',
		TOKEN_ASSIGN='=',
		TOKEN_LHPAREN='(',
		TOKEN_RHPAREN=')',
		TOKEN_COMMA=',',
		TOKEN_NOT='!',

		// comparisons
		TOKEN_LT='<',
		TOKEN_GT='>',
		TOKEN_LE,     // <=
		TOKEN_GE,     // >=
		TOKEN_EQ,     // ==
		TOKEN_NE,     // !=
		TOKEN_AND,    // &&
		TOKEN_OR,      // ||

		// special assignments
		TOKEN_ASSIGN_ADD,  //  +=
		TOKEN_ASSIGN_SUB,  //  +-
		TOKEN_ASSIGN_MUL,  //  +*
		TOKEN_ASSIGN_DIV   //  +/
	};

private:
	std::string m_program;

	const char * m_pWord;
	const char * m_pWordStart;
	// last token parsed
	TokenType m_type;
	std::string m_word;
	double m_value;

public:
	// ctor
	CExpEvaluate (const std::string & program) 
		: m_program (program), m_pWord (m_program.c_str ()), m_type (TOKEN_NONE)
	{
		 m_pfnCallback = NULL;
	}
	
	const double Evaluate (Function_EvaluateCallback pfnCallback);  // get result
	const double Evaluate (const std::string & program, Function_EvaluateCallback pfnCallback);  // get result

private:
	const TokenType GetToken (const bool ignoreSign = false);  
	const double CommaList (const bool get);
	const double Expression (const bool get);
	const double Comparison (const bool get);
	const double AddSubtract (const bool get);
	const double Term (const bool get);      // multiply and divide
	const double Primary (const bool get);   // primary (base) tokens

	inline void CheckToken (const TokenType wanted)
	{
		if (m_type != wanted)
		{
			std::ostringstream s;
			s << "'" << static_cast <char> (wanted) << "' expected.";
			throw std::runtime_error (s.str ());    
		}
	}

private:
	Function_EvaluateCallback m_pfnCallback;
};
