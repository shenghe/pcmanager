
#pragma once

#include "listtypes.h"

class CListEditEx : public CWindowImpl< CListEditEx, CEdit >
{
public:
	CListEditEx()
	{
		m_nItem = NULL_ITEM;
		m_nSubItem = NULL_SUBITEM;
		m_nFlags = ITEM_FLAGS_NONE;
		m_nExitChar = 0;
	}
	
	~CListEditEx()
	{
	}

protected:
	int m_nItem;
	int m_nSubItem;
	UINT m_nFlags;
	TCHAR m_nExitChar;
	CFont m_fntEditFont;
	
public:
	BOOL Create( HWND hWndParent, int nItem, int nSubItem, CRect& rcRect, UINT nFlags, LPCTSTR lpszItemText )
	{
		m_nItem = nItem;
		m_nSubItem = nSubItem;
		m_nFlags = nFlags;
		m_nExitChar = 0;
		
		// destroy old edit control...
		if ( IsWindow() )
			DestroyWindow();
		
		DWORD dwStyle = WS_CHILD | ES_AUTOHSCROLL;
		
		// right-justify numbers
		if ( nFlags & ( ITEM_FLAGS_EDIT_NUMBER | ITEM_FLAGS_EDIT_FLOAT ) )
			dwStyle |= ES_RIGHT;
		
		if ( nFlags & ITEM_FLAGS_EDIT_UPPER )
			dwStyle |= ES_UPPERCASE;
		
		// create edit control
		if ( CWindowImpl< CListEditEx, CEdit >::Create( hWndParent, CRect( rcRect.left + 2, rcRect.top + 3, rcRect.right - 3, rcRect.bottom - 2 ), NULL, dwStyle ) == NULL )
			return FALSE;
		
		// get system message font
		CLogFont logFont;
		logFont.SetMessageBoxFont();
		if ( !m_fntEditFont.IsNull() )
			m_fntEditFont.DeleteObject();
		if ( m_fntEditFont.CreateFontIndirect( &logFont ) == NULL )
			return FALSE;

		SetFont( m_fntEditFont );
		SetMargins( ITEM_EDIT_MARGIN, ITEM_EDIT_MARGIN );		
		SetWindowText( lpszItemText );
		
		// show edit control
		ShowWindow( SW_SHOW );
		
		SetSelAll();
		SetFocus();
		
		return TRUE;
	}
	
