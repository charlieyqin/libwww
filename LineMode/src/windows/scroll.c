/* SCROLL.C - module to maintain a primative scroll window.
*/
#include <windows.h>
#include <string.h>
//#include "ctrl.h"
//#include "resource.h"
#include "lib.h"
#include "scroll.h"

// cursor states
#define CS_HIDE         0x00
#define CS_SHOW         0x01
// ascii definitions
#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_TAB       0x09
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D

//private functions
int NEAR scroll_MoveCursor(ScrollInfo_t* pScroll)
	{//	if (pXtra->app.fConnected && (pScroll->wCursorState & CS_SHOW))
	if (pScroll->wCursorState & CS_SHOW)
		SetCaretPos((pScroll->nColumn * pScroll->xChar) - pScroll->xOffset,
							  (pScroll->nRow * pScroll->yChar) - pScroll->yOffset);
	return ( TRUE ) ;
	}

//public functions
int NEAR Scroll_SetupInfo(ScrollInfo_t* pScroll, int maxRows, int maxCols)
	{
	pScroll->xSize			= 0;
	pScroll->ySize			= 0;
	pScroll->xScroll		 = 0;
	pScroll->yScroll		 = 0;
	pScroll->xOffset		 = 0;
	pScroll->yOffset		 = 0;
	pScroll->nColumn		  = 0;
	pScroll->nRow			  = 0;
	pScroll->maxRows = maxRows;
	pScroll->maxCols = maxCols;
	pScroll->wCursorState	= CS_HIDE;
	pScroll->maxTrackSize.x = (int)0x0;	//really big window
	pScroll->maxTrackSize.y = (int)0x0;
//	pScroll->maxTrackSize.y = (int)0x7FFFFFFF;
	// clear screen space
	// GPF here means no memory was allocated by caller
	_fmemset(pScroll->abScreen, ' ', pScroll->maxRows * pScroll->maxCols);
	return (0);
	}

void NEAR Scroll_DestroyInfo(ScrollInfo_t* pScroll)
	{
	return;
	}

int NEAR Scroll_WriteControl(ScrollInfo_t* pScroll, HWND hWnd, Scroll_control2_t control, int x, int y)
	{
	switch (control)
		{
		case Scroll_control_null:
			return (0);
		case Scroll_control_CR:
			pScroll->nColumn = 0;
			scroll_MoveCursor(pScroll);
			return (0);
		case Scroll_control_LF:
			if (pScroll->nRow++ == pScroll->maxRows - 1)
				{
				memmove(pScroll->abScreen, pScroll->abScreen + pScroll->maxCols, (pScroll->maxRows - 1) * pScroll->maxCols);
				memset(pScroll->abScreen + (pScroll->maxRows - 1) * pScroll->maxCols, ' ', pScroll->maxCols);
				InvalidateRect(hWnd, 0, FALSE);
				pScroll->nRow--;
				}
			scroll_MoveCursor(pScroll);
			return (0);
		case Scroll_control_BS:
			if (pScroll->nColumn > 0)
				pScroll->nColumn--;
			scroll_MoveCursor(pScroll);
			return (0);
		case Scroll_control_Bell:
			MessageBeep(0);
			return (0);
		case Scroll_control_Tab:
			{
			int curTab;
			curTab = pScroll->nColumn/8;
			curTab++;
			pScroll->nColumn = curTab * 8;
			if (pScroll->nColumn >= pScroll->maxCols)
				{
				Scroll_WriteControl(pScroll, hWnd, Scroll_control_CR, 0, 0);
				Scroll_WriteControl(pScroll, hWnd, Scroll_control_LF ,0 ,0);
				}
			else
				scroll_MoveCursor(pScroll);
			}
			return (0);
		case Scroll_control_clearEol:
			memset(pScroll->abScreen + (pScroll->nRow)*pScroll->maxCols + pScroll->nColumn, ' ', pScroll->maxCols - pScroll->nColumn);
			InvalidateRect(hWnd, 0, FALSE);
			return (0);
		case Scroll_control_goto:
			pScroll->nColumn = x;
			pScroll->nRow = y;
			scroll_MoveCursor(pScroll);
			return (0);
		case Scroll_control_normal:
			return (0);
		case Scroll_control_bold:
			return (0);
		case Scroll_control_inverse:
			return (0);
		default:	//undefined request
			return (1);
		}
	}

