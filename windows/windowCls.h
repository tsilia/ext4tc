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

#ifndef __WINDOW_CLS_H__
#define __WINDOW_CLS_H__

#include "windows.h"
#include "controlObjCls.h"

//#include "resource.h"
//#include "afxres.h"

class windowCls : public controlObjCls
{
protected:
	char wClsName[100]; /** window class name*/
	char wTitleName[100]; /** window title*/
	int showMode;
public:
	windowCls(HINSTANCE hInst, int nCmdShow, char *clsname, char *title);
	void show(int nCmdShow);
protected:
	void init();
	ATOM RegisterWindowClass(HINSTANCE hInstance);
	BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
	/** appropriate window callback */
	virtual LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
	/** static callback, invokes corresponding window callback*/
	static LRESULT CALLBACK staticCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
