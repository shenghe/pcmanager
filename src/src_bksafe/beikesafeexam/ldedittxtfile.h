#pragma once

class CLDEditTxtFile
{
public:
	CLDEditTxtFile(void);
	CLDEditTxtFile(CString strFilePath);
	~CLDEditTxtFile(void);
	int GetFileItemCount(){return m_arrFileItem.GetSize();}
	CString EnumItem(int nItem=0);
	int GetAllItems(CSimpleArray<CString>& arrItems);
	int WriteItemsToFile(CString strFile, CSimpleArray<CString> arrItems);
	BOOL* SetStopFlag(BOOL* bFlag);
	int LoadData();
private:
	int _LoadTxtFileWithItems(CSimpleArray<CString>& arrFileItems);
	int _fgets(CSimpleArray<CString>& arrFileItems);
	CString m_strTxtFile;
	CSimpleArray<CString> m_arrFileItem;
	BOOL* m_pStop;
};
