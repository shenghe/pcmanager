#pragma once

struct TVectorFileHeader
{
	CHAR		szmagic[16];
	INT			dwItemNum;
	INT			dwItemSize;
	INT64		dwFileSize;
	SYSTEMTIME	fileTime;
	DWORD		dwFlags;
};

template<typename T>
class CVectorPersist
{
	typedef std::vector<T> TArray;
public:
	CVectorPersist(LPCTSTR szDownloadedFilename):m_strFilePath(szDownloadedFilename){}
	~CVectorPersist(void){}
	
	HRESULT Load( LPCSTR szMagic, TArray &items, SYSTEMTIME &fileTime )
	{
		HRESULT hr = S_OK;
		CAtlFile file;
		hr = file.Create(m_strFilePath, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
		if(FAILED(hr))	return hr;

		TVectorFileHeader hdr = {0};
		if(SUCCEEDED(hr=file.Read(&hdr, sizeof(hdr))))
		{
			if(strcmp(szMagic, hdr.szmagic)!=0)
				return E_FAIL;
			ATLASSERT(sizeof(T)==hdr.dwItemSize);
			if(sizeof(T)!=hdr.dwItemSize)
				return E_FAIL;
			
			items.clear();
			for(int i=0; i<hdr.dwItemNum; ++i)
			{
				T t;
				if(FAILED(file.Read(&t, sizeof(T))))
				{
					return E_FAIL;
				}
				items.push_back(t);
			}
			ATLASSERT(items.size()==hdr.dwItemNum);
			fileTime = hdr.fileTime;
			hr = S_OK;
		}
		return hr;
	}
	
	HRESULT Save( LPCSTR szMagic, const TArray &items )
	{
		HRESULT hr = S_OK;
		CAtlFile file;
		hr = file.Create(m_strFilePath, GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS);
		if(FAILED(hr))	return hr;
	
		TVectorFileHeader hdr = {0};
		strcpy(hdr.szmagic, szMagic);
		hdr.dwItemSize = sizeof(T);
		hdr.dwItemNum = items.size();
		hdr.dwFileSize = hdr.dwItemSize*hdr.dwItemNum + sizeof(TVectorFileHeader);

		hr = file.Write(&hdr, sizeof(hdr));
		if(FAILED(hr))	return hr;
		
		for(TArray::const_iterator it=items.begin(); it!=items.end(); ++it)
		{
			hr = file.Write(&(*it), sizeof(T));
			if(FAILED(hr))	return hr;
		}
		file.Close();
		return S_OK;
	}
	
protected:
	CString m_strFilePath;
};
