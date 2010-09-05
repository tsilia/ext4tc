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
