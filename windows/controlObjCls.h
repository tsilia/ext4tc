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
