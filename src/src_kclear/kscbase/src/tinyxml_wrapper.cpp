/********************************************************************
	created:	2010/06/05
	created:	5:6:2010   17:44
	filename: 	tinyxml_wrapper.cpp
	author:		Jiang Fengbing
	
	purpose:	tinyxml的包裹，用来消除警告
*********************************************************************/

#pragma warning(push)
#pragma warning(disable: 6011)
#pragma warning(disable: 6031)
#pragma warning(disable: 6244)

#include "tinystr.cpp"
#include "tinyxmlerror.cpp"
#include "tinyxmlparser.cpp"
#include "tinyxml.cpp"

#pragma warning(pop)
