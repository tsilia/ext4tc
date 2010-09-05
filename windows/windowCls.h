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
