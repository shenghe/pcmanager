/** 
* @file     ikrescanspmanager.h
* @author   jiaoyafei
* @date     2010-11-11
* @brief    the interface for service provider manager
*/

#ifndef _I_KRESCAN_SP_MANAGER_H__
#define _I_KRESCAN_SP_MANAGER_H__


__declspec(selectany) GUID CLSID_CKRescanSP = 
{ 0xc21140f0, 0x3a5b, 0x4c1f, { 0xae, 0x69, 0x4d, 0x86, 0x2b, 0xf7, 0xbd, 0xe8 } };


struct IReScanCallback;

MIDL_INTERFACE("A4D908A6-9AD9-4557-97B4-76BCD7652FE5")
IKRescanSPManager
{
    /**
    * @brief    初始化Service Provider,由服务管理器调用
    * @return   0 为成功, 其它为失败
    */
    virtual int __stdcall InitializeService(IReScanCallback* iRescanCallback) = 0;

    /**
    * @brief 反初始化Service Provider,由服务管理器调用
    * @return 0 为成功, 其它为失败
    */
    virtual int __stdcall UninitializeService() = 0;

    /**
    * @brief 启动Service Provider,由服务管理器调用
    * @return 0 为成功, 其它为失败
    */
    virtual int __stdcall StartService() = 0;

    /**
    * @brief 关闭Service Provider,由服务管理器调用
    * @return 0 为成功, 其它为失败
    */
    virtual int __stdcall StopService() = 0;
};

#endif //> _I_KRESCAN_SP_MANAGER_H__