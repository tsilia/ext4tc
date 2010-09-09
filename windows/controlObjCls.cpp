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

#include "controlObjCls.h"
#include <Windows.h>
#include <WinUser.h>

BOOL controlObjCls::setSize(int width, int height)
{
	return SetWindowPos(this->hWnd, HWND_NOTOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL controlObjCls::setPos(int x, int y)
{
	return SetWindowPos(this->hWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER |SWP_NOACTIVATE);
}

SIZE controlObjCls::getSize()
{
	RECT rc;
	SIZE s;
	GetClientRect(this->hWnd, &rc); 
	s.cx = rc.right - rc.left;
	s.cy = rc.bottom - rc.top;	
	return s;
}

void controlObjCls::add(controlObjCls *c)
{
	int code = 0;
	code = SetWindowLongPtr(c->getHandler(), GWL_STYLE, (LONG)(WS_CHILD| WS_VISIBLE | (c->getStyle())));
	SetParent(c->getHandler(), this->hWnd);
}
