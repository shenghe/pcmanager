/*

Parser - an expression parser

Author:  Nick Gammon 
http://www.gammon.com.au/ 

(C) Copyright Nick Gammon 2004. Permission to copy, use, modify, sell and
distribute this software is granted provided this copyright notice appears
in all copies. This software is provided "as is" without express or implied
warranty, and with no claim as to its suitability for any purpose.

Modified 24 October 2005 by Nick Gammon.

1. Changed use of "abs" to "fabs"
2. Changed inclues from math.h and time.h to fmath and ftime
3. Rewrote DoMin and DoMax to inline the computation because of some problems with some libraries.
4. Removed "using namespace std;" and put "std::" in front of std namespace names where appropriate
5. Removed MAKE_STRING macro and inlined the functionality where required.
6. Changed Evaluate function to take its argument by reference.

Thanks to various posters on my forum for suggestions. The relevant post is currently at:

http://www.gammon.com.au/forum/bbshowpost.php?bbsubject_id=4649

*/
#include <vector>
#include "ExpEvaluate.h"

/*

Expression-evaluator
--------------------

Author: Nick Gammon
-------------------


Example usage:

Parser p ("2 + 2 * (3 * 5) + nick");

p.symbols_ ["nick"] = 42;

double v = p.Evaluate ();

double v1 = p.Evaluate ("5 + 6");   // supply new expression and evaluate it

Syntax:

You can use normal algebraic syntax. 

Multiply and divide has higher precedence than add and subtract.

You can use parentheses (eg. (2 + 3) * 5 )

Variables can be assigned, and tested. eg. a=24+a*2

Variables can be preloaded:

p.symbols_ ["abc"] = 42;
p.symbols_ ["def"] = 42;

Afterwards they can be retrieved:

x = p.symbols_ ["abc"];

There are 2 predefined symbols, "pi" and "e".

You can use the comma operator to load variables and then use them, eg.

a=42, b=a+6

You can use predefined functions, see below for examples of writing your own.

42 + sqrt (64)


Comparisons
-----------

Comparisons work by returning 1.0 if true, 0.0 if false.

Thus, 2 > 3 would return 0.0
3 > 2 would return 1.0

Similarly, tests for truth (eg. a && b) test whether the values are 0.0 or not.

If test
-------

There is a ternary function: if (truth-test, true-value, false-value)

eg.  if (1 < 2, 22, 33)  returns 22


Precedence
----------

( )  =   - nested brackets, including function calls like sqrt (x), and assignment
* /      - multiply, divide
+ -      - add and subtract
< <= > >= == !=  - comparisons
&& ||    - AND and OR
,        - comma operator

Credits:

Based in part on a simple calculator described in "The C++ Programming Language"
by Bjarne Stroustrup, however with considerable enhancements by me, and also based
on my earlier experience in writing Pascal compilers, which had a similar structure.

*/
const CExpEvaluate::TokenType CExpEvaluate::GetToken (const bool ignoreSign)
{
	m_word.erase (0, std::string::npos);

	// skip spaces
	while (*m_pWord && isspace (*m_pWord))
		++m_pWord;

	m_pWordStart = m_pWord;   // remember where word_ starts *now*

	// look out for unterminated statements and things
	if (*m_pWord == 0 &&  // we have EOF
		m_type == TOKEN_END)  // after already detecting it
		throw std::runtime_error ("Unexpected end of expression.");

	unsigned char cFirstCharacter = *m_pWord;        // first character in new word_

	if (cFirstCharacter == 0)    // stop at end of file
	{
		m_word = "<end of expression>";
		return m_type = TOKEN_END;
	}

	unsigned char cNextCharacter  = *(m_pWord + 1);  // 2nd character in new word_

	// look for number
	if ((!ignoreSign && 
		(cFirstCharacter == '+' || cFirstCharacter == '-') && 
		isdigit (cNextCharacter)
		) 
		|| isdigit (cFirstCharacter))
	{
		// skip sign for now
		if ((cFirstCharacter == '+' || cFirstCharacter == '-'))
			m_pWord++;
		while (isdigit (*m_pWord) || *m_pWord == '.')
			m_pWord++;

		// allow for 1.53158e+15
		if (*m_pWord == 'e' || *m_pWord == 'E')
		{
			m_pWord++; // skip 'e'
			if ((*m_pWord  == '+' || *m_pWord  == '-'))
				m_pWord++; // skip sign after e
			while (isdigit (*m_pWord))  // now digits after e
				m_pWord++;      
		}

		m_word = std::string (m_pWordStart, m_pWord - m_pWordStart);

		std::istringstream is (m_word);
		// parse std::string into double value
		is >> m_value;

		if (is.fail () || !is.eof ())
			throw std::runtime_error ("Bad numeric literal: " + m_word);
		return m_type = TOKEN_NUMBER;
	}   // end of number found

	// special test for 2-character sequences: <= >= == !=
	// also +=, -=, /=, *=
	if (cNextCharacter == '=')
	{
		switch (cFirstCharacter)
		{
			// comparisons
		case '=': m_type = TOKEN_EQ;   break;
		case '<': m_type = TOKEN_LE;   break;
		case '>': m_type = TOKEN_GE;   break;
		case '!': m_type = TOKEN_NE;   break;
			// assignments
		case '+': m_type = TOKEN_ASSIGN_ADD;   break;
		case '-': m_type = TOKEN_ASSIGN_SUB;   break;
		case '*': m_type = TOKEN_ASSIGN_MUL;   break;
		case '/': m_type = TOKEN_ASSIGN_DIV;   break;
			// none of the above
		default:  m_type = TOKEN_NONE; break;
		} // end of switch on cFirstCharacter

		if (m_type != TOKEN_NONE)
		{
			m_word = std::string (m_pWordStart, 2);
			m_pWord += 2;   // skip both characters
			return m_type;
		} // end of found one    
	} // end of *=

	switch (cFirstCharacter)
	{
	case '&': if (cNextCharacter == '&')    // &&
			  {
				  m_word = std::string (m_pWordStart, 2);
				  m_pWord += 2;   // skip both characters
				  return m_type = TOKEN_AND;
			  }
			  break;
	case '|': if (cNextCharacter == '|')   // ||
			  {
				  m_word = std::string (m_pWordStart, 2);
				  m_pWord += 2;   // skip both characters
				  return m_type = TOKEN_OR;
			  }
			  break;
			  // single-character symboles
	case '=':
	case '<':
	case '>':
	case '+':
	case '-':
	case '/':
	case '*':
	case '(':
	case ')':
	case ',':
	case '!':
		m_word = std::string (m_pWordStart, 1);
		++m_pWord;   // skip it
		return m_type = TokenType (cFirstCharacter);
	} // end of switch on cFirstCharacter

	if(cFirstCharacter=='"')
	{
		const char* pFirst = ++m_pWord;
		while(*m_pWord!=0 && *m_pWord!='"')
			++ m_pWord;
		if(*m_pWord==0)
			throw std::runtime_error("string error");
		m_word.assign(pFirst, m_pWord-pFirst);
		++m_pWord;
		return m_type = TOKEN_STRING;
	}

	if (!isalpha (cFirstCharacter))
	{
		if (cFirstCharacter < ' ')
		{
			std::ostringstream s;
			s << "Unexpected character (decimal " << int (cFirstCharacter) << ")";
			throw std::runtime_error (s.str ());    
		}
		else
			throw std::runtime_error ("Unexpected character: " + std::string (1, cFirstCharacter));
	}

	// we have a word (starting with A-Z) - pull it out
	while (isalnum (*m_pWord) || *m_pWord == '_')
		++m_pWord;

	m_word = std::string (m_pWordStart, m_pWord - m_pWordStart);
	return m_type = TOKEN_NAME;
}   // end of Parser::GetToken

