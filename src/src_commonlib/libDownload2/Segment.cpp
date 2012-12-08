#include "StdAfx.h"
#include "Segment.h"
#include "FileStream.h"

CSegmentInfoFile::CSegmentInfoFile( LPCTSTR szDownloadedFilename ) : m_strFilePath(szDownloadedFilename)
{
}

BOOL CSegmentInfoFile::Load( int64 &iContentLength, int64 &iDownloaded, SegmentPtrs &parts )
{
	CString strSegFile;
	strSegFile = m_strFilePath + DOWNLOAD_TEMP_FILE_INFO_SUFFIX;

	CFileInStream fs(strSegFile);
	if(fs.Create())
	{
		TSegmentInfoFileHeader hdr = {0};
		DWORD dwReaded = 0;
		if(fs.Read(&hdr, sizeof(hdr), &dwReaded) && dwReaded==sizeof(hdr))
		{
			if(strcmp(hdr.magic, SEGMENT_INFO_FILE_MAGIC)==0)
			{
				iContentLength = hdr.filesize;
				iDownloaded = hdr.filesize_downloaded;
				// file is valid 
				DWORD buffer_size = hdr.dwsize-sizeof(hdr);
				ATLASSERT(buffer_size>0);
				ATLASSERT(buffer_size==(sizeof(Segment)*hdr.segnum));
				ATLASSERT(parts.empty());

				for(int i=0; i<hdr.segnum; ++i)
				{
					Segment *p = new Segment;
					ATLVERIFY( fs.Read(p, sizeof(Segment), &dwReaded) && dwReaded==sizeof(Segment) );
					p->instream = NULL;
					parts.push_back( p );					
					MYTRACE(_T("LOAD SEGMENT:%d - %I64d-%I64d-%I64d \r\n"), p->index, p->startposition, p->currentposition, p->endposition);
				}
				ATLASSERT(parts.size()==hdr.segnum);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CSegmentInfoFile::Save( int64 iContentLength, int64 iDownloaded, const SegmentPtrs &parts )
{
	TSegmentInfoFileHeader hdr = {0};
	strcpy(hdr.magic, SEGMENT_INFO_FILE_MAGIC);
	hdr.dwsize = sizeof(hdr) + sizeof(Segment) * parts.size();
	hdr.filesize = iContentLength;
	hdr.filesize_downloaded = iDownloaded;
	hdr.segnum = parts.size();

	char *pbuffer = new char[hdr.dwsize];
	memset(pbuffer, 0, hdr.dwsize);
	memcpy(pbuffer, &hdr, sizeof(hdr));
	char *pb = pbuffer + sizeof(hdr);
	for(size_t i=0; i<parts.size(); ++i)
	{
		Segment *p = parts[i];
		memcpy(pb, p, sizeof(Segment));
		pb += sizeof(Segment);
		
		MYTRACE(_T("SAVE SEGMENT:%d - %I64d-%I64d-%I64d \r\n"), p->index, p->startposition, p->currentposition, p->endposition);
	}
	CString strSegFile;
	strSegFile = m_strFilePath + DOWNLOAD_TEMP_FILE_INFO_SUFFIX;
	
	CFileStream fs(strSegFile);
	DWORD dwWrited = 0;
	BOOL bRet = fs.Create() && fs.Write(pbuffer, hdr.dwsize, &dwWrited) && dwWrited==hdr.dwsize;
	SAFE_DELETE_ARRAY(pbuffer);
	return bRet;
}

void CSegmentInfoFile::Unlink()
{
	CString strSegFile;
	strSegFile = m_strFilePath + DOWNLOAD_TEMP_FILE_INFO_SUFFIX;
	DeleteFile(strSegFile);
}