/**
* @file    instmanager.h
* @brief   ...
* @author  bbcallen
* @date    2010-03-24 16:04
*/

#ifndef INSTMANAGER_H
#define INSTMANAGER_H

class CInstManager
{
public:
    static HRESULT LoadAvEngine();

    static HRESULT UnloadAvEngine();

    static HRESULT ReloadAvSign();

    static HRESULT UpdateWhiteList();
};

#endif//INSTMANAGER_H