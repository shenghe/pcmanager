#ifndef KFONT_INC_
#define KFONT_INC_

//////////////////////////////////////////////////////////////////////////

#include "kfont.h"

//////////////////////////////////////////////////////////////////////////

#define KFONT_BOLD        0x0004
#define KFONT_UNDERLINE   0x0002
#define KFONT_ITALIC      0x0001

#define KFONT_MAKEKEY(bold, underline, italic, adding) \
	(unsigned short)((adding << 8) \
	| (bold ? KFONT_BOLD : 0) \
	| (underline ? KFONT_UNDERLINE : 0) \
	| (italic ? KFONT_ITALIC : 0))

#define KFONT_ISBOLD(key)         ((key & KFONT_BOLD) == KFONT_BOLD)
#define KFONT_ISUNDERLINE(key)    ((key & KFONT_UNDERLINE) == KFONT_UNDERLINE)
#define KFONT_ISITALIC(key)       ((key & KFONT_ITALIC) == KFONT_ITALIC)
#define KFONT_GETADDING(key)      (key >> 8)

#define KFONT_DEFAULTFONT         (KFONT_MAKEKEY(FALSE, FALSE, FALSE, 0))
#define KFONT_BOLDFONT            (KFONT_MAKEKEY(TRUE, FALSE, FALSE, 0))

//////////////////////////////////////////////////////////////////////////

typedef enum tagKFontFace {
	enumKFF_Tahoma = 0,
	enumKFF_Simsun = 1,
} KFontFace;

//////////////////////////////////////////////////////////////////////////

template<KFontFace t_nFontFace>
class KFontT
{
private:
	KFontT()
	{
	}

	~KFontT()
	{
		std::map<unsigned int, HFONT>::iterator i;

		for (i = m_mapFonts.begin(); i != m_mapFonts.end(); ++i)
		{
			if (i->second)
			{
				::DeleteObject(i->second);
			}
		}

		m_mapFonts.clear();
	}

	void GetFontFace(KFontFace nFontFace, std::wstring& strFontFace)
	{
		switch (nFontFace)
		{
		case enumKFF_Simsun:
			strFontFace = L"Simsun";
			break;

		case enumKFF_Tahoma:
			strFontFace = L"Tahoma";
			break;
		}
	}

	HFONT CreateFont(BOOL fBold, BOOL fUnderline, BOOL fItalic, char chAdding = 0)
	{
		HFONT retval = NULL;
		LOGFONTW logFont;
		std::wstring strFontFace;

		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &logFont);
		GetFontFace(t_nFontFace, strFontFace);
		if (strFontFace.length())
			wcscpy_s(logFont.lfFaceName, strFontFace.c_str());
		logFont.lfWeight = fBold ? FW_BOLD : FW_NORMAL;
		logFont.lfUnderline = fUnderline ? TRUE : FALSE;
		logFont.lfItalic = fItalic ? TRUE : FALSE;
		logFont.lfQuality = ANTIALIASED_QUALITY;
		logFont.lfHeight = 13;//logFont.lfHeight - chAdding;

		retval = ::CreateFontIndirect(&logFont);

		return retval;
	}

public:
	static KFontT& Instance()
	{
		static KFontT _singleton;
		return _singleton;
	}

	HFONT GetFont(BOOL fBold = FALSE, BOOL fUnderline = FALSE, BOOL fItalic = FALSE, char chAdding = 0)
	{
		HFONT retval = NULL;
		unsigned int uKey = KFONT_MAKEKEY(fBold, fUnderline, fItalic, 0);

		if (m_mapFonts.find(uKey) == m_mapFonts.end())
		{
			retval = CreateFont(fBold, fUnderline, fItalic, chAdding);
			m_mapFonts[uKey] = retval;
		}
		else
		{
			retval = m_mapFonts[uKey];
		}

		return retval;
	}

private:
	std::map<unsigned int, HFONT> m_mapFonts;
};

//////////////////////////////////////////////////////////////////////////

template<KFontFace t_nFontFace>
class KFontHelperT
{
public:
	KFontHelperT()
	{
	}

	~KFontHelperT()
	{
	}

	HFONT GetFont(BOOL fBold = FALSE, BOOL fUnderline = FALSE, BOOL fItalic = FALSE, char chAdding = 0)
	{
		KFontT<t_nFontFace>& m_fontPool = KFontT<t_nFontFace>::Instance();
		return m_fontPool.GetFont(fBold, fUnderline, fItalic, chAdding);
	}
};

//////////////////////////////////////////////////////////////////////////

typedef KFontHelperT<enumKFF_Simsun> KFontHelper;

//////////////////////////////////////////////////////////////////////////

#endif	// KFONT_INC_