// change program and evaluate it
const double CExpEvaluate::Evaluate (const std::string & program, Function_EvaluateCallback pfn)  // get result
{
	// do same stuff constructor did
	m_program  = program;
	m_pWord    = m_program.c_str ();
	m_type     = TOKEN_NONE;
	return Evaluate ( pfn );
}

const double CExpEvaluate::Evaluate( Function_EvaluateCallback pfn )  // get result
{
	m_pfnCallback = pfn;
	double v = CommaList (true);
	if (m_type != TOKEN_END)
		throw std::runtime_error ("Unexpected text at end of expression: " + std::string (m_pWordStart));
	return v;  
}

const double CExpEvaluate::CommaList (const bool get)  // expr1, expr2
{
	double left = Expression (get);
	while (true)
	{
		switch (m_type)
		{
		case TOKEN_COMMA:  left = Expression (true); break; // discard previous value
		default:    return left;
		} // end of switch on type
	}   // end of loop
} // end of Parser::CommaList

const double CExpEvaluate::Expression (const bool get)  // AND and OR
{
	double left = Comparison (get);
	while (true)
	{
		switch (m_type)
		{
		case TOKEN_AND: 
			{
				double d = Comparison (true);   // don't want short-circuit evaluation
				left = (left != 0.0) && (d != 0.0); 
			}
			break;
		case TOKEN_OR:  
			{
				double d = Comparison (true);   // don't want short-circuit evaluation
				left = (left != 0.0) || (d != 0.0); 
			}
			break;
		default:    return left;
		} // end of switch on type
	}   // end of loop
} // end of Parser::Expression

