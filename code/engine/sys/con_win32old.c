/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sys_local.h"
#include "windows.h"




#define COPY_ID			1
#define QUIT_ID			2
#define CLEAR_ID		3

#define ERRORBOX_ID		10
#define ERRORTEXT_ID	11

#define EDIT_ID			100
#define INPUT_ID		101

//#define INPUTBOX	1

typedef struct
{
	HWND            hWnd;
	HWND            hwndBuffer;

	HWND            hwndButtonClear;
	HWND            hwndButtonCopy;
	HWND            hwndButtonQuit;

	HWND            hwndErrorBox;
	HWND            hwndErrorText;

	HBITMAP         hbmLogo;
	HBITMAP         hbmClearBitmap;

	HBRUSH          hbrEditBackground;
	HBRUSH          hbrErrorBackground;

	HFONT           hfBufferFont;
	HFONT           hfButtonFont;

#if defined(INPUTBOX)
	HWND            hwndInputLine;
	char            consoleText[512], returnedText[512];
	WNDPROC         SysInputLineWndProc;
#endif

	char            errorString[80];

	
	int             visLevel;
	qboolean        quitOnClose;
	int             windowWidth, windowHeight;

} WinConData;

static WinConData	s_wcd;
static HINSTANCE	s_hInstance;


#define QCONSOLE_HISTORY 32

static WORD     qconsole_attrib;

// saved console status
static DWORD    qconsole_orig_mode;
static CONSOLE_CURSOR_INFO qconsole_orig_cursorinfo;

// cmd history
static char     qconsole_history[QCONSOLE_HISTORY][MAX_EDIT_LINE];
static int      qconsole_history_pos = -1;
static int      qconsole_history_oldest = 0;

// current edit buffer
static char     qconsole_line[MAX_EDIT_LINE];
static int      qconsole_linelen = 0;

static HANDLE   qconsole_hout;
static HANDLE   qconsole_hin;

/*
==================
CON_CtrlHandler

The Windows Console doesn't use signals for terminating the application
with Ctrl-C, logging off, window closing, etc.  Instead it uses a special
handler routine.  Fortunately, the values for Ctrl signals don't seem to
overlap with true signal codes that Windows provides, so calling
Sys_SigHandler() with those numbers should be safe for generating unique
shutdown messages.
==================
*/
static BOOL WINAPI CON_CtrlHandler(DWORD sig)
{
	Sys_SigHandler(sig);
	return TRUE;
}

/*
==================
CON_HistAdd
==================
*/
static void CON_HistAdd(void)
{
	Q_strncpyz(qconsole_history[qconsole_history_oldest], qconsole_line, sizeof(qconsole_history[qconsole_history_oldest]));

	if(qconsole_history_oldest >= QCONSOLE_HISTORY - 1)
		qconsole_history_oldest = 0;
	else
		qconsole_history_oldest++;

	qconsole_history_pos = qconsole_history_oldest;
}

/*
==================
CON_HistPrev
==================
*/
static void CON_HistPrev(void)
{
	int             pos;

	pos = (qconsole_history_pos < 1) ? (QCONSOLE_HISTORY - 1) : (qconsole_history_pos - 1);

	// don' t allow looping through history
	if(pos == qconsole_history_oldest)
		return;

	qconsole_history_pos = pos;
	Q_strncpyz(qconsole_line, qconsole_history[qconsole_history_pos], sizeof(qconsole_line));
	qconsole_linelen = strlen(qconsole_line);
}

/*
==================
CON_HistNext
==================
*/
static void CON_HistNext(void)
{
	int             pos;

	pos = (qconsole_history_pos >= QCONSOLE_HISTORY - 1) ? 0 : (qconsole_history_pos + 1);

	// clear the edit buffer if they try to advance to a future command
	if(pos == qconsole_history_oldest)
	{
		qconsole_line[0] = '\0';
		qconsole_linelen = 0;
		return;
	}

	qconsole_history_pos = pos;
	Q_strncpyz(qconsole_line, qconsole_history[qconsole_history_pos], sizeof(qconsole_line));
	qconsole_linelen = strlen(qconsole_line);
}

