/********************************************************************
* CreatedOn: 2006-12-11   11:13
* FileName:  KDOMBasic.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KDOMBasic_H_
#define __KDOMBasic_H_

//#include "..\..\Common\KAVPublic.h"
#include "..\KMPCommon\KConv.h"
#include "..\..\kxeerror\kxeerror.h"

#include <vector>

#ifndef _KAN_DOM_NAMESPACE

#define _KAN_DOM_NAMESPACE
#define _KAN_DOM_BEGIN  namespace KANDOM {
#define _KAN_DOM_END    };

#endif//_KAN_DOM_BEGIN


_KAN_DOM_BEGIN


//////////////////////////////////////////////////////////////////////////
// 将形如"abcd/efg/hij"的路径字符串解析为包含"abcd","efg","hij"三个元素的列表
inline int KDOMPhrasePath(const char* pcszPath, std::vector<std::string>& pathItemList)
{
    assert(pcszPath);

    const char* iter = pcszPath;
    std::string strItem;

    while (*iter != '\0')
    {
//        strItem.clear();
		strItem = "";
        while (*iter != '/' && *iter != '\0')
        {
            //strItem.push_back(*iter++);
			strItem += (*iter++);
        }

        pathItemList.push_back(strItem);

        if (*iter == '/')
        {
            ++iter;
        }
    }

    return 0;
}

#define E_KXML_FAIL_TO_LOAD_FILE    E_KXEIDL_XML_FAIL_TO_LOAD_FILE
#define E_KXML_FAIL_TO_LOAD_TEXT    E_KXEIDL_XML_FAIL_TO_LOAD_TEXT
#define E_KXML_FAIL_TO_LOAD_ROOT    E_KXEIDL_XML_FAIL_TO_LOAD_ROOT
#define E_KXML_FAIL_TO_SAVE_FILE    E_KXEIDL_XML_FAIL_TO_SAVE_FILE
#define E_KXML_FAIL_TO_GEN_TEXT     E_KXEIDL_XML_FAIL_TO_GEN_TEXT

#define E_KXML_PARENT_NODE_IS_NULL  E_KXEIDL_XML_PARENT_NODE_IS_NULL
#define E_KXML_CURRENT_NODE_IS_NULL E_KXEIDL_XML_CURRENT_NODE_IS_NULL

#define E_KXML_FAIL_TO_REMOVE_NODE      E_KXEIDL_XML_FAIL_TO_REMOVE_NODE

#define E_KXML_FAIL_TO_GET_NODE_NAME    E_KXEIDL_XML_FAIL_TO_GET_NODE_NAME
#define E_KXML_FAIL_TO_GET_NODE_TEXT    E_KXEIDL_XML_FAIL_TO_GET_NODE_TEXT
#define E_KXML_FAIL_TO_SET_NODE_TEXT    E_KXEIDL_XML_FAIL_TO_SET_NODE_TEXT

#define E_KXML_VALUE_STREAM_ERROR   E_KXEIDL_XML_VALUE_STREAM_ERROR

#define E_KXML_XMLMANAGER_EXPECTED  E_KXEIDL_XML_XMLMANAGER_EXPECTED

#define E_KXML_SCOPE_STACK_IS_EMPTY  E_KXEIDL_XML_SCOPE_STACK_IS_EMPTY

#define E_KXML_FAIL_TO_INIT_CURSOR  E_KXEIDL_XML_FAIL_TO_INIT_CURSOR

#define E_KXML_FAIL_TO_LOAD_DEST_DOM_ROOT E_KXEIDL_XML_FAIL_TO_LOAD_DEST_DOM_ROOT
#define E_KXML_FAIL_TO_SET_DEST_DOM_VALUE E_KXEIDL_XML_FAIL_TO_SET_DEST_DOM_VALUE
#define E_KXML_FAIL_TO_COPY_NODE_TO_DEST_DOM E_KXEIDL_XML_FAIL_TO_COPY_NODE_TO_DEST_DOM

#define E_KXML_INVALID_CURSOR_STATE E_KXEIDL_XML_INVALID_CURSOR_STATE


#define E_KJSON_FAIL_TO_LOAD_FILE   E_KXEIDL_JSON_FAIL_TO_LOAD_FILE
#define E_KJSON_FAIL_TO_LOAD_TEXT   E_KXEIDL_JSON_FAIL_TO_LOAD_TEXT
#define E_KJSON_FAIL_TO_LOAD_ROOT   E_KXEIDL_JSON_FAIL_TO_LOAD_ROOT
#define E_KJSON_FAIL_TO_SAVE_FILE   E_KXEIDL_JSON_FAIL_TO_SAVE_FILE


#define E_KJSON_PARENT_NODE_IS_NULL E_KXEIDL_JSON_PARENT_NODE_IS_NULL
#define E_KJSON_CURRENT_NODE_IS_NULL E_KXEIDL_JSON_CURRENT_NODE_IS_NULL
#define E_KJSON_ROOT_NODE_IS_NULL   E_KXEIDL_JSON_ROOT_NODE_IS_NULL

#define E_KJSON_EXPECT_STRING_TYPE  E_KXEIDL_JSON_EXPECT_STRING_TYPE

#define E_KJSON_VALUE_STREAM_ERROR  E_KXEIDL_JSON_VALUE_STREAM_ERROR

#define E_KJSON_FAIL_TO_GET_NODE_NAME       E_KXEIDL_JSON_FAIL_TO_GET_NODE_NAME
#define E_KJSON_FAIL_TO_GET_ROOT_NAME       E_KXEIDL_XML_FAIL_TO_GET_ROOT_NAME
#define E_KJSON_FAIL_TO_LOAD_DEST_DOM_ROOT  E_KXEIDL_JSON_FAIL_TO_LOAD_DEST_DOM_ROOT

#define E_KJSON_FAIL_TO_INIT_CURSOR E_KXEIDL_JSON_FAIL_TO_INIT_CURSOR

#define E_KJSON_FAIL_TO_COPY_NODE_TO_DEST_DOM E_KXEIDL_JSON_FAIL_TO_COPY_NODE_TO_DEST_DOM

#define E_KJSON_SCOPE_STACK_IS_EMPTY  E_KXEIDL_JSON_SCOPE_STACK_IS_EMPTY



#define E_KDOM_DATA_OUT_OF_RANGE                E_KXEIDL_KDOM_DATA_OUT_OF_RANGE
#define E_KDOM_DATA_TYPE_DO_NOT_SUPPORT_WRITING E_KXEIDL_KDOM_DATA_TYPE_DO_NOT_SUPPORT_WRITING



_KAN_DOM_END


#endif//__KDOMBasic_H_