	BOOL IsValid( TCHAR nChar )
	{
		// validate number and float input
		if ( !( m_nFlags & ( ITEM_FLAGS_EDIT_NUMBER | ITEM_FLAGS_EDIT_FLOAT ) ) || nChar == VK_BACK )
			return TRUE;
		
		CString strValue;
		int nValueLength = GetWindowTextLength() + 1;
		GetWindowText( strValue.GetBuffer( nValueLength ), nValueLength );
		strValue.ReleaseBuffer();
		
		// get selected positions
		int nStartChar;
		int nEndChar;
		GetSel( nStartChar, nEndChar );
		
		// are we changing the sign?
		if ( ( m_nFlags & ITEM_FLAGS_EDIT_NEGATIVE ) && nChar == _T( '-' ) )
		{
			BOOL bNegative = FALSE;
			if ( m_nFlags & ITEM_FLAGS_EDIT_FLOAT )
			{
				double dblValue = _tstof( strValue );
				bNegative = ( dblValue < 0 );
				strValue.Format( _T( "%lf" ), -dblValue );
			}
			else
			{
				long lValue = _ttol( strValue );
				bNegative = ( lValue < 0 );
				strValue.Format( _T( "%ld" ), -lValue );
			}
			
			SetWindowText( strValue );
			
			// restore select position
			SetSel( bNegative ? nStartChar - 1 : nStartChar + 1, bNegative ? nEndChar - 1 : nEndChar + 1 );
			return FALSE;
		}
		
		// construct new value string using entered character
		CString strNewValue = strValue.Left( nStartChar ) + nChar + strValue.Right( strValue.GetLength() - nEndChar );
		
		int nGreaterThan = 0;
		int nLessThan = 0;
		int nEquals = 0;
		int nDecimalPoint = 0;
		
		int nNegativeIndex = -1;
		int nGreaterIndex = -1;
		int nLessIndex = -1;
		int nEqualIndex = -1;
		int nDecimalIndex = -1;
		int nDigitIndex = -1;
		
		for ( int nCharIndex = 0; nCharIndex < strNewValue.GetLength(); nCharIndex++ )
		{
			TCHAR nCharValue = strNewValue[ nCharIndex ];
			switch ( nCharValue )
			{
				case _T( '-' ):	nNegativeIndex = nCharIndex;
								break;
				case _T( '>' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_OPERATOR ) )
									return FALSE;
								nGreaterIndex = nCharIndex;
								nGreaterThan++;
								break;
				case _T( '<' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_OPERATOR ) )
									return FALSE;
								nLessIndex = nCharIndex;
								nLessThan++;
								break;
				case _T( '=' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_OPERATOR ) )
									return FALSE;
								nEqualIndex = nCharIndex;
								nEquals++;
								break;
				case _T( '.' ):	if ( !( m_nFlags & ITEM_FLAGS_EDIT_FLOAT ) )
									return FALSE;
								nDecimalIndex = nCharIndex;
								nDecimalPoint++;
								break;
				default:		if ( !_istdigit( nCharValue ) )
									return FALSE;
								if ( nDigitIndex < 0 )
									nDigitIndex = nCharIndex;
								break;
			}

			// invalid if text contains more than one '>', '<', '=' or '.'
			if ( nGreaterThan > 1 || nLessThan > 1 || nEquals > 1 || nDecimalPoint > 1 )
				return FALSE;
		}

		// invalid if text contains '=>' or '=<'
		if ( nGreaterIndex != -1 && nEqualIndex != -1 && nGreaterIndex > nEqualIndex )
			return FALSE;
		if ( nLessIndex != -1 && nEqualIndex != -1 && nLessIndex > nEqualIndex )
			return FALSE;

		// invalid if digits exist before operator
		if ( nDigitIndex != -1 && nGreaterIndex != -1 && nGreaterIndex > nDigitIndex )
			return FALSE;
		if ( nDigitIndex != -1 && nLessIndex != -1 && nLessIndex > nDigitIndex )
			return FALSE;
		if ( nDigitIndex != -1 && nEqualIndex != -1 && nEqualIndex > nDigitIndex )
			return FALSE;
		if ( nDigitIndex != -1 && nNegativeIndex != -1 && nNegativeIndex > nDigitIndex )
			return FALSE;
		
		return TRUE;
	}
	
	BEGIN_MSG_MAP(CListEditEx)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_GETDLGCODE(OnGetDlgCode)
		MSG_WM_CHAR(OnChar)
	END_MSG_MAP()
	
	void OnKillFocus( HWND hNewWnd )
	{
		CWindow wndParent( GetParent() );
		if ( wndParent.IsWindow() )
		{
			CString strValue;
			int nValueLength = GetWindowTextLength() + 1;
			GetWindowText( strValue.GetBuffer( nValueLength ), nValueLength );
			strValue.ReleaseBuffer();
			
			CListNotify listNotify;
			listNotify.m_hdrNotify.hwndFrom = m_hWnd;
			listNotify.m_hdrNotify.idFrom = GetDlgCtrlID();
			listNotify.m_hdrNotify.code = LCN_ENDEDIT;
			listNotify.m_nItem = m_nItem;
			listNotify.m_nSubItem = m_nSubItem;
			listNotify.m_nExitChar = m_nExitChar;
			listNotify.m_lpszItemText = strValue;
			listNotify.m_lpItemDate = NULL;

			// forward notification to parent
			FORWARD_WM_NOTIFY( wndParent, listNotify.m_hdrNotify.idFrom, &listNotify.m_hdrNotify, ::SendMessage );
		}
		
		ShowWindow( SW_HIDE );
	}
	
	UINT OnGetDlgCode( LPMSG lpMessage )
	{
		return DLGC_WANTALLKEYS;
	}
	
	void OnChar( TCHAR nChar, UINT nRepCnt, UINT nFlags )
	{
		switch ( nChar )
		{
			case VK_TAB:
			case VK_RETURN:
			case VK_ESCAPE:	{
								m_nExitChar = nChar;
								CWindow wndParent( GetParent() );
								if ( wndParent.IsWindow() )
									wndParent.SetFocus();
							}
							break;
			default:		SetMsgHandled( !IsValid( nChar ) );
							break;
		}
	}
};
