#pragma once
#define SAFE_CLOSE_HANDLE(h) if(h){CloseHandle(h);h=NULL;}
#define SAFE_CLOSE_INTERNETHANDLE(h) if(h){InternetCloseHandle(h);h=NULL;}
#define SIZE_OF_ARRAY(x) sizeof(x)/sizeof(x[0])
//#define SAFE_DELETE(x) if(x){delete x;x=NULL;}
#define SAFE_DELETE_ARRAY(x) if(x){delete []x;x=NULL;}
#define LOINT32(i)				((int)((__int64)(i) & 0xffffffff))
#define HIINT32(i)				((int)((__int64)(i) >> 32))

template<typename T> void release_array(std::vector<T*> &arr)
{
	typedef std::vector<T*> TPtrs;
	for(TPtrs::iterator it=arr.begin(); it!=arr.end(); ++it)
	{
		if(*it)
			delete *it;
		*it= NULL;
	}
	arr.clear();
}