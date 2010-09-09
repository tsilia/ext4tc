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

#ifndef __PARTITION_LINUX_EXT2_H__
#define __PARTITION_LINUX_EXT2_H__

#include "../fs/ext4/ext4.h"
#include "../fs/ext4/ext4_sb_info.h"
#include "partition.h"
#include <queue>

#define MAX_GROUP_DESC_QUEUE_LEN 100

class partition_linux_ext2 : public partition
{
	std::queue <int> group_desc_queue;
public:
	ext4_sb_info *ext2;
public:
	partition_linux_ext2() : partition(){ this->ext2 = NULL; }
	~partition_linux_ext2(){ delete this->ext2; }

	bool get_ext2_sb();
	bool get_ext2_gr_descriptors();
	ext4_inode *get_ext4_inode(unsigned int n_inode);
	ext4_dir_entry_2 **get_dir_entry(unsigned int inode_nr, int *read_dirs);
	unsigned char *read_file(unsigned int inode_nr, HANDLE hnd, int *error);

	inline void get_ext4_dir_entry(struct ext4_dir_entry_2 *dir_entry, unsigned char *buffer)
	{ 
		struct ext4_dir_entry_2 *src = (struct ext4_dir_entry_2 *)buffer;
		memcpy(dir_entry, src, (sizeof(struct ext4_dir_entry_2) < src->rec_len) ? 
			sizeof(struct ext4_dir_entry_2): src->rec_len); 		
		src->rec_len = src->rec_len;
	}
	//int get_file_inode_nr(char *file_name, unsigned int cur_inode_nr);
	/*inline unsigned int indirect_blk_nr(unsigned int ind_blk_nr, ext2_inode *inode, unsigned int blk_index);
	unsigned int get_ext2_blk_nr(ext2_inode *inode, unsigned int blk_index);
	ext2_dir_entry_2 **get_dir_entry(char *path, unsigned int current_inode_nr);*/
};

#endif
