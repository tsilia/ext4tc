#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifdef _DEBUG
#include "ext4PluginDescr.h"
extern ext4PluginDescr *pluginDescr;
#define LOG_MESSAGE(FORMAT,...) \
	do {\
	pluginDescr->dWin.appendText(FORMAT, __VA_ARGS__);\
	}while(0)
#else
#define LOG_MESSAGE(FORMAT,...)
#endif

#endif /*__LOGGER_H__*/