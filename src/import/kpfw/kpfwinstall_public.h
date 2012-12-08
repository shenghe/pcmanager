/*
created by :chl chenliang@kingsoft.com

网镖安装接口
*/
#ifndef _KPFWINSTALL_PUBLIC_H_
#define _KPFWINSTALL_PUBLIC_H_

#ifdef __cplusplus

#include <Unknwn.h>

#endif //__cpluspluss


typedef enum enumErrorCode
{
	ErrorRegCoComponent = 0x01,
	ErrorCheckKpfwSvr,
	ErrorInstallKpfwSvr,
	ErrorUnInstallKpfwSvr,
	ErrorInstallDriver,
	ErrorUnInstallDriver,
	ErrorActiveAll,
	ErrorSuccess,
	ErrorFalied
}ERRORCODE;

typedef enum enumInstallStep
{
	INS_Initialize		= 0x01,
	INS_CheckSvr,
	INS_RegCoComponent,
	INS_InstallKpfwSvr,
	INS_InstallDriver,
	INS_ActiveAll,
	INS_DeActiveAll,
	INS_UnInstallDriver,
	INS_UnInstallKpfwSvr,
	INS_UnRegCoComponent,
	INS_UnInstallFinish,
	INS_InstallFinish
}INSTALLSTEP;

typedef enum enumInstallType
{
	enumLocalInstall = 0x01,
	enumLocalUnInstall,
	enumSwitchVer,
	enumUnKnown
}INSTALLTYPE;
//------------------------------------------------------------------------------

#ifdef __cplusplus

interface IKpfwInstallerCallBack;

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
[
	uuid("D79DA92D-AB93-4bc4-B11A-025C6BC94FF9")
]
#else//_MSC_VER
interface __declspec(uuid("D79DA92D-AB93-4bc4-B11A-025C6BC94FF9")) IKpfwInstaller;
#endif//_MSC_VER

interface IKpfwInstaller : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Init(INSTALLTYPE eInstallType) PURE;

	virtual HRESULT STDMETHODCALLTYPE SetInstallCallBack(IKpfwInstallerCallBack* ipCallBack) PURE;

	virtual HRESULT STDMETHODCALLTYPE StartInstall() PURE;

	virtual HRESULT STDMETHODCALLTYPE UnInit() PURE;

	virtual HRESULT STDMETHODCALLTYPE PauseInstall() PURE;

	virtual HRESULT STDMETHODCALLTYPE ResumeInstall() PURE;

	virtual HRESULT STDMETHODCALLTYPE StopInstall() PURE;

	virtual HRESULT STDMETHODCALLTYPE CheckKpfwSvrExist(LPCTSTR lpcszSvrName, BOOL* pbExist) PURE;

	virtual HRESULT STDMETHODCALLTYPE CheckFireWallConflict(BOOL& bConflict) PURE;

	virtual HRESULT STDMETHODCALLTYPE SwitchtoOldKpfw() PURE;

	virtual HRESULT STDMETHODCALLTYPE SwitchtoNewKpfw() PURE;
};


#if defined(_MSC_VER) && (_MSC_VER >= 1300)
[
	uuid("279F6B17-539E-472f-A078-C7982490000A")
]
#else//_MSC_VER
interface __declspec(uuid("279F6B17-539E-472f-A078-C7982490000A")) IKpfwInstallerCallBack;
#endif//_MSC_VER

interface IKpfwInstallerCallBack : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE OnError(ERRORCODE eError) PURE;
	
	virtual HRESULT STDMETHODCALLTYPE OnStep(INSTALLSTEP eProgress) PURE;
};

#endif //__cplusplus


#endif