int NEAR Scroll_writeLiteral(ScrollInfo_t* pScroll, HWND hWnd, LPSTR lpBlock, int nLength)
	{
	int		  i ;
	RECT		 rect ;
int NEAR Scroll_writeInterpreted(ScrollInfo_t* pScroll, HWND hWnd, LPSTR lpBlock, int nLength);

	for (i = 0 ; i < nLength; i++)
		{
		*(pScroll->abScreen + pScroll->nRow * pScroll->maxCols + pScroll->nColumn) = lpBlock[i];
		rect.left = (pScroll->nColumn * pScroll->xChar) -	pScroll->xOffset;
		rect.right = rect.left + pScroll->xChar;
		rect.top = (pScroll->nRow * pScroll->yChar) -	pScroll->yOffset;
		rect.bottom = rect.top + pScroll->yChar;
		InvalidateRect(hWnd, &rect, FALSE);
		// Line wrap
		if (pScroll->nColumn < pScroll->maxCols - 1)
			pScroll->nColumn++;
		else if (pScroll->control & Scroll_control_autoWrap)
			{
			if (pScroll->control  & Scroll_control_crBegetsLf)
				Scroll_writeInterpreted(pScroll, hWnd, "\r", 1);
			else
				Scroll_writeInterpreted(pScroll, hWnd, "\r\n", 2);
			}
		}
	return (TRUE);
	}

int NEAR Scroll_writeInterpreted(ScrollInfo_t* pScroll, HWND hWnd, LPSTR lpBlock, int nLength)
	{
	int		  i ;

	for (i = 0 ; i < nLength; i++)
		{
		switch (lpBlock[i])
			{
			case ASCII_BEL:
				Scroll_WriteControl(pScroll, hWnd, Scroll_control_Bell, 0, 0);
				break ;

			case ASCII_BS:
				Scroll_WriteControl(pScroll, hWnd, Scroll_control_BS, 0, 0);
				break;

			case ASCII_TAB:
				Scroll_WriteControl(pScroll, hWnd, Scroll_control_Tab, 0, 0);
				break;

			case ASCII_CR:
				Scroll_WriteControl(pScroll, hWnd, Scroll_control_CR, 0, 0);
				if (pScroll->control  & Scroll_control_crBegetsLf)
					Scroll_WriteControl(pScroll, hWnd, Scroll_control_LF, 0, 0);
				break;

			case ASCII_LF:
				if (pScroll->control  & Scroll_control_lfBegetsCr)
					Scroll_WriteControl(pScroll, hWnd, Scroll_control_CR, 0, 0);
				Scroll_WriteControl(pScroll, hWnd, Scroll_control_LF, 0, 0);
				break ;

			default:
	 			Scroll_writeLiteral(pScroll, hWnd, lpBlock+i, 1);
				break;
			}
		}
	return (TRUE);
	} // end of Scroll_writeInterpreted()

int NEAR Scroll_WriteBlock(ScrollInfo_t* pScroll, HWND hWnd, LPSTR lpBlock, int nLength)
	{
	if (pScroll->control & Scroll_control_literal)
		return (Scroll_writeLiteral(pScroll, hWnd, lpBlock, nLength));
	else
		return (Scroll_writeInterpreted(pScroll, hWnd, lpBlock, nLength));
	}

int NEAR Scroll_SetSize(ScrollInfo_t* pScroll, HWND hWnd, WORD wVertSize, WORD wHorzSize)
	{
	int		  nScrollAmt ;

	pScroll->ySize = (int) wVertSize ;
	pScroll->yScroll = max(0, (pScroll->maxRows * pScroll->yChar) -	pScroll->ySize);
	nScrollAmt = min(pScroll->yScroll, pScroll->yOffset ) -	pScroll->yOffset;
	ScrollWindow(hWnd, 0, -nScrollAmt, 0, 0);

	pScroll->yOffset = pScroll->yOffset + nScrollAmt;
	SetScrollPos(hWnd, SB_VERT, pScroll->yOffset, FALSE);
	SetScrollRange(hWnd, SB_VERT, 0, pScroll->yScroll, TRUE);

	pScroll->xSize = (int) wHorzSize ;
	pScroll->xScroll = max(0, (pScroll->maxCols * pScroll->xChar) - pScroll->xSize);
	nScrollAmt = min(pScroll->xScroll, pScroll->xOffset) - pScroll->xOffset;
	ScrollWindow(hWnd, 0, -nScrollAmt, 0, 0);
	pScroll->xOffset = pScroll->xOffset + nScrollAmt;
	SetScrollPos(hWnd, SB_HORZ, pScroll->xOffset, FALSE);
	SetScrollRange(hWnd, SB_HORZ, 0, pScroll->xScroll, TRUE);

	InvalidateRect(hWnd, 0, TRUE);

	return (TRUE);

	} // end of Scroll_SetSize()

