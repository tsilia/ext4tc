/*
    This file is part of Ext4tc - plugin for Total Commander.

    Ext4tc is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation version 3 of the License

    Ext4tc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Ext4tc.  If not, see <http://www.gnu.org/licenses/>.

    Ext4tc  Copyright (C) 2010  Krzysztof Stasiak mail: krzychuu.stasiak@gmail.com

 */

#ifdef _DEBUG
#include "debugWindowCls.h"
#include <RichEdit.h>
#include <Windowsx.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
//#include <time.h>

debugWindowCls::debugWindowCls(HINSTANCE hInst, int nCmdShow, char *clsname, char *title):windowCls(hInst, nCmdShow, clsname, title)
{
	CHARFORMAT font = {0};
	hRichEdit = LoadLibrary("RICHED32.DLL") ;
	this->init();
	this->setSize(700, 400);
	SetWindowLongPtr(this->hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
	hwnd_rich = CreateWindowEx(WS_EX_CLIENTEDGE, "RichEdit", "", WS_CHILD | WS_VISIBLE | WS_HSCROLL | 
		WS_VSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 200, 150, this->hWnd, 0, hInst, 0) ;

	font.dwMask = CFM_COLOR;
	font.cbSize = sizeof(CHARFORMAT);
	font.crTextColor = RGB(0, 255, 0);
	SendMessage(hwnd_rich, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&font);
	SendMessage(hwnd_rich, EM_SETBKGNDCOLOR, 0, RGB(0, 0, 0));

}

/**
 *
 */

debugWindowCls::~debugWindowCls()
{
	FreeLibrary(hRichEdit);
}

/**
 *
 */

LRESULT CALLBACK debugWindowCls::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

		case WM_SIZE:
			{
				int width = LOWORD(lParam);
				int height = HIWORD(lParam);
				SetWindowPos(this->hwnd_rich, HWND_NOTOPMOST, 0, 0, 
					width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
				break;
			}		
		case WM_CLOSE:
			{
				ShowWindow(this->hWnd, SW_HIDE);
				break;
			}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(this->hWnd, message, wParam, lParam);
	}
	return 0;
}
void debugWindowCls::appendTextNoTimestamp(char *format, ... )
{
	va_list args;
	char *buffer = NULL, *formatW = NULL;
	int len = 0, wcharLen = 0;

	va_start(args, format );


	len = _vscprintf( format, args ) + 1;

	buffer = new char[len + 20];
	buffer[0] = '\0';

	vsprintf(buffer + lstrlen(buffer), format, args );	
	SendMessage(this->hwnd_rich, EM_SETSEL, -1, -1);
	SendMessage(this->hwnd_rich, EM_REPLACESEL, (WPARAM)0, (LPARAM)buffer);
	delete [] buffer;
}

void debugWindowCls::appendText(char *format, ... )
{
	va_list args;
	char *buffer = NULL, *formatW = NULL;
	int len = 0, wcharLen = 0;

	va_start(args, format );


	len = _vscprintf( format, args ) + 1;

	buffer = new char[len + 20];
	sprintf(buffer, "%d: ", GetTickCount());

	vsprintf(buffer + lstrlen(buffer), format, args );	
	SendMessage(this->hwnd_rich, EM_SETSEL, -1, -1);
	SendMessage(this->hwnd_rich, EM_REPLACESEL, (WPARAM)0, (LPARAM)buffer);
	delete [] buffer;
}
#endif