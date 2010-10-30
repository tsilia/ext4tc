/**
 *
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

// ext4Plugin.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <shellapi.h>
#include "ext4plugin.h"
#include "ext4PluginDescr.h"
#include <fstream>

#define pluginrootlen 1

#ifdef _MANAGED
#pragma managed(push, off)
#endif

ext4PluginDescr pluginDescr;

struct thCopyParam
{
//	char file_path[256];
	partition_linux_ext2 *partition;
	unsigned int inode_num;
	HANDLE ext2_hnd;
	int error;
};

typedef struct {
	char Path[MAX_PATH];
	char LastFoundName[MAX_PATH];
	int disk_no;
	int part_no_map;
	int num_last_entry;
	HANDLE searchhandle;
} tLastFindStuct,*pLastFindStuct;

DWORD WINAPI copy_thread_subroutine(LPVOID p);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


BOOL IsUserAdmin(VOID)
/*++ 
Routine Description: This routine returns TRUE if the caller's
process is a member of the Administrators local group. Caller is NOT
expected to be impersonating anyone and is expected to be able to
open its own process and process token. 
Arguments: None. 
Return Value: 
   TRUE - Caller has Administrators local group. 
   FALSE - Caller does not have Administrators local group. --
*/ 
{
	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup; 
	b = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup); 
	if(b) 
	{
		if (!CheckTokenMembership( NULL, AdministratorsGroup, &b)) 
		{
			 b = FALSE;
		} 
		FreeSid(AdministratorsGroup); 
	}

	return(b);
}



/**
 * FsInit - plugin init function
 */

int __stdcall FsInit(int PluginNr,tProgressProc pProgressProc,tLogProc pLogProc,tRequestProc pRequestProc)
{
	int hdd_count = -1;
	pluginDescr.ProgressProc = pProgressProc;
    pluginDescr.LogProc = pLogProc;
    pluginDescr.RequestProc = pRequestProc;
	pluginDescr.PluginNumber = PluginNr;
	
	if (!IsUserAdmin())
	{
		MessageBox(NULL, (LPCTSTR)"You are not admin", TEXT("INFO"), MB_OK); 
		return -1;
	}
#ifdef _DEBUG
	pluginDescr.dWin.show(SW_NORMAL);
	pluginDescr.dWin.appendText("----======= Ext2Viewer written by Krzysztof Stasiak =======----\n");
#endif
	if (pluginDescr.search_system_for_linux_ext2_partitions() == -1)
	{
		return -1;
	}
	pluginDescr.set_plugin_initialized(true);
	return 0;
}