int NEAR Scroll_SetVert(ScrollInfo_t* pScroll, HWND hWnd, WORD wScrollCmd, WORD wScrollPos)
	{
	int		  nScrollAmt ;

	switch (wScrollCmd)
		{
		case SB_TOP:
			nScrollAmt = -pScroll->yOffset ;
			break ;

		case SB_BOTTOM:
			nScrollAmt = pScroll->yScroll - pScroll->yOffset ;
			break ;

		case SB_PAGEUP:
			nScrollAmt = -pScroll->ySize ;
			break ;

		case SB_PAGEDOWN:
			nScrollAmt = pScroll->ySize ;
			break ;

		case SB_LINEUP:
			nScrollAmt = -pScroll->yChar ;
			break ;

		case SB_LINEDOWN:
			nScrollAmt = pScroll->yChar ;
			break ;

		case SB_THUMBPOSITION:
			nScrollAmt = wScrollPos - pScroll->yOffset ;
			break ;

		default:
			return ( FALSE ) ;
		}
	if ((pScroll->yOffset + nScrollAmt) > pScroll->yScroll)
		nScrollAmt = pScroll->yScroll - pScroll->yOffset ;
	if ((pScroll->yOffset + nScrollAmt) < 0)
		nScrollAmt = -pScroll->yOffset ;
	ScrollWindow( hWnd, 0, -nScrollAmt, 0, 0 ) ;
	pScroll->yOffset = pScroll->yOffset + nScrollAmt ;
	SetScrollPos( hWnd, SB_VERT, pScroll->yOffset, TRUE ) ;

	return ( TRUE ) ;

	} // end of Scroll_SetVert()

int NEAR Scroll_SetHorz(ScrollInfo_t* pScroll, HWND hWnd, WORD wScrollCmd, WORD wScrollPos)
	{
	int		  nScrollAmt ;

	switch (wScrollCmd)
		{
		case SB_TOP:
			nScrollAmt = -pScroll->xOffset ;
			break ;

		case SB_BOTTOM:
			nScrollAmt = pScroll->xScroll - pScroll->xOffset ;
			break ;

		case SB_PAGEUP:
			nScrollAmt = -pScroll->xSize ;
			break ;

		case SB_PAGEDOWN:
			nScrollAmt = pScroll->xSize ;
			break ;

		case SB_LINEUP:
			nScrollAmt = -pScroll->xChar ;
			break ;

		case SB_LINEDOWN:
			nScrollAmt = pScroll->xChar ;
			break ;

		case SB_THUMBPOSITION:
			nScrollAmt = wScrollPos - pScroll->xOffset ;
			break ;

		default:
			return ( FALSE ) ;
		}
	if ((pScroll->xOffset + nScrollAmt) > pScroll->xScroll)
		nScrollAmt = pScroll->xScroll - pScroll->xOffset ;
	if ((pScroll->xOffset + nScrollAmt) < 0)
		nScrollAmt = -pScroll->xOffset ;
	ScrollWindow( hWnd, -nScrollAmt, 0, 0, 0 ) ;
	pScroll->xOffset = pScroll->xOffset + nScrollAmt ;
	SetScrollPos( hWnd, SB_HORZ, pScroll->xOffset, TRUE ) ;

	return ( TRUE ) ;
	} // end of Scroll_SetHorz()

int NEAR Scroll_SetFocus(HWND hWnd, ScrollInfo_t* pScroll)
	{
	if (pScroll->wCursorState != CS_SHOW)
		{
		CreateCaret(hWnd, 0, pScroll->xChar, pScroll->yChar);
		ShowCaret(hWnd);
		pScroll->wCursorState = CS_SHOW;
		}
	scroll_MoveCursor(pScroll);
	return (TRUE);
	} // end of Scroll_SetFocus()

int NEAR Scroll_KillFocus(HWND hWnd, ScrollInfo_t* pScroll)
	{
	if (pScroll->wCursorState != CS_HIDE)
		{
		HideCaret(hWnd);
		DestroyCaret();
		pScroll->wCursorState = CS_HIDE;
		}
	return (TRUE);
	} // end of Scroll_KillFocus()

