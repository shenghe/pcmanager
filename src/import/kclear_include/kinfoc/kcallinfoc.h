#pragma once
#include <wtypes.h>
#include <string>
#include <deque>
using namespace std;

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

enum enumSendMode
{
	enumSendByGet = 1,
	enumSendByPost = 2,
};

typedef BOOL (__stdcall *DllSend)(int);
typedef BOOL (__stdcall *DllAddData)(LPCWSTR, LPCWSTR);
typedef BOOL (__stdcall *DllSetPublic)(LPCWSTR, LPCWSTR);
typedef void (__stdcall *DllCheckAndDelFiles)(int);
typedef BOOL (__stdcall *DllGetUUID)(LPWSTR, DWORD&);


class KCallInfoc
{
public:
	KCallInfoc(void);
	~KCallInfoc(void);

public:
	/*
	* @brief								初始化，用来找到kinfoc.dll的位置并加载，可以调用，也可以不调用。
	* @param lpStrFilePath				--- 文件路径，默认为去自己去查找
	* @return 
	*							   TRUE --- 成功
	*							   FALSE--- 失败
	*/
	BOOL Init(IN const wchar_t* lpStrFilePath = NULL);

	/*
	* @brief								反初始化
	* @return 
	*							   TRUE --- 成功
	*							   FALSE--- 失败
	*/
	BOOL Unit();

	/*
	* @brief								添加公共文件，如果该文件存在，则会替换
	* @param lpStrActionName			--- 动作名，如duba_install(毒霸安装)
	* @param lpStrData					---	需要添加到文件中的数据,每个列使用'&'隔开(eg:a=1&b=2&c=3)
	* @param lpStrFilePath				--- 指定的kinfoc的文件的路径，默认为空
	* @return 
	*							   TRUE --- 添加成功
	*							   FALSE--- 添加失败
	*/
	BOOL SetPublicFile(IN const wchar_t* lpStrPublicFileName, IN const wchar_t* lpStrPublicData, IN const wchar_t * lpStrFilePath = NULL);
	
	/*
	* @brief								添加数据
	* @param lpStrActionName			--- 动作名，如duba_install(毒霸安装)
	* @param lpStrData					---	需要添加到文件中的数据,每个列使用'&'隔开(eg:a=1&b=2&c=3)
	* @param lpStrFilePath				--- 指定的kinfoc的文件的路径，默认为空
	* @return 
	*							   TRUE --- 添加成功
	*							   FALSE--- 添加失败
	*/
	BOOL AddData(IN const wchar_t * lpStrActionName, IN const wchar_t * lpStrData, IN const wchar_t * lpStrFilePath = NULL);

	/*
	* @brief								发送数据
	* @param enumMode					--- 指定发送的模式，默认为使用GET的方式发送数据
	* @param lpStrFilePath				--- 指定的kinfoc的文件的路径，默认为空
	* @return 
	*							   TRUE --- 发送成功
	*							   FALSE--- 发送失败
	*/
	BOOL SendData(IN enumSendMode enumMode = enumSendByGet, IN const wchar_t * lpStrFilePath = NULL );

	/*
	* @brief								删除所有的文件，当文件夹中的文件超过某一个数时
	* @param nNum						--- 设置当文件超过多少个数时候，删除文件
	* @param lpStrFilePath				--- 指定的kinfoc的文件的路径，默认为空
	*/
	void CheckAndDelFiles(IN int nNum, IN const wchar_t * lpStrFilePath = NULL);

	BOOL AddDataEx(IN const wchar_t * lpStrActionName, IN const wchar_t * lpStrData);

private:
	BOOL _GetKInfocPath(OUT std::wstring& strInfocPath);
	BOOL _CheckFileFromReg(OUT std::wstring& strFilePath);
	BOOL _ReadOneRegKey(OUT wchar_t* lpszResult, IN long dwOutMaxSize);

private:
	HMODULE m_hMoudle;
	BOOL m_bIsInit;

private:
	DllSetPublic m_pSendData;
	DllAddData m_pAddData;
	DllSend m_pSend;
	DllCheckAndDelFiles m_pCheckAndDelFiles;
	DllGetUUID m_GetUUID;
};
