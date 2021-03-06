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

#ifndef __PARTITION_H__
#define __PARTITION_H__

#include <windows.h>
/**********************************************************
	PARTITION class
***********************************************************/

class partition
{
protected:
	BOOL active;
	BOOL primary; //TRUE - primary, FALSE - logical
	unsigned int sector_size;
	unsigned char type;
	unsigned int H_start, H_end; //head
	unsigned char S_start, S_end; //sector
	unsigned int C_start, C_end; //cylinder
	unsigned int LBA;
	unsigned int n_sectors;
	unsigned long long offset;
	unsigned int part_no;
	HANDLE part_handler; //like disk_handler

public:
	partition();
	virtual ~partition(){}
	void set_part_info(unsigned char *buffer);
	void set_part_handler(HANDLE h=INVALID_HANDLE_VALUE){this->part_handler=h;}
	unsigned int get_LBA(){return LBA;}
	unsigned char get_type(){return type;}
	void set_type(unsigned char t){this->type = t;}
	unsigned char *read_sectors(DWORD s_start, unsigned int n_sectors);
	unsigned char *read_sectors_ex(unsigned long long s_start, unsigned int n_sectors);
	void set_logic(BOOL value=TRUE){primary = !value;}
	void set_offset(unsigned long long p_offs){offset = p_offs;}
	unsigned long long get_offset(){return offset;}
	unsigned int get_part_no(){return this->part_no;}
	void set_part_no(unsigned int i){this->part_no = i;}
	unsigned int get_sector_size(){ return this->sector_size; }
	void set_sector_size(unsigned int size){ this->sector_size = size; }
};

#endif
