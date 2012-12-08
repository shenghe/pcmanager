/**
* @file    bkdbplat.h
* @brief   ...
* @author  bbcallen
* @date    2010-01-26 16:30
*/

#ifndef BKDBPLAT_H
#define BKDBPLAT_H

NS_SKYLARK_BEGIN

class CBKDbPlat
{
public:
    static CBKDbPlat& GetInstance();

    CBKDbPlat();
    ~CBKDbPlat();

protected:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;


    CObjLock m_objLock;
};

NS_SKYLARK_END

#endif//BKDBPLAT_H