static void Sys_ShowConsole(int visLevel, qboolean quitOnClose)
{
	s_wcd.quitOnClose = quitOnClose;

	if(visLevel == s_wcd.visLevel)
	{
		return;
	}

	s_wcd.visLevel = visLevel;

	if(!s_wcd.hWnd)
		return;

	switch (visLevel)
	{
		case 0:
			ShowWindow(s_wcd.hWnd, SW_HIDE);
			break;
		case 1:
			ShowWindow(s_wcd.hWnd, SW_SHOWNORMAL);
			SendMessage(s_wcd.hwndBuffer, EM_LINESCROLL, 0, 0xffff);
			break;
		case 2:
			ShowWindow(s_wcd.hWnd, SW_MINIMIZE);
			break;
		default:
			Sys_Error("Invalid visLevel %d sent to Sys_ShowConsole\n", visLevel);
			break;
	}
}

static LONG WINAPI ConWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char           *cmdString;
	static qboolean s_timePolarity;

	switch (uMsg)
	{
		case WM_ACTIVATE:
#if defined(INPUTBOX)
			if(LOWORD(wParam) != WA_INACTIVE)
			{
				SetFocus(s_wcd.hwndInputLine);
			}
#endif

			if(com_viewlog && (com_dedicated && !com_dedicated->integer))
			{
				// if the viewlog is open, check to see if it's being minimized
				if(com_viewlog->integer == 1)
				{
					if(HIWORD(wParam))	// minimized flag
					{
						Cvar_Set("viewlog", "2");
					}
				}
				else if(com_viewlog->integer == 2)
				{
					if(!HIWORD(wParam))	// minimized flag
					{
						Cvar_Set("viewlog", "1");
					}
				}
			}
			break;

		case WM_CLOSE:
			if((com_dedicated && com_dedicated->integer))
			{
				cmdString = CopyString("quit");
				Com_QueueEvent(0, SE_CONSOLE, 0, 0, strlen(cmdString) + 1, cmdString);
			}
			else if(s_wcd.quitOnClose)
			{
				PostQuitMessage(0);
			}
			else
			{
				Sys_ShowConsole(0, qfalse);
				Cvar_Set("viewlog", "0");
			}
			return 0;
		
		case WM_CTLCOLORSTATIC:
			if((HWND) lParam == s_wcd.hwndBuffer)
			{
				SetBkColor((HDC) wParam, RGB(0x00, 0x00, 0xB0));
				SetTextColor((HDC) wParam, RGB(0xff, 0xff, 0x00));

#if 0							// this draws a background in the edit box, but there are issues with this
				if((hdcScaled = CreateCompatibleDC((HDC) wParam)) != 0)
				{
					if(SelectObject((HDC) hdcScaled, s_wcd.hbmLogo))
					{
						StretchBlt((HDC) wParam, 0, 0, 512, 384, hdcScaled, 0, 0, 512, 384, SRCCOPY);
					}
					DeleteDC(hdcScaled);
				}
#endif
				return (long)s_wcd.hbrEditBackground;
			}
			else if((HWND) lParam == s_wcd.hwndErrorBox)
			{
				if(s_timePolarity & 1)
				{
					SetBkColor((HDC) wParam, RGB(0x80, 0x80, 0x80));
					SetTextColor((HDC) wParam, RGB(0xff, 0x0, 0x00));
				}
				else
				{
					SetBkColor((HDC) wParam, RGB(0x80, 0x80, 0x80));
					SetTextColor((HDC) wParam, RGB(0x00, 0x0, 0x00));
				}
				return (long)s_wcd.hbrErrorBackground;
			}
			break;

		case WM_COMMAND:
			if(wParam == COPY_ID)
			{
				#if 1
				// old way 
				SendMessage(s_wcd.hwndBuffer, EM_SETSEL, 0, -1);
				SendMessage(s_wcd.hwndBuffer, WM_COPY, 0, 0);
				#else
				// ioquake3 way
				HGLOBAL         memoryHandle;
				char           *clipMemory;

				memoryHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, CON_LogSize() + 1);
				clipMemory = (char *)GlobalLock(memoryHandle);

				if(clipMemory)
				{
					char           *p = clipMemory;
					char            buffer[1024];
					unsigned int    size;

					while((size = CON_LogRead(buffer, sizeof(buffer))) > 0)
					{
						Com_Memcpy(p, buffer, size);
						p += size;
					}

					*p = '\0';

					if(OpenClipboard(NULL) && EmptyClipboard())
						SetClipboardData(CF_TEXT, memoryHandle);

					GlobalUnlock(clipMemory);
					CloseClipboard();
				}
				#endif
			}
			else if(wParam == QUIT_ID)
			{
				if(s_wcd.quitOnClose)
				{
					PostQuitMessage(0);
				}
				else
				{
					cmdString = CopyString("quit");
					Com_QueueEvent(0, SE_CONSOLE, 0, 0, strlen(cmdString) + 1, cmdString);
				}
			}
			else if(wParam == CLEAR_ID)
			{
				SendMessage(s_wcd.hwndBuffer, EM_SETSEL, 0, -1);
				SendMessage(s_wcd.hwndBuffer, EM_REPLACESEL, FALSE, (LPARAM) "");
				UpdateWindow(s_wcd.hwndBuffer);
			}
			break;
		
		case WM_CREATE:
