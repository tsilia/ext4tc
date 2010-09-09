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

#ifndef __CONTROLOBJ_CLS_H__
#define __CONTROLOBJ_CLS_H__

#include <windows.h>


class controlObjCls
{
protected:
	HINSTANCE hInstance;
	HWND hWnd; /** window handler*/
	int controlID;
	DWORD style;
public:
	controlObjCls(HINSTANCE h){this->hInstance = h; this->hWnd = 0;}
	HWND getHandler(){return this->hWnd;}
	int getControlID(){return this->controlID;}
	void setControlID(int id){this->controlID = id;}
	int setStyle(DWORD s){SetWindowLongPtr(this->hWnd, GWL_STYLE, s);}
	DWORD getStyle(){return this->style;}
	BOOL setSize(int width, int height);
	BOOL setPos(int x, int y);
	SIZE getSize();
	void add(controlObjCls *c);
};

#endif
