#ifndef SOFTMGR_FILE_UTIL_H
#define SOFTMGR_FILE_UTIL_H
#include <io.h>

template<typename Func>
void ForeachFile(CString dir,Func& func)
{
	CString f=dir;
	if(dir.Find(_T("*"))<0)
		f=dir.TrimRight('\\')+_T("\\*");
	dir=f.Left(f.ReverseFind('\\'));
	struct _wfinddata_t findinfo={0};
	intptr_t h=_wfindfirst(f,&findinfo);
	if(h==-1)
		return;
	do 
	{
		if(func(dir,findinfo))
			break;
	} while (0==_wfindnext(h,&findinfo));
	_findclose(h);
}


#endif