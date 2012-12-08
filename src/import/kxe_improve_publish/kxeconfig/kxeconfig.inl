//////////////////////////////////////////////////////////////////////
///		@file		kxeconfig.inl
///		@author		luopeng
///		@date		2008-9-25 09:07:58
///	
///		@brief		kxeconfig相关定义
//////////////////////////////////////////////////////////////////////

#pragma once
#include "kxeconfig.h"
#include "../kxeerror/kxeerror.h"
#include "../scom/scom/scom.h"
#include "../scom/scom/scomhelper.h"

template
<
	typename T
>
inline void KxEStringToType(const wchar_t* pwszBuffer,
							T& value)
{
	std::wistringstream is(pwszBuffer);
	is >> value;
}

template
<
>
inline void KxEStringToType(const wchar_t* pwszBuffer,
							std::wstring& value)
{
	value = pwszBuffer;
}


inline int kxe_config_create_key(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	kxe_config_key_t* pNewKey
	)
{
	return kxe_base_create_config_key(
		key,
		pwszPath,
		pNewKey
		);
}

inline int kxe_config_open_key(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	kxe_config_key_t* pNewKey
	)
{
	return kxe_base_open_config_key(
		key,
		pwszPath,
		pNewKey
		);
}

inline int kxe_config_close_key(
    kxe_config_key_t key
	)
{
	return kxe_base_close_config_key(key);
}

inline int kxe_config_delete_key(
	kxe_config_key_t key,
	const wchar_t* pwszPath
	)
{
	return kxe_base_delete_config_key(key, pwszPath);
}

inline int kxe_config_read_key(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	std::list<std::wstring>& listKey
	)
{
	std::vector<wchar_t> vecBuffer(1024);

	kxe_config_key_t newKey = NULL;
	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}

	int nReturnBuffer = 0;
	unsigned int uIndex = 0;
	while (1)
	{
		nRet = kxe_base_enum_config_key(
			key,
			uIndex,
			&vecBuffer[0],
			(int)vecBuffer.size(),
			&nReturnBuffer
			);
		if (nRet == ERROR_MORE_DATA)
		{
			vecBuffer.resize(nReturnBuffer + 1);
		}
		else if (nRet == ERROR_NO_MORE_ITEMS)
		{
			nRet = 0;
			break;
		}
		else if (nRet != 0)
		{
			break;
		}

		listKey.push_back(&vecBuffer[0]);
		uIndex++;
	}
	return nRet;
}

template
<
	typename T
>
inline int kxe_config_read_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	T& value
	)
{
	kxe_config_key_t newKey = NULL;

	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}

	nRet = KxEConfigDataExchange(key, false, const_cast<T&>(value));

	if (newKey)
	{
		kxe_base_close_config_key(newKey);
	}


	return nRet;
}

template
<
	typename T
>
inline int kxe_config_write_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const T& value
	)
{
	STATIC_CHECK(KxEUserDefineConfigStruct<T>::value, KxENotUserDefineStruct);

	kxe_config_key_t newKey = NULL;

	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}
	nRet = KxEConfigDataExchange(key, true, const_cast<T&>(value));

	if (newKey)
	{
		kxe_base_close_config_key(newKey);
	}

	return nRet;
}

