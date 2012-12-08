/********************************************************************
* CreatedOn: 2006-12-11   20:29
* FileName:  KJSONDoc.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KJSONDoc_H_
#define __KJSONDoc_H_

#include "KJSONBasic.h"
#include "KJSONHandle.h"

_KAN_DOM_BEGIN

//////////////////////////////////////////////////////////////////////////
// 除了文件操作LoadFile和SaveFile外
// 输入输出的字符串都应该是UTF8编码,少数包含宽字符的接口会被转化为UTF8
class KJSONDoc
{
public:
    KJSONDoc();

    ~KJSONDoc();

    //////////////////////////////////////////////////////////////////////////
    // i/o operations
    long LoadFile(const TKChar* pcszFileName);

    long LoadJSON(const char* pcszJSON);

    long LoadJSONRootName(const char* pcszJSONRootName);

    long SaveFile(const TKChar* pcszFileName);

    long ToJSONString(std::string* pstrRet);

    long ToCompactJSONString(std::string* pstrRet);

    void ClearDoc();


    //////////////////////////////////////////////////////////////////////////
    // structure operations
    long PhrasePath(const char* pcszPath, std::vector<std::string>& pathItemList);

    KJSONHandle OpenItemPath(const char* pcszPath, bool bCreateIfNotExist);

    KJSONHandle OpenChild(
        KJSONHandle hParent,
        const char* pcszName,
        bool        bCreateIfNotExist
    );

    //KJSONHandle OpenArray(KJSONHandle hParent, bool bCreateIfNotExist);

    KJSONHandle OpenArrayElement(KJSONHandle hParent, size_t nIndex, bool bCreateIfNotExist);

    //KJSONHandle AppendArrayElement(KJSONHandle hParent);

    //size_t GetArraySize(KJSONHandle hParent);

    long EraseAllChildren(KJSONHandle hParent);

    long RemoveChildren(KJSONHandle hParent, const char* pcszName);

    long GetMemberNameList(std::vector<std::string> memberNameList);


    long GetRootName(std::string& strRootName);

    KJSONHandle RootHandle();


    //////////////////////////////////////////////////////////////////////////
    // data operations
    template<class T>
    long SetValue(KJSONHandle hJSONCursor, const T& value)
	{
		assert(m_bInitFlag);
		
		return _SetJSONItem(hJSONCursor, value);
	}

    template<class T>
    long GetValue(KJSONHandle hJSONCursor, T& value)
	{
		assert(m_bInitFlag);
		
		if (!hJSONCursor.ValueRef().isString())
		{
			return E_KJSON_EXPECT_STRING_TYPE;
		}
		
		
		return _GetJSONItem(hJSONCursor, value);
	}


private:
    KJSONDoc(const KJSONDoc&);
    KJSONDoc& operator=(const KJSONDoc&);


    template<class T>
    long _SetJSONItem(KJSONHandle hJSONCursor, const T& value)
	{
		assert(m_bInitFlag);
		
		std::ostringstream os;
		os << value;
		if (os.fail())
		{
			return E_KJSON_VALUE_STREAM_ERROR;
		}
		
		hJSONCursor.ValueRef() = os.str();
		
		return S_OK;
	}

    template<class T>
    long _GetJSONItem(KJSONHandle hJSONCursor, T& value)
	{
		assert(m_bInitFlag);
		
		// 在KJSONDoc::GetValue处理不是string的情形
		assert(hJSONCursor.ValueRef().isString());
		
		std::istringstream is;
		is.str(hJSONCursor.ValueRef().asString());
		is >> value;
		if (is.fail())
		{
			return E_KJSON_VALUE_STREAM_ERROR;
		}
		
		
		return S_OK;
	}




#if _MSC_VER > 1200
	template <class _Traits, class _Alloc>
    long _SetJSONItem(
        KJSONHandle hJSONCursor,
        const std::basic_string<char, _Traits, _Alloc>& strValue
    )
	{
		assert(m_bInitFlag);
		
		hJSONCursor.ValueRef() = strValue.c_str();
		
		return S_OK;
	}

    template <class _Traits, class _Alloc>
    long _GetJSONItem(
        KJSONHandle hJSONCursor,
        std::basic_string<char, _Traits, _Alloc>& strValue
    )
	{
		assert(m_bInitFlag);
		
		// 在KJSONDoc::GetValue处理不是string的情形
		assert(hJSONCursor.ValueRef().isString());
		
		strValue = hJSONCursor.ValueRef().asCString();
		
		return S_OK;
	}




    template <class _Traits, class _Alloc>
    long _SetJSONItem(
        KJSONHandle hJSONCursor,
        const std::basic_string<wchar_t, _Traits, _Alloc>& wstrValue
    )
	{
		assert(m_bInitFlag);
		
		hJSONCursor.ValueRef() = (const char*)KSCE::KU16ToU8(wstrValue.c_str());
		
		return S_OK;
	}

    template <class _Traits, class _Alloc>
    long _GetJSONItem(
        KJSONHandle hJSONCursor,
        std::basic_string<wchar_t, _Traits, _Alloc>& wstrValue
    )
	{
		assert(m_bInitFlag);
		
		// 在KJSONDoc::GetValue处理不是string的情形
		assert(hJSONCursor.ValueRef().isString());
		
		wstrValue = (const wchar_t*)KSCE::KU8ToU16(hJSONCursor.ValueRef().asString().c_str());
		
		return S_OK;
	}
#else
	long _SetJSONItem(
        KJSONHandle hJSONCursor,
        const std::string& strValue
		)
	{
		assert(m_bInitFlag);
		
		hJSONCursor.ValueRef() = strValue.c_str();
		
		return S_OK;
	}
	
	long _GetJSONItem(
        KJSONHandle hJSONCursor,
        std::string& strValue
		)
	{
		assert(m_bInitFlag);
		
		// 在KJSONDoc::GetValue处理不是string的情形
		assert(hJSONCursor.ValueRef().isString());
		
		strValue = hJSONCursor.ValueRef().asCString();
		
		return S_OK;
	}
	
	long _SetJSONItem(
        KJSONHandle hJSONCursor,
        const std::wstring& wstrValue
		)
	{
		assert(m_bInitFlag);
		
		hJSONCursor.ValueRef() = (const char*)KSCE::KU16ToU8(wstrValue.c_str());
		
		return S_OK;
	}
	
	long _GetJSONItem(
        KJSONHandle hJSONCursor,
        std::wstring& wstrValue
		)
	{
		assert(m_bInitFlag);
		
		// 在KJSONDoc::GetValue处理不是string的情形
		assert(hJSONCursor.ValueRef().isString());
		
		wstrValue = (const wchar_t*)KSCE::KU8ToU16(hJSONCursor.ValueRef().asString().c_str());
		
		return S_OK;
	}
#endif


    long _SetJSONItem(KJSONHandle hJSONCursor, wchar_t wcValue);

    long _GetJSONItem(KJSONHandle hJSONCursor, wchar_t& wcValue);



    long _SetJSONItem(KJSONHandle hJSONCursor, const wchar_t* pwszValue);


    //////////////////////////////////////////////////////////////////////////
    // 危险的重载，应当禁止
    long _GetJSONItem(KJSONHandle hJSONCursor, void* value);

    long _GetJSONItem(KJSONHandle hJSONCursor, char* value);

    long _GetJSONItem(KJSONHandle hJSONCursor, unsigned char* value);

    long _GetJSONItem(KJSONHandle hJSONCursor, wchar_t* value);




    class AutoFile
    {
    public:
        AutoFile(FILE* pFile): m_pFile(pFile)
        {
        }

        ~AutoFile()
        {
            if (m_pFile)
            {
                fclose(m_pFile);
                m_pFile = NULL;
            }
        }

        operator FILE*()
        {
            return m_pFile;
        }

        bool operator !()
        {
            return !m_pFile;
        }

		FILE** operator &()
		{
			return &m_pFile;
		}
    private:
        AutoFile(const AutoFile&);
        AutoFile& operator=(const AutoFile&);

        FILE* m_pFile;
    };

    bool m_bInitFlag;

    Json::Value m_superRoot; // m_superRoot不是真正的root,真正的root从其子结点开始
};



_KAN_DOM_END


#include "KJSONDocImp.h"


#endif//__KJSONDoc_H_