//      s_wcd.hbmLogo = LoadBitmap( s_hInstance, MAKEINTRESOURCE( IDB_BITMAP1 ) );
//      s_wcd.hbmClearBitmap = LoadBitmap( s_hInstance, MAKEINTRESOURCE( IDB_BITMAP2 ) );
			s_wcd.hbrEditBackground = CreateSolidBrush(RGB(0x00, 0x00, 0xB0));
			s_wcd.hbrErrorBackground = CreateSolidBrush(RGB(0x80, 0x80, 0x80));
			SetTimer(hWnd, 1, 1000, NULL);
			break;
		
		case WM_ERASEBKGND:
#if 0
			HDC hdcScaled;
			HGDIOBJ         oldObject;

#if 1							// a single, large image
			hdcScaled = CreateCompatibleDC((HDC) wParam);
			assert(hdcScaled != 0);

			if(hdcScaled)
			{
				oldObject = SelectObject((HDC) hdcScaled, s_wcd.hbmLogo);
				assert(oldObject != 0);
				if(oldObject)
				{
					StretchBlt((HDC) wParam, 0, 0, s_wcd.windowWidth, s_wcd.windowHeight, hdcScaled, 0, 0, 512, 384, SRCCOPY);
				}
				DeleteDC(hdcScaled);
				hdcScaled = 0;
			}
#else							// a repeating brush
			{
				HBRUSH          hbrClearBrush;
				RECT            r;

				GetWindowRect(hWnd, &r);

				r.bottom = r.bottom - r.top + 1;
				r.right = r.right - r.left + 1;
				r.top = 0;
				r.left = 0;

				hbrClearBrush = CreatePatternBrush(s_wcd.hbmClearBitmap);

				assert(hbrClearBrush != 0);

				if(hbrClearBrush)
				{
					FillRect((HDC) wParam, &r, hbrClearBrush);
					DeleteObject(hbrClearBrush);
				}
			}
#endif
			return 1;
