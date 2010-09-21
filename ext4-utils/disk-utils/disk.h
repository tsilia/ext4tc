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

#ifndef __DISK_H__
#define __DISK_H__

#include <windows.h>
#include <winioctl.h>

#include "partition_list.h"
#include "partition_linux_ext2.h"

/**********************************************************
	DISK class
***********************************************************/
#define SECTOR_SIZE 512

class disk
{
	HANDLE disk_handler;
	long disk_pos;
	unsigned int sector_size;
	partition_list *partitions;
	partition_linux_ext2 **partitions_ext2;
	int partitions_ext2_count;

	BOOL set_handler(char *device);
private:
	int get_disk_geometry(PDISK_GEOMETRY_EX pdg);
	int make_partitions_list();
	int make_partitions_list_winapi();
	int find_linux_partitions();
	partition_linux_ext2 **ext2_partitions_init();
public:
	unsigned char *read_sectors_data(DWORD start_sector, int n_sectors);
	disk(char *device = NULL);
	~disk();	
	bool init();	
	partition_list *get_partitions();
	partition_linux_ext2 **get_partitions_ext2();
	int get_partitions_ext2_count(){ return this->partitions_ext2_count; }
};


#endif