/**
* @file    bkdbplat.cpp
* @brief   ...
* @author  bbcallen
* @date    2010-01-26 16:30
*/

#include "stdafx.h"
#include "bkdbplat.h"

NS_SKYLARK_USING

CBKDbPlat& CBKDbPlat::GetInstance()
{
    static CBKDbPlat s_plat;

    return s_plat;
}

CBKDbPlat::CBKDbPlat()
{
    BKInitDebugOutput();
}

CBKDbPlat::~CBKDbPlat()
{

}