#endif
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		
		case WM_TIMER:
			if(wParam == 1)
			{
				s_timePolarity = !s_timePolarity;
				if(s_wcd.hwndErrorBox)
				{
					InvalidateRect(s_wcd.hwndErrorBox, NULL, FALSE);
				}
			}
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#if defined(INPUTBOX)
LONG WINAPI InputLineWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char            inputBuffer[1024];

	switch (uMsg)
	{
		case WM_KILLFOCUS:
			if((HWND) wParam == s_wcd.hWnd || (HWND) wParam == s_wcd.hwndErrorBox)
			{
				SetFocus(hWnd);
				return 0;
			}
			break;

		case WM_KEYDOWN:
			if(wParam == VK_RETURN)
			{
			#if 1
				GetWindowText(s_wcd.hwndInputLine, inputBuffer, sizeof(inputBuffer));
				Q_strncpyz(s_wcd.consoleText, inputBuffer, sizeof(s_wcd.consoleText) - strlen(s_wcd.consoleText) - 5);
				Q_strcat(s_wcd.consoleText, sizeof(s_wcd.consoleText), "\n");

				Q_strncpyz(qconsole_line, inputBuffer, sizeof(qconsole_line));
				qconsole_linelen = strlen(qconsole_line);

				if(!qconsole_linelen)
				{
					SetWindowText(s_wcd.hwndInputLine, "");
					break;
				}

				CON_HistAdd();

				qconsole_linelen = 0;
				SetWindowText(s_wcd.hwndInputLine, "");

				//Com_Printf(va("]%s\n", inputBuffer));
			#else

				if(!qconsole_linelen)
				{
					Com_Printf("\n");
					SetWindowText(s_wcd.hwndInputLine, "");
					break;
				}

				CON_HistAdd();
				//Com_Printf("%s\n", qconsole_line);

				//Q_strcat(s_wcd.consoleText, sizeof(s_wcd.consoleText), qconsole_line); 
				Q_strncpyz(s_wcd.consoleText, qconsole_line, sizeof(s_wcd.consoleText)); 
				SetWindowText(s_wcd.hwndInputLine, "");

				qconsole_linelen = 0;
			#endif
				return 0;
			}
			else if(wParam == VK_UP )
			{
				CON_HistPrev();

				SetWindowText(s_wcd.hwndInputLine, qconsole_line);
			}
			else if(wParam == VK_DOWN )
			{
				CON_HistNext();

				SetWindowText(s_wcd.hwndInputLine, qconsole_line);
			}
			else if(wParam == VK_TAB)
			{
				field_t         f;

				GetWindowText(s_wcd.hwndInputLine, inputBuffer, sizeof(inputBuffer));
				Q_strncpyz(qconsole_line, inputBuffer, sizeof(qconsole_line));

				Q_strncpyz(f.buffer, qconsole_line, sizeof(f.buffer));
				Field_AutoComplete(&f);
				Q_strncpyz(qconsole_line, f.buffer, sizeof(qconsole_line));
				qconsole_linelen = strlen(qconsole_line);

				if(qconsole_linelen > 1)
				{
					SetWindowText(s_wcd.hwndInputLine, &qconsole_line[1]);
				}
			}
			#if 0
			else
			{
				//GetWindowText(s_wcd.hwndInputLine, inputBuffer, sizeof(inputBuffer));

				if(qconsole_linelen < sizeof(qconsole_line) - 1)
				{
					char			c;
					WORD			lChar = 0;
					BYTE			keystate[256];

					//char            c = buff[i].Event.KeyEvent.uChar.AsciiChar;
					//char c = wParam;

					GetKeyboardState(keystate);
					ToAscii(wParam, lParam, keystate, &lChar, 1);
					c = lChar;
					c = wParam;

					if(wParam == VK_BACK)
					{
						int             pos = (qconsole_linelen > 0) ? qconsole_linelen - 1 : 0;

						qconsole_line[pos] = '\0';
						qconsole_linelen = pos;
					}
					else if(c)
					{
						qconsole_line[qconsole_linelen++] = c;
						qconsole_line[qconsole_linelen] = '\0';
					}
				}

				SetWindowText(s_wcd.hwndInputLine, qconsole_line);
			}
			#endif
			break;

	}

	return CallWindowProc(s_wcd.SysInputLineWndProc, hWnd, uMsg, wParam, lParam);
}
#endif

static HINSTANCE GetConsoleInstance()
{	
	HWND hwndC = GetConsoleWindow();

	HINSTANCE hInstC = GetModuleHandle( 0 ) ;
	//HINSTANCE hInstCons = (HINSTANCE)GetWindowLong( hwndC, GWL_HINSTANCE )

	return hInstC;
}





/*
==================
CON_SetVisibility
==================
*/
void CON_SetVisibility(int visLevel)
{
	Sys_ShowConsole(visLevel, qfalse);
}

/*
==================
CON_Shutdown
==================
*/
void CON_Shutdown(void)
{
	if(s_wcd.hWnd)
	{
		ShowWindow(s_wcd.hWnd, SW_HIDE);
		CloseWindow(s_wcd.hWnd);
		DestroyWindow(s_wcd.hWnd);
		s_wcd.hWnd = 0;
	}
}

