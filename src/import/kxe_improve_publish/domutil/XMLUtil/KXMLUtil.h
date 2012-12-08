/********************************************************************
* CreatedOn: 2006-11-23   15:32
* FileName:  KXMLUtil.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KXMLUtil_H_
#define __KXMLUtil_H_

#include "KXMLDataAccess.h"


//////////////////////////////////////////////////////////////////////////
//
// 此代码主要用于将任意类型数据(内建或自定义)进行XML序列化
// 主要的接口类是KXMLDataAccess，详见KXMLDataAccess.h

//////////////////////////////////////////////////////////////////////////
// 一个典型的结构体如果要支持xml数据交换有两种办法
//
// 1,使用KDX_XXX宏
// 
//
// Example:
//
//class KTestPoint
//{
//public:
//    int m_x;
//    int m_y;
//};
//
//KDX_BEGIN(KTestPoint);
//    KDX_OBJ_EX("x", m_x, 0);
//    KDX_OBJ_NM("y", m_y);
//KDX_END();
//
//=== 宏的说明 =========================================================
//
// 上面的例子中,KDX_BEGIN()的参数指定类要进行交换的类
// "x"和"y"指定了元素的名称,会成员XML的结点名
//
// m_x和m_y必须是一个左值表达式,用来与XML进行进行读/写交换
//
// KDX_OBJ_EX()的第三个参数是一个与第二参数基础类型相同的右值表达式,
// 它指定了读XML失败时,对应成员的缺省值
// EX代表Extra
//
// KDX_OBJ_NM()没有指定缺省值的参数,当读失败时,直接返回失败
// NM代表Normal
// 不过也可以理解为Nightmare,读XMl真的是个Nightmare (n_n)
//
//======================================================================
//
//class KTestLine
//{
//public:
//    KTestPoint m_vertexBegin;
//    KTestPoint m_vertexEnd;
//};
//
//KTestPoint MakeDefault_KTestPoint()
//{
//    KTestPoint testPoint = {0, 0};
//    return testPoint;
//}
//
//KDX_BEGIN(KTestLine);
//    KDX_OBJ_EX("begin", m_vertexBegin,  MakeDefault_KTestPoint());
//    KDX_OBJ_EX("end",   m_vertexEnd,    MakeDefault_KTestPoint());
//KDX_END();
//
//
//=== 缺省值的再讨论 ===================================================
//
// 对于内建(Build in)类型,可以很容易可以指定一个常量作为缺省值
// 但是对于结构体(struct)和类(class),必须有另外的办法
//
// 上面的例子是指定了一个函数调用作为表达式
// 另外还可以用类型转换,构造函数等办法
// 只要是和第二参数类型相同的右值表达式,都可以用在这里
//
// 要注意的是,这里不能使用隐式类型转换,必须明确的描述出表达式的类型
//
//======================================================================
//
//
//


#endif//__KXMLUtil_H_