int NEAR Scroll_Paint(ScrollInfo_t* pScroll, FontInfo_t* pFont, HWND hWnd, int showCursor)
	{
	int			 nRow, nCol, nEndRow, nEndCol, nCount, nHorzPos, nVertPos ;
	HDC			 hDC ;
	HFONT		  hOldFont ;
	PAINTSTRUCT  ps ;
	RECT			rect ;

	hDC = BeginPaint( hWnd, &ps ) ;
	hOldFont = SelectObject( hDC, pFont->hFont ) ;
	SetTextColor( hDC, pFont->rgbFGColor ) ;
	SetBkColor( hDC, GetSysColor( COLOR_WINDOW ) ) ;
	rect = ps.rcPaint ;
	nRow = min(pScroll->maxRows - 1, max(0, (rect.top + pScroll->yOffset) / pScroll->yChar));
	nEndRow = min(pScroll->maxRows - 1, ((rect.bottom + pScroll->yOffset - 1) / pScroll->yChar));
	nCol = min(pScroll->maxCols - 1, max(0, (rect.left + pScroll->xOffset) / pScroll->xChar));
	nEndCol = min(pScroll->maxCols - 1, ((rect.right + pScroll->xOffset - 1) / pScroll->xChar));
	nCount = nEndCol - nCol + 1 ;
	for (; nRow <= nEndRow; nRow++)
		{
		nVertPos = (nRow * pScroll->yChar) - pScroll->yOffset ;
		nHorzPos = (nCol * pScroll->xChar) - pScroll->xOffset ;
		rect.top = nVertPos ;
		rect.bottom = nVertPos + pScroll->yChar ;
		rect.left = nHorzPos ;
		rect.right = nHorzPos + pScroll->xChar * nCount ;
		SetBkMode( hDC, OPAQUE ) ;
		ExtTextOut( hDC, nHorzPos, nVertPos, ETO_OPAQUE | ETO_CLIPPED, &rect,
						(LPSTR)( pScroll->abScreen + nRow * pScroll->maxCols + nCol ),
						nCount, 0 ) ;
		}
	SelectObject( hDC, hOldFont ) ;
	EndPaint( hWnd, &ps ) ;
	if (showCursor)
		scroll_MoveCursor(pScroll);
	return ( TRUE ) ;
	} // end of Scroll_Paint()

int NEAR Scroll_ResetScreen(HWND hWnd, ScrollInfo_t* pScroll, FontInfo_t* pFont)
	{
	HDC			hDC ;
	TEXTMETRIC  tm ;
	RECT rect;

	if (0 != pFont->hFont)
		DeleteObject( pFont->hFont ) ;

	pFont->hFont = CreateFontIndirect( &pFont->logFont ) ;

	hDC = GetDC(hWnd);
	SelectObject(hDC, pFont->hFont);
	GetTextMetrics(hDC, &tm);
	ReleaseDC(hWnd, hDC);

	pScroll->xChar = tm.tmAveCharWidth;
	pScroll->yChar = tm.tmHeight + tm.tmExternalLeading;
	rect.left = 0;
	rect.top = 0;
	rect.right = pScroll->xChar * pScroll->maxCols;
	rect.bottom = pScroll->yChar * pScroll->maxRows;
	AdjustWindowRect(&rect, GetWindowLong(hWnd, GWL_STYLE), TRUE);
	pScroll->maxTrackSize.x = rect.right - rect.left;
	pScroll->maxTrackSize.y = rect.bottom - rect.top;
//	pScroll->maxTrackSize.x = pScroll->xChar * pScroll->maxCols + 2*GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXVSCROLL);
//	pScroll->maxTrackSize.y = pScroll->yChar * pScroll->maxRows + 2*GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYHSCROLL);

	// a slimy hack to force the scroll position, region to
	// be recalculated based on the new character sizes

//	SendMessage(hWnd, WM_SIZE, SIZENORMAL, (LPARAM)MAKELONG(rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top));
//	Scroll_SetSize(pScroll, hWnd, (WORD)(rcWindow.bottom - rcWindow.top), (WORD)(rcWindow.right - rcWindow.left));
	Scroll_SetSize(pScroll, hWnd, (WORD)pScroll->maxTrackSize.y, (WORD)pScroll->maxTrackSize.x);
	return (TRUE);
	} // end of Scroll_ResetScreen()

