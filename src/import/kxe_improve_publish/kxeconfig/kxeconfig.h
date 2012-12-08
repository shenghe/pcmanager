//////////////////////////////////////////////////////////////////////
///		@file		kxeconfig.h
///		@author		luopeng
///		@date		2008-9-25 09:07:58
///	
///		@brief		kxeconfig相关定义
//////////////////////////////////////////////////////////////////////

#pragma once
#include "../kxebase/kxebase_config.h"
#include "../kxeerror/kxeerror.h"
#include <list>
#include <map>

/**
  @page kxeconfig_example KXEngine Config Example

  @code 
struct target
{
	std::wstring strName;
	int nType;
	int nId;	
	std::wstring strPath;
	std::wstring strComment;
};

KXE_CFG_DDX_BEGIN(target, strName)
	KXE_CFG_DDX_MEMBER_DEFAULT("Type", nType, 0)
	KXE_CFG_DDX_MEMBER_DEFAULT("Id", nId, 0)
	KXE_CFG_DDX_MEMBER_DEFAULT("Path", strPath, L"")
	KXE_CFG_DDX_MEMBER_DEFAULT("Comment", strComment, L"")
KXE_CFG_DDX_END()

struct target_tree
{
	std::wstring strName;
	std::map<std::wstring, target_tree> mapTargetTree;
};

KXE_CFG_DDX_BEGIN(target_tree, strName)
	KXE_CFG_DDX_MEMBER("target", mapTargetTree)
KXE_CFG_DDX_END()

struct aliases
{
	std::wstring strName;
	std::map<std::wstring, target> mapTarget;
};

KXE_CFG_DDX_BEGIN(aliases, strName)
	KXE_CFG_DDX_MEMBER("target", mapTarget)
KXE_CFG_DDX_END()


struct solution
{
	std::wstring strName;
	std::map<std::wstring, target_tree> mapTargetTree;
};

KXE_CFG_DDX_BEGIN(solution, strName)
	KXE_CFG_DDX_MEMBER("target", mapTargetTree)
KXE_CFG_DDX_END()

void TestFunction()
{
	kxe_base_init();
	kxe_base_start();

	target_tree parent;
	parent.strName = L"关键区域";

	target_tree child1;
	child1.strName = L"恶意软件";

	target_tree child2;
	child2.strName = L"System目录";

	target_tree child3;
	child3.strName = L"Windows目录";

	parent.mapTargetTree[child1.strName] = child1;
	parent.mapTargetTree[child2.strName] = child2;
	parent.mapTargetTree[child3.strName] = child3;

	solution sln;
	sln.strName = L"solution";
	sln.mapTargetTree[parent.strName] = parent;

	kxe_config_write_config(KXE_CURRENT_USER_CONFIG, L"ScanSystem", sln);

	sln.mapTargetTree.clear();


	target t1 = {L"关键区域", 0x20005, 0, L"", L""};
	target t2 = {L"System目录", 0x10003, 0, L"c:\\windows\\system32", L"包含子目录"}; 
	target t3 = {L"Windows目录", 0x10003, 0, L"c:\\windows", L"不包含子目录"}; 
	target t4 = {L"恶意软件", 0x10200, 0, L"", L"恶意软件"}; 

	aliases ali;
	ali.strName = L"aliases";
	ali.mapTarget[t1.strName] = t1;
	ali.mapTarget[t2.strName] = t2;
	ali.mapTarget[t3.strName] = t3;
	ali.mapTarget[t4.strName] = t4;

	kxe_config_write_config(KXE_CURRENT_USER_CONFIG, L"ScanSystem", ali);
	ali.mapTarget.clear();

	kxe_config_read_config(KXE_CURRENT_USER_CONFIG, L"ScanSystem", sln);
	kxe_config_read_config(KXE_CURRENT_USER_CONFIG, L"ScanSystem", ali);

	kxe_base_stop();
	kxe_base_fini();
}
	@endcode
*/


/**
 * @defgroup kxeconfig_group KXEngine Architecture Config Interface
 * @{
 */

