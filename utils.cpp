#include <windows.h>
#include "utils.h"

int MultibyteToMultibyte(UINT dwSrcCodePage, char *lpSrcStr, int cbSrcStr,   
                         UINT dwDestCodePage, char *lpDestStr, int cbDestStr)   
{   
    WCHAR *wzTemp = NULL;   
    int nSrcLen = 0;   
   
    if (dwSrcCodePage == dwDestCodePage)   
    {   
        return 0;   
    }   
   
    if (lpSrcStr == NULL)   
    {   
        return 0;   
    }   
   
    if (-1 == cbSrcStr)   
    {   
        nSrcLen = strlen(lpSrcStr) + 1;   
    }   
    else   
    {   
        nSrcLen = cbSrcStr;   
    }   
   
    if ((NULL == lpDestStr) || (0 == cbDestStr))   
    {   
        return nSrcLen;   
    }   
   
    wzTemp = new WCHAR[nSrcLen + 1];   
    if (NULL == wzTemp)   
    {   
        return 0;   
    }   
    memset(wzTemp, 0, (nSrcLen + 1) * sizeof(WCHAR));   
    MultiByteToWideChar(dwSrcCodePage, 0, lpSrcStr, cbSrcStr, wzTemp, nSrcLen); 
    nSrcLen = WideCharToMultiByte(dwDestCodePage, 0, wzTemp, -1, lpDestStr, cbDestStr, NULL, FALSE);   
   
    delete [] wzTemp;   
    wzTemp = NULL;   
       
    return nSrcLen;   
}