#if _MSC_VER > 1200
 template
 <
 	typename T
 >
 int kxe_config_read_config(
 	kxe_config_key_t key,
 	const wchar_t* pwszPath, 
 	std::list<T>& listValue
 	)
 {
 	kxe_config_key_t newKey = NULL;
 
 	int nRet = 0;
 	if (pwszPath)
 	{
 		nRet = kxe_base_create_config_key(
 			key,
 			pwszPath,
 			&newKey
 			);
 		if (nRet != 0)
 		{
 			return nRet;
 		}
 		key = newKey;
 	}
 
 	
 	std::list<std::wstring> listKey;
 	nRet = kxe_config_read_key(key, NULL, listKey);
 	if (nRet == 0)
 	{
 		std::list<std::wstring>::iterator iter = listKey.begin();
 
 		T value = T();
 		for (; iter != listKey.end(); ++iter)
 		{
 			KxEConfigSetKeyValue(value, *iter);
 			int nRet = kxe_config_read_config(
 				key,
 				NULL,
 				value
 				);
 			if (nRet != 0)
 			{
 				break;
 			}
 			listValue.push_back(value);
 		}
 	}
 
 	if (newKey)
 	{
 		kxe_config_close_key(newKey);
 	}
 
 	return nRet;
 }

 template
 <
 	typename T
 >
 int kxe_config_write_config(
 	kxe_config_key_t key,
 	const wchar_t* pwszPath,
 	const std::list<T>& listValue
 	)
 {
 	kxe_config_key_t newKey = NULL;
 
 	int nRet = 0;
 	if (pwszPath)
 	{
 		nRet = kxe_base_create_config_key(
 			key,
 			pwszPath,
 			&newKey
 			);
 		if (nRet != 0)
 		{
 			return nRet;
 		}
 		key = newKey;
 	}
 
 	
 	std::list<T>::const_iterator iter = listValue.begin();
 	for (; iter != listValue.end(); ++iter)
 	{
 		nRet = kxe_config_write_config(
 			key,
 			NULL,
 			*iter
 			);
 		if (nRet != 0)
 		{
 			break;
 		}
 	}
 	
 	if (newKey)
 	{
 		kxe_config_close_key(newKey);
 	}
 
 	return nRet;
 }

#endif

///// 读取一个列表结构
//template
//<
//	typename T
//>
//int kxe_config_read_config(
//	kxe_config_key_t key,
//	const wchar_t* pwszPath, 
//	std::list<T*>& listValue
//	)
//{
//	kxe_config_key_t newKey = NULL;
//
//	int nRet = 0;
//	if (pwszPath)
//	{
//		nRet = kxe_base_create_config_key(
//			key,
//			pwszPath,
//			&newKey
//			);
//		if (nRet != 0)
//		{
//			return nRet;
//		}
//		key = newKey;
//	}
//
//
//	std::list<std::wstring> listKey;
//	nRet = kxe_config_read_key(key, NULL, listKey);
//	if (nRet == 0)
//	{
//		std::list<std::wstring>::iterator iter = listKey.begin();
//
//		//T value = T();
//		for (; iter != listKey.end(); ++iter)
//		{
//			T* pValue = new T;
//			KxEConfigSetKeyValue(*pValue, *iter);
//			int nRet = kxe_config_read_config(
//				key,
//				NULL,
//				*pValue
//				);
//			if (nRet != 0)
//			{
//				delete pValue;
//				break;
//			}
//			listValue.push_back(pValue);
//		}
//	}
//
//	if (newKey)
//	{
//		kxe_config_close_key(newKey);
//	}
//
//	return nRet;
//}
//
///// 写入一个结构
//template
//<
//	typename T
//>
//int kxe_config_write_config(
//	kxe_config_key_t key,
//	const wchar_t* pwszPath,
//	const std::list<T*>& listValue
//	)
//{
//	kxe_config_key_t newKey = NULL;
//
//	int nRet = 0;
//	if (pwszPath)
//	{
//		nRet = kxe_base_create_config_key(
//			key,
//			pwszPath,
//			&newKey
//			);
//		if (nRet != 0)
//		{
//			return nRet;
//		}
//		key = newKey;
//	}
//
//
//	std::list<T*>::const_iterator iter = listValue.begin();
//	for (; iter != listValue.end(); ++iter)
//	{
//		nRet = kxe_config_write_config(
//			key,
//			NULL,
//			*(*iter)
//			);
//		if (nRet != 0)
//		{
//			break;
//		}
//	}
//
//	if (newKey)
//	{
//		kxe_config_close_key(newKey);
//	}
//
//	return nRet;
//}

template
<
	typename Key,
	typename T