/**
* @brief 根据路径,创建或打开一个指定的配置键
* @param[in] key 传入要打开的绝对键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_base_create_config_key或kxe_base_open_config_key创建出来的键
* @param[in] pwszPath 相对路径,最后打开的路径为key的path+pwszPath
* @param[out] pNewKey 成功打开后传出的配置键
* @remark 如果指定的键存在,则打开它,否则创建它
* @return 0 成功, 其他值为失败的错误码
*/
int kxe_config_create_key(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	kxe_config_key_t* pNewKey
	);

/**
* @brief 根据路径,打开一个指定的配置键
* @param[in] key 传入要打开的绝对键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_base_create_config_key或kxe_base_open_config_key创建出来的键
* @param[in] pwszPath 相对路径,最后打开的路径为key的path+pwszPath
* @param[out] pNewKey 成功打开后传出的配置键
* @remark 如果指定的键存在,则打开它,否则失败
* @return 0 成功, 其他值为失败的错误码
*/
int kxe_config_open_key(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	kxe_config_key_t* pNewKey
	);

/**
* @brief 关闭指定的配置键
* @param[in] key 通过kxe_config_create_key或kxe_config_open_key获得的键值
* @remark 
* @return 0 成功, 其他值为失败的错误码
*/
int kxe_config_close_key(
    kxe_config_key_t key
	);

/**
* @brief 删除指定的键值
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后路径为key的path+pwszPath
* @remark 
* @return 0 成功, 其他值为失败的错误码
*/
int kxe_config_delete_key(
	kxe_config_key_t key,
	const wchar_t* pwszPath
	);


/**
* @brief 通过指定的键及其相对路径,获取对应的数据结构
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in, out] value 传入需要获取数据的数据结构类型
* @remark 
*      传入的模板类型T,需要已经定义过KXE_CFG_DDX_BEGIN等相关宏,否则将获取空数据
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename T
>
int kxe_config_read_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	T& value
	);

/**
* @brief 通过指定的键及其相对路径,写入对应的数据结构
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] value 传入需要写入的数据结构类型
* @remark 
*      传入的模板类型T,需要已经定义过KXE_CFG_DDX_BEGIN等相关宏,否则写入空数据
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename T
>
int kxe_config_write_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const T& value
	);

#if _MSC_VER > 1200
/**
* @brief 通过指定的键及其相对路径,获取一列数据结构
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in, out] listValue 传入需要获取数据的数据结构列表类型
* @remark 
*    此模板类型T可以为KXE_CFG_DDX_BEGIN定义过的类型,也可以是定义过ostream与istream的
*    输入输出流类型
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename T
>
int kxe_config_read_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	std::list<T>& listValue
	);

/**
* @brief 通过指定的键及其相对路径,写入一列数据结构
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] listValue 传入需要写入数据的数据结构列表类型
* @remark 
*    此模板类型T可以为KXE_CFG_DDX_BEGIN定义过的类型,也可以是定义过ostream与istream的
*    输入输出流类型
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename T
>
int kxe_config_write_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const std::list<T>& listValue
	);
#endif

/**
* @brief 通过指定的键及其相对路径,获取一列数据结构,并用map表达
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in, out] mapKeyToType 传入需要获取数据的数据结构列表类型
* @remark 
*    此模板类型T可以为KXE_CFG_DDX_BEGIN定义过的类型,其map中的key为
*    KXE_CFG_DDX_BEGIN定义的数据
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename Key,
	typename T
>
int kxe_config_read_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	std::map<Key, T>& mapKeyToType
	);

/**
* @brief 通过指定的键及其相对路径,写入一列数据结构,并用map表达
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] mapKeyToValue 传入需要写入的数据结构列表类型
* @remark 
*    此模板类型T可以为KXE_CFG_DDX_BEGIN定义过的类型,其map中的key为
*    KXE_CFG_DDX_BEGIN定义的数据
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename Key,
	typename T
>
int kxe_config_write_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const std::map<Key, T>& mapKeyToValue
	);

/**
* @brief 通过指定的键及其相对路径,获取该键的所有子键
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in, out] listKey 传入存储获取到的所有子键
* @remark 
* @return 0 成功, 其他值为失败的错误码
*/
int kxe_config_read_key(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	std::list<std::wstring>& listKey
	);

