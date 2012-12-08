#pragma once
//

static TCHAR g_tszExDoc[23][5] = {	L".wps", L".wpt", L".doc", L".dot", L".rtf", L".txt", L".uof", \
									L".et",  L".ett", L".xls", L".xlt", L".dbf", L".csv", L".prn",\
									L".dps", L".dpt", L".ppt", L".pot", L".pps", L".wmf", L".emf", \
									L".pdf", L".pdp"};

static TCHAR g_tszEXPic[48][6] = {	L".jpg", L".jpeg", L".jjpe", L".bmp",  L".rle", L".dib", L".gif", \
									L".png", L".dcm",  L".dc3",  L".dic",  L".img", L".ei",  L".eizz", \
									L".eps", L".iff",  L".tdi",  L".jpf",  L".jpx", L".j2c", L".j2k", \
									L".jpc", L".pcx",  L".raw",  L".pict", L".pct", L".pxr", L".pxi", \
									L".sct", L".sgi",  L".rgb",  L".rgba", L".bw",  L".pic", L".tga", \
									L".vda", L".icb",  L".vst",  L".tiff", L".tif", L".rla", L".psb", \
									L".pbm", L".pgm",  L".ppm",  L".pnm",  L".pfm", L".pam" };

static TCHAR g_tszExMv[91][7] = {   L".asf",  L".avi",  L".vm",   L".wmp",  L".wmv",  L".ram",  L".rm", \
									L".rmvb", L".rp",   L".rpm",  L".rt",   L".smi",  L".smil", L".dvd",\
									L".m1v",  L".m2v",  L".m2p",  L".m2t",  L".m2ts", L".mp2v", L".mpe", \
									L".mpeg", L".mpv2", L".pss",  L".pva",  L".tp",   L".tpr",  L".ts", \
									L".m4b",  L".m4p",  L".m4v",  L".mp4",  L".mpeg4", L".3g2", L".3gp", \
									L".3gp2", L".3gpp", L".mov",  L".qt",   L".flv",  L".hlv",  L".swf", \
									L".f4v",  L".ifo",  L".vob",  L".amv",  L".bik",  L".csf",  L".divx", \
									L".evo",  L".ivm",  L".mkv",  L".mod",  L".mts",  L".ogm",  L".pmp", \
									L".scm",  L".tod",  L".vp6",  L".aac",  L".ac3",  L".amr",  L".ape", \
									L".cda",  L".dts",  L".flac", L".mla",  L".m2a",  L".m4a",  L".midi", \
									L".mid",  L".mka",  L".mp2",  L".mp3",  L".mpa",  L".ogg",  L".ra", \
									L".rmi",  L".tta",  L".wav",  L".wma",  L".wv",   L".asx",  L".cue", \
									L".kpl",  L".m3u",  L".qpl",  L".pls",  L".smpl", L".ass",  L".srt"};

static TCHAR g_tszExZip[32][8] = {  L".zip", L".rar", L".7z",	 L".tzr",   L".iso",  L".cab",	L".dmg", \
									L".hfs", L".arj", L".gz",	 L".gzip",  L".tgz",  L".tpz",	L".xar", \
									L".rpm", L".bz2", L".bzip2", L".tbz2",  L".tbz",  L".deb",	L".cpio", \
									L".z",	 L".taz", L".jar",	 L".xpi",   L".lzma", L".lzma86", L".lzh", \
									L".lha", L".001", L".wim",   L".swm" };
class DeskTypeCnt
{
public:
	int nDoc;
	int nLnk;
	int nPic;
	int nMv;
	int nZip;
	int nFld;
	int nOt;

	DeskTypeCnt()
	{
		nDoc = 0;
		nLnk = 0;
		nPic = 0;
		nMv  = 0;
		nZip = 0;
		nFld = 0;
		nOt  = 0;
	}
};


class DeskNoUseCnt
{
public:
	int nW;
	int nTw;
	int nM;
	int nOm;

	DeskNoUseCnt()
	{
		nW	= 0;
		nTw = 0;
		nM	= 0;
		nOm = 0;
	}

};


#include <ShlObj.h>

class CDesktopInfo
{
public:
	CDesktopInfo();
	~CDesktopInfo();

	HRESULT CollectTypeInfo( DeskTypeCnt& info );
	HRESULT CollectRecentInfo( DeskTypeCnt& info );
	HRESULT CollectNoUseInfo( DeskNoUseCnt& info );

protected:

	HRESULT _CollectTypeInfo( DeskTypeCnt& info, int nCsidl, BOOL bChange = FALSE );
	void	CalTypeCnt( DeskTypeCnt& info, CString& strFile, BOOL bChange );
	HRESULT LnkToRealPath( LPCTSTR pszLnk, LPTSTR tszFilePath );

	HRESULT _CollectTimeInfo( DeskNoUseCnt& info, int nCsidl);
	void	CalTimeCnt( DeskNoUseCnt& info, CString& strFile );
	BOOL	GetFileLastVisit(LPCTSTR lpszFileName, __time64_t& tVisit );

	BOOL IsDoc( CString& strExten );
	BOOL IsPic( CString& strExten );
	BOOL IsMv( CString& strExten );
	BOOL IsZip( CString& strExten );

};