>
inline int kxe_config_read_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	std::map<Key, T>& mapKeyToType)
{
	kxe_config_key_t newKey = NULL;

	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}


	std::list<std::wstring> listKey;
	nRet = kxe_config_read_key(key, NULL, listKey);
	if (nRet == 0)
	{
		std::list<std::wstring>::iterator iter = listKey.begin();

		T value = T();
		for (; iter != listKey.end(); ++iter)
		{
			KxEConfigSetKeyValue(value, *iter);
			int nRet = kxe_config_read_config(
				key,
				NULL,
				value
				);
			if (nRet != 0)
			{
				break;
			}

			std::wistringstream is(*iter);
			Key key;
			is >> key;
			mapKeyToType[key] = value;
		}
	}

	if (newKey)
	{
		kxe_config_close_key(newKey);
	}

	return nRet;
}

template
<
	typename Key,
	typename T
>
inline int kxe_config_write_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const std::map<Key, T>& mapKeyToValue
	)
{
	kxe_config_key_t newKey = NULL;

	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}


	std::map<Key, T>::const_iterator iter = mapKeyToValue.begin();
	for (; iter != mapKeyToValue.end(); ++iter)
	{
		nRet = kxe_config_write_config(
			key,
			NULL,
			iter->second
			);
		if (nRet != 0)
		{
			break;
		}
	}

	if (newKey)
	{
		kxe_config_close_key(newKey);
	}

	return nRet;
}

#if _MSC_VER > 1200

template
<
	typename Key,
	typename T
>
inline int kxe_config_read_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	std::map<Key, T*>& mapKeyToType
	)
{
	kxe_config_key_t newKey = NULL;

	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}


	std::list<std::wstring> listKey;
	nRet = kxe_config_read_key(key, NULL, listKey);
	if (nRet == 0)
	{
		std::list<std::wstring>::iterator iter = listKey.begin();

		for (; iter != listKey.end(); ++iter)
		{
			T* pTypeValue = new T;
			KxEConfigSetKeyValue(*pTypeValue, *iter);
			int nRet = kxe_config_read_config(
				key,
				NULL,
				pTypeValue
				);
			if (nRet != 0)
			{
				break;
			}

			std::wistringstream is(*iter);
			Key key;
			is >> key;
			mapKeyToType[key] = pTypeValue;
		}
	}

	if (newKey)
	{
		kxe_config_close_key(newKey);
	}

	return nRet;
}

template
<
	typename Key,
	typename T
>
inline int kxe_config_write_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const std::map<Key, T*>& mapKeyToValue
	)
{
	kxe_config_key_t newKey = NULL;

	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}


	std::map<Key, T*>::const_iterator iter = mapKeyToValue.begin();
	for (; iter != mapKeyToValue.end(); ++iter)
	{
		nRet = kxe_config_write_config(
			key,
			NULL,
			(*iter->second)
			);
		if (nRet != 0)
		{
			break;
		}
	}

	if (newKey)
	{
		kxe_config_close_key(newKey);
	}

	return nRet;
}

#endif

template
<
	typename T
>
inline int kxe_config_read_value(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const wchar_t* pwszValueName,
	T& value
	)
{
	int nRetBuffer = 0;
	int nRet = kxe_base_get_config(
		key,
		pwszPath,
		pwszValueName,
		NULL,
		0,
		&nRetBuffer
		);
	if (nRet != 0 && nRet != ERROR_MORE_DATA)
	{
		return nRet;
	}

	std::vector<wchar_t> vec(nRetBuffer);
	nRet = kxe_base_get_config(
		key,
		pwszPath,
		pwszValueName,
		&vec[0],
		(int)vec.size(),
		&nRetBuffer
		);
	if (nRet == 0)
	{
		KxEStringToType(&vec[0], value);
	}
	return nRet;

}

template
<
	typename T
>
inline int kxe_config_write_value(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const wchar_t* pwszValueName,
	const T& value
	)
{
	std::wostringstream os;
	os << value;
	return kxe_base_set_config(
		key,
		pwszPath,
		pwszValueName,
		os.str().c_str(),
		(int)os.str().size() + 1
		);
}

template
<
	typename T