/**
* @brief 通过指定的键及其相对路径,获取指定valuename的数据结构
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] pwszValueName 指定要获取的valuename
* @param[in, out] value 存储获取到的数据类型
* @remark 
*    此模板类型T需要定义过输入输出流
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename T
>
int kxe_config_read_value(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const wchar_t* pwszValueName,
	T& value
	);

/**
* @brief 通过指定的键及其相对路径,向指定valuename的写入数据
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] pwszValueName 指定要获取的valuename
* @param[in] value 存储的数据
* @remark 
*    此模板类型T需要定义过输入输出流
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename T
>
int kxe_config_write_value(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const wchar_t* pwszValueName,
	const T& value
	);

/**
* @brief 通过指定的键及其相对路径pwszPath及pwsaValueName获取一列数据结构
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] pwszValueName 与pwszPath组合相对路径
* @param[in, out] listValue 传入需要获取数据的数据结构
* @see 	kxe_config_read_config(kxe_config_key_t key, const wchar_t* pwszPath, std::list<T>& listValue);
* @remark 
*     此函数最终调用kxe_config_read_config
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename T
>
int kxe_config_read_value(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const wchar_t* pwszValueName,
	std::list<T>& listValue
	);

/**
* @brief 通过指定的键及其相对路径pwszPath及pwsaValueName写入一列数据结构
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] pwszValueName 与pwszPath组合相对路径
* @param[in, out] listValue 传入需要存储的数据
* @see kxe_config_write_config(kxe_config_key_t key,const wchar_t* pwszPath, const std::list<T>& listValue);
* @remark 
*     此函数最终调用kxe_config_write_config
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename T
>
int kxe_config_write_value(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const wchar_t* pwszValueName,
	const std::list<T>& listValue
	);

/**
* @brief 通过指定的键及其相对路径pwszPath与pwszValueName,获取一列数据结构,并用map表达
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] pwszValueName 与pwszPath组合相对路径
* @param[in, out] keyToValue 传入需要获取数据的数据结构列表类型
* @see kxe_config_read_config(kxe_config_key_t key, const wchar_t* pwszPath, std::map<Key, T>& mapKeyToType);
* @remark 
*     最终转换调用kxe_config_read_config
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename Key,
	typename T
>
int kxe_config_read_value(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const wchar_t* pwszValueName,
	std::map<Key, T>& keyToValue
	);

/**
* @brief 通过指定的键及其相对路径pwszPath与pwszValueName,获取一列数据结构,并用map表达
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_config_create_key或kxe_config_open_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] pwszValueName 与pwszPath组合相对路径
* @param[in, out] keyToValue 传入需要存储的数据
* @see kxe_config_write_config(kxe_config_key_t key,const wchar_t* pwszPath,const std::map<Key, T>& mapKeyToValue);
* @remark 
*     最终转换调用kxe_config_write_config
* @return 0 成功, 其他值为失败的错误码
*/
template
<
	typename Key,
	typename T
>
int kxe_config_write_value(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const wchar_t* pwszValueName,
	const std::map<Key, T>& keyToValue
	);

//template
//<
//	typename Class,
//	typename Left,
//	typename Right
//>
//bool KxEBaseIsSame(Left Class::* pLeft, Right Class::* pRight)
//{
//	return false;  // 类型不一样,当然false
//}
//
//template
//<
//	typename Class,
//	typename Left
//>
//bool KxEBaseIsSame(Left Class::* pLeft, Left Class::* pRight)
//{
//	return pLeft == pRight;
//}

template
<
	typename T,
	typename Key
>
inline int KxEConfigDataKey(kxe_config_key_t key, 
							bool bWrite, 
							const T& value, 
							Key T::* pMemberKey,
							kxe_config_key_t* pNewKey)
{
	std::wostringstream os;
	os << value.*pMemberKey;

	return kxe_config_create_key(
		key,
		os.str().c_str(),
		pNewKey
		);
}

