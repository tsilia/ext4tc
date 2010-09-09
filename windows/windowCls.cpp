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
#include "windowCls.h"
#include <commctrl.h>

/**
 *
 */

windowCls::windowCls(HINSTANCE hInst, int nCmdShow, char *clsname, char *title) : controlObjCls(hInst)
{
	hInstance = hInst;
	showMode = nCmdShow; 
	strncpy_s(wTitleName, 100, title, 99);
	strncpy_s(wClsName, 100, clsname, 99);
}

/**
 *
 */

void windowCls::init()
{
	this->RegisterWindowClass(this->hInstance);
	this->InitInstance(this->hInstance, this->showMode);
}

/**
 * RegisterWindowClass - registers new window class
 * @hInstance - handler to the parrent
 */

ATOM windowCls::RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_DBLCLKS/*CS_HREDRAW | CS_VREDRAW*/;
	wcex.lpfnWndProc	= &windowCls::staticCallback;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(windowCls *);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	/** Menu*/
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= wClsName;
	wcex.hIconSm		= NULL;	

	return RegisterClassEx(&wcex);
}

/**
 *
 */

BOOL windowCls::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	this->hWnd = CreateWindow(wClsName, wTitleName, WS_OVERLAPPEDWINDOW,
	  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if(!this->hWnd)
	{
		return FALSE;
	}
	SetWindowLongPtr(this->hWnd, GWLP_USERDATA, (LONG_PTR)this);
	
	return TRUE;
}

/**
 *
 */

LRESULT CALLBACK windowCls::staticCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
		windowCls *instance = reinterpret_cast<windowCls *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if(instance == NULL)
			return DefWindowProc(hwnd, message, wParam, lParam);
		return instance->WindowProcedure(hwnd, message, wParam, lParam);
}

/**
 *
 */

void windowCls::show(int nCmdShow)
{
	ShowWindow(this->hWnd, nCmdShow);
	UpdateWindow(this->hWnd);
}

#endif