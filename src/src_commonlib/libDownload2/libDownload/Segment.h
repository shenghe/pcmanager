#pragma once
#include <vector>
#include "Defines.h"

class CHttpAsyncIn;
struct Segment
{
	Segment() : index(0),startposition(0), currentposition(0), endposition(0), instream(NULL)
	{}
	Segment(int _index, int64 startpos, int64 endpos)
		: index(_index), startposition(startpos), currentposition(startpos), endposition(endpos), instream(NULL)
	{}
	int index;
	int64 startposition, endposition, currentposition;
	CHttpAsyncIn *instream;
	
	BOOL isDone() const
	{
		return endposition>0 && currentposition>=endposition;
	}
	
	int64 Remain() const
	{
		return endposition - currentposition;
	}
};
typedef Segment* SegmentPtr;
typedef std::vector<Segment*> SegmentPtrs;

class CSegmentInfoFile
{
protected:
	// *.sif
	struct TSegmentInfoFileHeader
	{
		CHAR magic[8];
		int ver;
		uint32 dwsize;
		int64 filesize;
		int64 filesize_downloaded;
		SYSTEMTIME filetime;
		int32 segnum;
	};
#define SEGMENT_INFO_FILE_MAGIC "kdown$"
public:
	CSegmentInfoFile(LPCTSTR szDownloadedFilename);
	BOOL Load( int64 &iContentLength, int64 &iDownloaded, SegmentPtrs &parts );
	BOOL Save( int64 iContentLength, int64 iDownloaded, const SegmentPtrs &parts );
	void Unlink();

protected:
	CString m_strFilePath;
};