template
<
	typename T
>
inline void KxEConfigSetKeyValue(T& object, const std::wstring& value)
{
	std::wistringstream is(value);
	is >> object;
}

template
<
	typename T
>
inline int KxEConfigDataExchange(kxe_config_key_t key, bool bWrite, T& value)
{
	return 0;
}

template
<
	typename T
>
struct KxEUserDefineConfigStruct
{
	enum {value = false};
};

/**
 * @brief 开始定义将数据结构与配置之间进行序列化与反序列化的中间数据
 * @param[in] _Class 需要定义中间数据的数据结构
 * @param[in] _Key _Class中用于作为Key的数据成员变量名
 */
#define KXE_CFG_DDX_BEGIN(_Class, _Key) \
	template\
	<\
	>\
	struct KxEUserDefineConfigStruct<_Class>\
	{\
		enum {value = true};\
	};\
	template<>\
	inline void KxEConfigSetKeyValue(_Class& object, const std::wstring& value)\
	{\
		std::wistringstream is(value);\
		is >> object.*(&_Class::_Key);\
	}\
	template<>\
	inline int KxEConfigDataExchange(kxe_config_key_t key, bool bWrite, _Class& value)\
	{\
		kxe_config_key_t newKey = NULL;\
		int nRet = KxEConfigDataKey(key, bWrite, value, &_Class::_Key, &newKey);\
		if (nRet != 0) \
		{\
			return nRet;\
		}\
		key = newKey;\

/**
* @brief 定义将数据结构与配置之间进行序列化与反序列化的中间数据中的数据成员,
*        只能用于通地KXE_CFG_DDX_BEGIN定义的数据
* @param[in] _Name 存储时将用的ValueName
* @param[in] _Member 数据结构的成员变量名
*/
#define KXE_CFG_DDX_MEMBER(_Name, _Member)\
	if (bWrite) kxe_config_write_config(key, L##_Name, value._Member);\
	else kxe_config_read_config(key, L##_Name, value._Member);

/**
* @brief 定义将数据结构与配置之间进行序列化与反序列化的中间数据中的数据成员,
*        如果没有找到值,则用_Default填充
* @param[in] _Name 存储时将用的ValueName
* @param[in] _Member 数据结构的成员变量名
* @param[in] _Default 在读取数据时,如果没有找到对应的ValueName,则用_Default填充
*/
#define KXE_CFG_DDX_MEMBER_DEFAULT(_Name, _Member, _Default)\
	if (bWrite)\
	{\
		kxe_config_write_value(key, NULL, L##_Name, value._Member);\
	}\
	else \
	{\
		nRet = kxe_config_read_value(key, NULL, L##_Name, value._Member);\
		if (nRet == ERROR_FILE_NOT_FOUND || nRet == E_KXECFG_VALUE_NOT_CORRECT) \
		{\
			value._Member = _Default;\
		}\
	}

/**
* @brief 结束定义将数据结构与配置之间进行序列化与反序列化的中间数据
*/
#define KXE_CFG_DDX_END() \
	if (newKey) \
	{\
		kxe_config_close_key(newKey); \
	}\
	return 0;\
	}

/**
* @brief 直接将一个struct定义为一个键
*/
#define KXE_CFG_DDX_DEFINE(_Class) \
	template\
	<\
	>\
	struct KxEUserDefineConfigStruct<_Class>\
	{\
		enum {value = true};\
	};\
	template<>\
	inline int KxEConfigDataExchange(kxe_config_key_t key, bool bWrite, _Class& value)\
	{\
		std::wostringstream os;\
		os << value;\
		kxe_config_key_t newKey = NULL;\
		int nRet = kxe_config_create_key(key, os.str().c_str(), &newKey);\
		if (nRet != 0)\
		{\
			return nRet;\
		}\
		else\
		{\
			kxe_config_close_key(newKey);\
			return 0;\
		}\
	}
		
// 内联实现
#include "kxeconfig.inl"

/**
 * @}
 */