/*
==================
CON_Init
==================
*/
void CON_Init(void)
{
	int				i;
	HDC             hDC;
	WNDCLASS        wc;
	RECT            rect;
	const char     *DEDCLASS = "XreaL WinConsole";
	int             nHeight;
	int             swidth, sheight;
	int             DEDSTYLE = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX;

	memset(&wc, 0, sizeof(wc));

	// get application instance first
	s_hInstance = GetConsoleInstance();

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC) ConWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = s_hInstance;
//	wc.hIcon = LoadIcon(s_hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (void *)COLOR_WINDOW;
	wc.lpszMenuName = 0;
	wc.lpszClassName = DEDCLASS;

	if(!RegisterClass(&wc))
		return;

	rect.left = 0;
	rect.right = 540;
	rect.top = 0;
	rect.bottom = 450;
	AdjustWindowRect(&rect, DEDSTYLE, FALSE);

	hDC = GetDC(GetDesktopWindow());
	swidth = GetDeviceCaps(hDC, HORZRES);
	sheight = GetDeviceCaps(hDC, VERTRES);
	ReleaseDC(GetDesktopWindow(), hDC);

	s_wcd.windowWidth = rect.right - rect.left + 1;
	s_wcd.windowHeight = rect.bottom - rect.top + 1;

	s_wcd.hWnd = CreateWindowEx(0,
								DEDCLASS,
								"XreaL Console",
								DEDSTYLE,
								(swidth - 600) / 2, (sheight - 450) / 2, rect.right - rect.left + 1, rect.bottom - rect.top + 1,
								NULL, NULL, s_hInstance, NULL);

	if(s_wcd.hWnd == NULL)
	{
		return;
	}

	// create fonts
	hDC = GetDC(s_wcd.hWnd);
	nHeight = -MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72);

	s_wcd.hfBufferFont = CreateFont(nHeight,
									0,
									0,
									0,
									FW_LIGHT,
									0,
									0,
									0,
									DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN | FIXED_PITCH, "Courier New");

	ReleaseDC(s_wcd.hWnd, hDC);


	// create the input line
#if defined(INPUTBOX)
	s_wcd.hwndInputLine = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, 6, 400, 528, 20, s_wcd.hWnd, (HMENU) INPUT_ID,	// child window ID
									   s_hInstance, NULL);
#endif
	
	// create the buttons
	s_wcd.hwndButtonCopy = CreateWindow("button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 5, 425, 72, 24, s_wcd.hWnd, (HMENU) COPY_ID,	// child window ID
										s_hInstance, NULL);
	SendMessage(s_wcd.hwndButtonCopy, WM_SETTEXT, 0, (LPARAM) "copy");

	s_wcd.hwndButtonClear = CreateWindow("button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 82, 425, 72, 24, s_wcd.hWnd, (HMENU) CLEAR_ID,	// child window ID
										 s_hInstance, NULL);
	SendMessage(s_wcd.hwndButtonClear, WM_SETTEXT, 0, (LPARAM) "clear");

	s_wcd.hwndButtonQuit = CreateWindow("button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 462, 425, 72, 24, s_wcd.hWnd, (HMENU) QUIT_ID,	// child window ID
										s_hInstance, NULL);
	SendMessage(s_wcd.hwndButtonQuit, WM_SETTEXT, 0, (LPARAM) "quit");


	
	// create the scrollbuffer
	s_wcd.hwndBuffer = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 6, 40, 526, 354, s_wcd.hWnd, (HMENU) EDIT_ID,	// child window ID
									s_hInstance, NULL);
	SendMessage(s_wcd.hwndBuffer, WM_SETFONT, (WPARAM) s_wcd.hfBufferFont, 0);

#if defined(INPUTBOX)
	s_wcd.SysInputLineWndProc = (WNDPROC) SetWindowLong(s_wcd.hwndInputLine, GWL_WNDPROC, (long)InputLineWndProc);
	SendMessage(s_wcd.hwndInputLine, WM_SETFONT, (WPARAM) s_wcd.hfBufferFont, 0);
#endif

	ShowWindow(s_wcd.hWnd, SW_SHOWDEFAULT);
	UpdateWindow(s_wcd.hWnd);
	SetForegroundWindow(s_wcd.hWnd);

#if defined(INPUTBOX)
	SetFocus(s_wcd.hwndInputLine);
#endif

	s_wcd.visLevel = 1;

	// initialize history
	for(i = 0; i < QCONSOLE_HISTORY; i++)
		qconsole_history[i][0] = '\0';
}

