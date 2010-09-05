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