>
inline int kxe_config_read_value(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const wchar_t* pwszValueName,
	std::list<T>& listValue
	)
{
	if (pwszValueName == NULL)
	{
		return E_KXEBASE_INVALID_PARAMETER;
	}

	std::wstring strPath;
	if (pwszPath)
	{
		strPath = pwszPath;
		strPath += L"\\";
	}

	strPath += pwszValueName;

	return kxe_config_read_config(
		key,
		strPath.c_str(),
		listValue
		);
}

template
<
	typename T
>
inline int kxe_config_write_value(kxe_config_key_t key,
								  const wchar_t* pwszPath,
								  const wchar_t* pwszValueName,
								  const std::list<T>& listValue)
{
	if (pwszValueName == NULL)
	{
		return E_KXEBASE_INVALID_PARAMETER;
	}
	else if (listValue.empty())
	{
		return 0;
	}

	std::wstring strPath;
	if (pwszPath)
	{
		strPath = pwszPath;
		strPath += L"\\";
	}

	strPath += pwszValueName;

	return kxe_config_write_config(
		key,
		strPath.c_str(),
		listValue
		);
}

template
<
	typename Key,
	typename T
>
inline int kxe_config_read_value(kxe_config_key_t key,
								 const wchar_t* pwszPath,
								 const wchar_t* pwszValueName,
								 std::map<Key, T>& keyToValue)
{
	if (pwszValueName == NULL)
	{
		return E_KXEBASE_INVALID_PARAMETER;
	}

	std::wstring strPath;
	if (pwszPath)
	{
		strPath = pwszPath;
		strPath += L"\\";
	}

	strPath += pwszValueName;

	return kxe_config_read_config(
		key,
		strPath.c_str(),
		keyToValue
		);
}

template
<
	typename Key,
	typename T
>
inline int kxe_config_write_value(
	kxe_config_key_t key,
	const wchar_t* pwszPath,
	const wchar_t* pwszValueName,
	const std::map<Key, T>& keyToValue
	)
{
	if (pwszValueName == NULL)
	{
		return E_KXEBASE_INVALID_PARAMETER;
	}
	else if (keyToValue.empty())
	{
		return 0;
	}

	std::wstring strPath;
	if (pwszPath)
	{
		strPath = pwszPath;
		strPath += L"\\";
	}

	strPath += pwszValueName;

	return kxe_config_write_config(
		key,
		strPath.c_str(),
		keyToValue
		);
}

#if _MSC_VER == 1200

KXE_CFG_DDX_DEFINE(KSCLSID);

inline int kxe_config_write_config(kxe_config_key_t key,
								   const wchar_t* pwszPath,
								   const std::list<KSCLSID>& listValue)
{
	kxe_config_key_t newKey = NULL;
	
	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}
	
	
	std::list<KSCLSID>::const_iterator iter = listValue.begin();
	for (; iter != listValue.end(); ++iter)
	{
		nRet = kxe_config_write_config(
			key,
			NULL,
			*iter
			);
		if (nRet != 0)
		{
			break;
		}
	}
	
	if (newKey)
	{
		kxe_config_close_key(newKey);
	}
	
	return nRet;
}

inline int kxe_config_read_config(kxe_config_key_t key,
								  const wchar_t* pwszPath, 
								  std::list<KSCLSID>& listValue)
{
	kxe_config_key_t newKey = NULL;
	
	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}
	
	
	std::list<std::wstring> listKey;
	nRet = kxe_config_read_key(key, NULL, listKey);
	if (nRet == 0)
	{
		std::list<std::wstring>::iterator iter = listKey.begin();
		
		KSCLSID value = {0};
		for (; iter != listKey.end(); ++iter)
		{
			KxEConfigSetKeyValue(value, *iter);
			int nRet = kxe_config_read_config(
				key,
				NULL,
				value
				);
			if (nRet != 0)
			{
				break;
			}
			listValue.push_back(value);
		}
	}
	
	if (newKey)
	{
		kxe_config_close_key(newKey);
	}
	
	return nRet;
}

#endif