/*
==================
CON_Input
==================
*/
char           *CON_Input(void)
{
#if defined(INPUTBOX)
#if 1
	MSG             msg;

	// pump the message loop
	while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if(!GetMessage(&msg, NULL, 0, 0))
		{
			Com_Quit_f();
		}

		// save the msg time, because wndprocs don't have access to the timestamp
		//g_wv.sysMsgTime = msg.time;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif

	if(s_wcd.consoleText[0] == 0)
	{
		return NULL;
	}

	strcpy(s_wcd.returnedText, s_wcd.consoleText);
	s_wcd.consoleText[0] = 0;

	return s_wcd.returnedText;
#else
	return NULL;
#endif
}

/*
==================
CON_Print
==================
*/
void CON_Print(const char *pMsg)
{
#if 0
	#define CONSOLE_BUFFER_SIZE		16384

	static char		consoleBuffer[CONSOLE_BUFFER_SIZE * 2];
	char            buffer[1024];
	char           *p = consoleBuffer;
	unsigned int    size;

	consoleBuffer[0] = '\0';
	while((size = CON_LogRead(buffer, sizeof(buffer))) > 0)
	{
		Q_strcat(consoleBuffer, sizeof(consoleBuffer), buffer);
		//Com_Memcpy(p, buffer, size);
		//p += size;
	}

	*p = '\0';

	// put this text into the windows console
	SendMessage(s_wcd.hwndBuffer, EM_LINESCROLL, 0, 0xffff);
	SendMessage(s_wcd.hwndBuffer, EM_SCROLLCARET, 0, 0);
	SendMessage(s_wcd.hwndBuffer, EM_REPLACESEL, 0, (LPARAM) buffer);

#elif 1

	//#define CONSOLE_BUFFER_SIZE		(16384 / 2)
	#define CONSOLE_BUFFER_SIZE		4096

	static char     buffer[CONSOLE_BUFFER_SIZE * 2];
	char           *b = buffer;
	const char     *msg;
	int             bufLen;
	int             i = 0;
	static unsigned long s_totalChars;

	// if the message is REALLY long, use just the last portion of it
	if(strlen(pMsg) > CONSOLE_BUFFER_SIZE - 1)
	{
		msg = pMsg + strlen(pMsg) - CONSOLE_BUFFER_SIZE + 1;
	}
	else
	{
		msg = pMsg;
	}

	// copy into an intermediate buffer
	while(msg[i] && ((b - buffer) < sizeof(buffer) - 1))
	{
		if(msg[i] == '\n' && msg[i + 1] == '\r')
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
			i++;
		}
		else if(msg[i] == '\r')
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		}
		else if(msg[i] == '\n')
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		}
		else if(Q_IsColorString(&msg[i]))
		{
			i++;
		}
		else
		{
			*b = msg[i];
			b++;
		}
		i++;
	}
	*b = 0;
	bufLen = b - buffer;

	s_totalChars += bufLen;

	// replace selection instead of appending if we're overflowing
	if(s_totalChars > 0x7fff)
	{
		SendMessage(s_wcd.hwndBuffer, EM_SETSEL, 0, -1);
		s_totalChars = bufLen;
	}
	
	// put this text into the windows console
	SendMessage(s_wcd.hwndBuffer, EM_LINESCROLL, 0, 0xffff);
	SendMessage(s_wcd.hwndBuffer, EM_SCROLLCARET, 0, 0);
	SendMessage(s_wcd.hwndBuffer, EM_REPLACESEL, 0, (LPARAM) buffer);
#endif
}

void CON_SetErrorText(const char *buf)
{
	s_wcd.quitOnClose = qtrue;

	Q_strncpyz(s_wcd.errorString, buf, sizeof(s_wcd.errorString));

	if(!s_wcd.hwndErrorBox)
	{
		s_wcd.hwndErrorBox = CreateWindow("static", NULL, WS_CHILD | WS_VISIBLE | SS_SUNKEN, 6, 5, 526, 30, s_wcd.hWnd, (HMENU) ERRORBOX_ID,	// child window ID
										  s_hInstance, NULL);
		SendMessage(s_wcd.hwndErrorBox, WM_SETFONT, (WPARAM) s_wcd.hfBufferFont, 0);
		SetWindowText(s_wcd.hwndErrorBox, s_wcd.errorString);

#if defined(INPUTBOX)
		DestroyWindow(s_wcd.hwndInputLine);
		s_wcd.hwndInputLine = NULL;
#endif
	}
}