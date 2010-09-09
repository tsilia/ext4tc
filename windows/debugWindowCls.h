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

#ifndef __DEBUG_WINDOW_H__
#define __DEBUG_WINDOW_H__

#include "windowCls.h"

#define FILE_COMPIL(ptr) {(ptr) = __FILEW__; \
						   (ptr) = wcsrchr(ptr, L'\\'); \
							if((ptr) == NULL) \
								(ptr) = __FILEW__; \
							else \
								(ptr)++;}

class debugWindowCls : public windowCls
{
	HINSTANCE hRichEdit;
	HWND hwnd_rich;
public:
	debugWindowCls(HINSTANCE hInst, int nCmdShow, char *clsname, char *title);
	~debugWindowCls();
	void appendText(char *format, ...);
	void appendTextNoTimestamp(char *format, ... );
protected:
	virtual LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