HANDLE __stdcall FsFindFirst(char* Path, WIN32_FIND_DATA *FindData)
{
	pLastFindStuct lf;
	partition_linux_ext2 **ext2_partitions = NULL;
	int disk_no, part_no;

	if (!pluginDescr.get_plugin_initialized() || pluginDescr.ext4_partitions_id_map == NULL)
	{
		return INVALID_HANDLE_VALUE;
	}

	memset(FindData,0,sizeof(WIN32_FIND_DATA));
	if (strcmp(Path, "\\") == 0) {
		int hd_index = -1, part_index = -1;
		FindData->dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
		FindData->ftLastWriteTime.dwHighDateTime=0xFFFFFFFF;
		FindData->ftLastWriteTime.dwLowDateTime=0xFFFFFFFE;
		pluginDescr.clear_current_path();

		if (pluginDescr.get_first_ext4_disk_and_part_no(&disk_no, &part_no) == -1)
			return INVALID_HANDLE_VALUE;

		ext2_partitions = pluginDescr.hard_disks[disk_no]->get_partitions_ext2();
		if (ext2_partitions == NULL) {
			return INVALID_HANDLE_VALUE;
		}

		char buff[10] = {0};
		//TODO
		char disk_id_char = disk_no + 'a';
		sprintf_s(FindData->cFileName, sizeof(FindData->cFileName), "%s%c%d", DEVICE_HDD_DISPLAY_PREFIX, disk_id_char, part_no);
#ifdef _DEBUG2
		pluginDescr.dWin.appendText(PART_PREFIX"%d Inode size: %d \n", 0, ext2_partitions[0]->ext2->ext4_sb.s_inode_size);
#endif
		lf=(pLastFindStuct)malloc(sizeof(tLastFindStuct));
		strncpy_s(lf->Path, strlen(Path)+1, Path, strlen(Path));
		lf->searchhandle=INVALID_HANDLE_VALUE;
		lf->disk_no = disk_no;
		lf->part_no_map = pluginDescr.get_partition_index_via_real_number(disk_no, part_no);
		strcpy(lf->LastFoundName, FindData->cFileName);
		return (HANDLE)lf;
	} 
	else
	{
		ext4_dir_entry_2 *dir = NULL;
		int num_last_entry = 0;		
		size_t len = strlen(PART_PREFIX);
		char *ptr = strstr(Path, "\\"PART_PREFIX);

		int ret = pluginDescr.extract_disk_and_part_no(Path, &disk_no, &part_no);
		if (ret == -1)
			return INVALID_HANDLE_VALUE;
		int part_no_map = pluginDescr.get_partition_index_via_real_number(disk_no, part_no);

		ext2_partitions = pluginDescr.hard_disks[disk_no]->get_partitions_ext2();
		if (ext2_partitions == NULL) {
			return INVALID_HANDLE_VALUE;
		}
#ifdef _DEBUG
		pluginDescr.dWin.appendText("Path %s ret: %d disk_no: %d part_no %d part_inumndex %d\n", 
			Path, ret, disk_no, part_no, pluginDescr.get_partition_index_via_real_number(disk_no, part_no));
		/*pluginDescr.dWin.appendText("Path %s ret: %d disk_no: %d part_no %d 'z'-'a'+1=%d\n", 
				Path2, ret, disk_no, part_no, 'z'-'a'+1);*/
#endif
		
		if (pluginDescr.get_current_path() == NULL)
		{
			dir = pluginDescr.get_first_dir_entry(Path, disk_no, part_no_map, &num_last_entry);			
		}
		else
		{
			//if goes up
			//should be optimalized with get_first_dir_entry2
			
			dir = pluginDescr.get_first_dir_entry(Path, disk_no, part_no_map, &num_last_entry);
		}
		if (dir != NULL){
			if (dir->file_type != EXT4_FT_DIR)
			{
				ext4_inode *inode = NULL;
				inode = pluginDescr.get_ext2_inode(disk_no, part_no_map, dir->inode);
				FindData->nFileSizeLow = inode->i_size_lo;
				if (dir->file_type == EXT4_FT_REG_FILE)
					FindData->nFileSizeHigh = inode->i_size_high;
			}
			else
			{
				FindData->nFileSizeLow = 0;
			}
			RETURN_FILE(FindData, dir);
		}
		else
		{
			strcpy(FindData->cFileName, "..");
			FindData->dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
			FindData->ftLastWriteTime.dwHighDateTime=0xFFFFFFFF;
			FindData->ftLastWriteTime.dwLowDateTime=0xFFFFFFFE;
		}
		lf=(pLastFindStuct)malloc(sizeof(tLastFindStuct));
		strncpy_s(lf->Path, strlen(Path)+1, Path, strlen(Path));
		lf->searchhandle = 0;
		lf->disk_no = disk_no;
		lf->part_no_map = pluginDescr.get_partition_index_via_real_number(disk_no, part_no);
		lf->num_last_entry = num_last_entry;
		return (HANDLE)lf;
	}
	return INVALID_HANDLE_VALUE;
}

BOOL __stdcall FsFindNext(HANDLE Hdl,WIN32_FIND_DATA *FindData)
{
	pLastFindStuct lf;
	int disk_no, part_no_map;

	if (Hdl == (HANDLE)1)
		return false;

	lf=(pLastFindStuct)Hdl;
	disk_no = lf->disk_no;
	part_no_map = lf->part_no_map;

	if (lf->searchhandle == INVALID_HANDLE_VALUE) 
	{
		if(pluginDescr.get_next_ext4_disk_and_part_no(&disk_no, &part_no_map) == -1)
			return false;

		partition_linux_ext2 **ext2_partitions = pluginDescr.hard_disks[disk_no]->get_partitions_ext2();
		if (ext2_partitions[part_no_map] == NULL)
			return false;

		lf->disk_no = disk_no;
		lf->part_no_map = part_no_map;
		char disk_id_char = disk_no + 'a';
		sprintf_s(FindData->cFileName, sizeof(FindData->cFileName), "%s%c%d", DEVICE_HDD_DISPLAY_PREFIX, 
					disk_id_char, ext2_partitions[part_no_map]->get_part_no());
		strcpy(lf->LastFoundName, FindData->cFileName);
#ifdef _DEBUG2
		pluginDescr.dWin.appendText(PART_PREFIX"%d Inode size: %d \n", lf->num, pluginDescr.ext2_part[lf->num]->ext2->ext4_sb.s_inode_size);
#endif
		return true;
	}
	else
	{		
		ext4_dir_entry_2 *dir = NULL;
		dir = pluginDescr.get_next_dir_entry(&lf->num_last_entry);
		if (dir != NULL)
		{
			if (dir->file_type != EXT4_FT_DIR)
			{
				ext4_inode *inode = NULL;
				inode = pluginDescr.get_ext2_inode(lf->disk_no, lf->part_no_map, dir->inode);
				FindData->nFileSizeLow = inode->i_size_lo;
				if (dir->file_type == EXT4_FT_REG_FILE)
					FindData->nFileSizeHigh = inode->i_size_high;
			}
			else
			{
				FindData->nFileSizeLow = 0;
			}
			RETURN_FILE(FindData, dir);

#ifdef _DEBUG2
//			pluginDescr.dWin.appendText("%s filename: %s\n", lf->Path, FindData->cFileName);
#endif
			return true;
		}
	}
	return false;
}