const double CExpEvaluate::Comparison (const bool get)  // LT, GT, LE, EQ etc.
{
	double left = AddSubtract (get);
	while (true)
	{
		switch (m_type)
		{
		case TOKEN_LT:  left = left <  AddSubtract (true) ? 1.0 : 0.0; break;
		case TOKEN_GT:  left = left >  AddSubtract (true) ? 1.0 : 0.0; break;
		case TOKEN_LE:  left = left <= AddSubtract (true) ? 1.0 : 0.0; break;
		case TOKEN_GE:  left = left >= AddSubtract (true) ? 1.0 : 0.0; break;
		case TOKEN_EQ:  left = left == AddSubtract (true) ? 1.0 : 0.0; break;
		case TOKEN_NE:  left = left != AddSubtract (true) ? 1.0 : 0.0; break;
		default:    return left;
		} // end of switch on type
	}   // end of loop
} // end of Parser::Comparison

const double CExpEvaluate::AddSubtract (const bool get)  // add and subtract
{
	double left = Term (get);
	while (true)
	{
		switch (m_type)
		{
		case TOKEN_PLUS:  left += Term (true); break;
		case TOKEN_MINUS: left -= Term (true); break;
		default:    return left;
		} // end of switch on type
	}   // end of loop
} // end of Parser::AddSubtract

const double CExpEvaluate::Term (const bool get)    // multiply and divide
{
	double left = Primary (get);
	while (true)
	{
		switch (m_type)
		{
		case TOKEN_MULTIPLY:  
			left *= Primary (true); break;
		case TOKEN_DIVIDE: 
			{
				double d = Primary (true);
				if (d == 0.0)
					throw std::runtime_error ("Divide by zero");
				left /= d; 
				break;
			}
		default:    return left;
		} // end of switch on type
	}   // end of loop
} // end of Parser::Term

const double CExpEvaluate::Primary (const bool get)   // primary (base) tokens
{
	if (get)
		GetToken ();    // one-token lookahead  

	switch (m_type)
	{
	case TOKEN_NUMBER:  
		{
			double v = m_value; 
			GetToken (true);  // get next one (one-token lookahead)
			return v;
		}

	case TOKEN_NAME:
		{
			std::string word = m_word;
			GetToken (true); 
			if (m_type == TOKEN_LHPAREN)
			{
				std::vector<std::string> params;
				while(true)
				{
					GetToken(true);
					if(m_type==TOKEN_RHPAREN)
						break;
					else if(m_type==TOKEN_STRING||m_type==TOKEN_NAME||m_type==TOKEN_NUMBER)
					{
						params.push_back( m_word );
						GetToken( true );
						if(m_type==TOKEN_RHPAREN)
							break;
						else if(m_type!=TOKEN_COMMA)
							return 0.0;
					}
				}
				GetToken();
				double v = 0.0;
				if( m_pfnCallback(word.c_str(), params, v) )
					return v;
				// evaluation function funname(params)
				return v;               // and return new value
			} // end of switch on type_              
		}

	case TOKEN_MINUS:               // unary minus
		return - Primary (true);

	case TOKEN_NOT:   // unary not
		return (Primary (true) == 0.0) ? 1.0 : 0.0;;

	case TOKEN_LHPAREN:
		{
			double v = CommaList (true);    // inside parens, you could have commas
			CheckToken (TOKEN_RHPAREN);
			GetToken ();                    // eat the )
			return v;
		}

	default:   
		throw std::runtime_error ("Unexpected token: " + m_word);

	} // end of switch on type
	throw std::runtime_error ("Unexpected token: " + m_word);
} // end of Parser::Primary 
