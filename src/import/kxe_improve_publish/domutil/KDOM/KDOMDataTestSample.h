/********************************************************************
* CreatedOn: 2007-1-9   15:53
* FileName:  KDOMDataTestSample.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KDOMDataTestSample_H_
#define __KDOMDataTestSample_H_

#include "KDOMMacro.h"

//////////////////////////////////////////////////////////////////////////
class KTestPoint
{
public:
    int m_x;
    int m_y;
};

inline bool operator==(const KTestPoint& lValue, const KTestPoint& rValue)
{
    return
        lValue.m_x == rValue.m_x &&
        lValue.m_y == rValue.m_y;
}

inline KTestPoint MakeDefault_KTestPoint()
{
    KTestPoint testPoint = {0, 0};
    return testPoint;
}

KDX_BEGIN(KTestPoint);
    KDX_OBJ_NM("x", m_x);
    KDX_OBJ_NM("y", m_y);
KDX_END();



//////////////////////////////////////////////////////////////////////////
class KTestLine
{
public:
    KTestPoint m_vertexBegin;
    KTestPoint m_vertexEnd;
};

inline bool operator==(const KTestLine& lValue, const KTestLine& rValue)
{
    return
        lValue.m_vertexBegin == rValue.m_vertexBegin &&
        lValue.m_vertexEnd   == rValue.m_vertexEnd;
}

inline KTestLine MakeDefault_KTestLine()
{
    KTestLine testLine = {MakeDefault_KTestPoint(), MakeDefault_KTestPoint()};
    return testLine;
}

KDX_BEGIN(KTestLine);
    KDX_OBJ_NM("begin", m_vertexBegin);
    KDX_OBJ_EX("end",   m_vertexEnd,    MakeDefault_KTestPoint());
KDX_END();




//////////////////////////////////////////////////////////////////////////
class KTestTriangle
{
public:
    KTestPoint m_vertexA;
    KTestPoint m_vertexB;
    KTestPoint m_vertexC;
};

inline bool operator==(const KTestTriangle& lValue, const KTestTriangle& rValue)
{
    return
        lValue.m_vertexA == rValue.m_vertexA &&
        lValue.m_vertexB == rValue.m_vertexB &&
        lValue.m_vertexC == rValue.m_vertexC;
}

inline KTestTriangle MakeDefault_KTestTriangle()
{
    KTestTriangle testTriangle = {
        MakeDefault_KTestPoint(),
        MakeDefault_KTestPoint(),
        MakeDefault_KTestPoint()
    };
    return testTriangle;
}

KDX_BEGIN(KTestTriangle);
    KDX_OBJ_EX("vertexA", m_vertexA,  MakeDefault_KTestPoint());
    KDX_OBJ_EX("vertexB", m_vertexB,  MakeDefault_KTestPoint());
    KDX_OBJ_EX("vertexC", m_vertexC,  MakeDefault_KTestPoint());
KDX_END();




class KTestVertexVector
{
public:
    std::vector<std::vector<KTestPoint> > m_vertexList;
};

KDX_BEGIN(KTestVertexVector);
    KDX_OBJ_NM("vertex_list", m_vertexList);
KDX_END();


class KTestVertexList
{
public:
    std::list<std::vector<KTestPoint> > m_vertexList;
};

KDX_BEGIN(KTestVertexList);
    KDX_OBJ_NM("vertex_list", m_vertexList);
KDX_END();


class KTestPointMap
{
public:
    std::map<int, int> m_pointMap;
};

KDX_BEGIN(KTestPointMap);
    KDX_OBJ_NM("point_map", m_pointMap);
KDX_END();



#endif//__KDOMDataTestSample_H_