int __stdcall FsFindClose(HANDLE Hdl)
{	
	if (Hdl == INVALID_HANDLE_VALUE)
	{
#ifdef _DEBUG2
		pluginDescr.dWin.appendText("fsfindclose invalid\n");
#endif
		return 0;
	}
#ifdef _DEBUG2
	pluginDescr.dWin.appendText("fsfindclose\n");
#endif
	pLastFindStuct lf;
	lf=(pLastFindStuct)Hdl;
	if (lf->searchhandle!=INVALID_HANDLE_VALUE) {
		//FindClose(lf->searchhandle);
		lf->searchhandle=INVALID_HANDLE_VALUE;
	}
	free(lf);
	return 0;
}


int __stdcall FsGetFile(char* RemoteName,char* LocalName,int CopyFlags, RemoteInfoStruct* ri)
{
	thCopyParam params = {0};
	size_t len = strlen(PART_PREFIX);
	HANDLE hTh = INVALID_HANDLE_VALUE;
	unsigned int inode_num = 0;
	unsigned long long fsize = 0, pos = 0;
	ext4_inode *inode = NULL;
	char *ptr = strstr(RemoteName, "\\"PART_PREFIX);
	WCHAR *buffer = NULL;
	int wcharLen = 0;
	HANDLE hnd = INVALID_HANDLE_VALUE;
	LARGE_INTEGER large_pos = {0};
	int disk_no, part_no;

	if (pluginDescr.extract_disk_and_part_no(RemoteName, &disk_no, &part_no) == -1)
		return FS_FILE_NOTFOUND;

	int part_no_map = pluginDescr.get_partition_index_via_real_number(disk_no, part_no);
	if((inode = pluginDescr.get_ext2_inode(disk_no, part_no_map, RemoteName, &inode_num)) == NULL)
	{
#ifdef _DEBUG
		pluginDescr.dWin.appendText("File %s was not found\n", RemoteName);
#endif
		return FS_FILE_NOTFOUND;
	}
#ifdef _DEBUG
	pluginDescr.dWin.appendText("inode: %u mode: %#0x, %o\n", inode_num, inode->i_mode, inode->i_mode);
#endif
	if(!S_ISREG(inode->i_mode))	
	{
		return FS_FILE_NOTSUPPORTED;
	}
	wcharLen = MultiByteToWideChar(CP_ACP, 0, LocalName, (int)strlen(LocalName), NULL, 0) + 2;
	buffer = new WCHAR[wcharLen];
	memset(buffer, 0, sizeof(WCHAR) * wcharLen);
	MultiByteToWideChar(CP_ACP, 0, LocalName, (int)strlen(LocalName), buffer, (int)strlen(LocalName));
	

	if ((hnd = CreateFile(LocalName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
#ifdef _DEBUG2
		pluginDescr.dWin.appendText("%s FS_FILE_WRITEERROR\n", LocalName);
#endif
		return FS_FILE_WRITEERROR;
	}
	params.ext2_hnd = hnd;
	partition_linux_ext2 **ext2_partitions = pluginDescr.hard_disks[disk_no]->get_partitions_ext2();
	params.partition = ext2_partitions[part_no_map];
	params.inode_num = inode_num;

	pluginDescr.ProgressProc(pluginDescr.PluginNumber, RemoteName, LocalName, 0);
#ifdef _DEBUG
	int startTicks = GetTickCount();
#endif
	hTh = CreateThread(NULL, 0, copy_thread_subroutine, &params, 0, NULL);
	fsize = (unsigned long long)inode->i_size_high << 32;
	fsize |= (unsigned long long)inode->i_size_lo;

	
	while(1)
	{
		if (GetFileSizeEx(hnd, &large_pos) == 0 || params.error != 0)
		{
#ifdef _DEBUG
			pluginDescr.dWin.appendText("An error %d occured while reading %s\n", params.error, RemoteName);
#endif			
			CloseHandle(hnd);
			WaitForSingleObject(hTh, INFINITE);
			CloseHandle(hTh);
			return FS_FILE_READERROR;
		}
		if ((unsigned long long)large_pos.QuadPart >= fsize)
			break;

		if(pluginDescr.ProgressProc(pluginDescr.PluginNumber, RemoteName,
			LocalName, (int)(((double)large_pos.QuadPart / fsize)*100)) != 0)
		{
			CloseHandle(hnd);
			WaitForSingleObject(hTh, INFINITE);
			CloseHandle(hTh);
			return FS_FILE_USERABORT;
		}
	}	
#ifdef _DEBUG
	pluginDescr.dWin.appendText("Finished copying file %s in %d ms\n", RemoteName, GetTickCount() - startTicks);
#endif			
	CloseHandle(hnd);
	WaitForSingleObject(hTh, INFINITE);
	CloseHandle(hTh);
	return FS_FILE_OK;
}

DWORD WINAPI copy_thread_subroutine(LPVOID p)
{
	thCopyParam *params = (thCopyParam *)p;
	params->partition->read_file(params->inode_num, params->ext2_hnd, &params->error);
	return 0;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

