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

#ifndef __EXT4_SB_INFO_H__
#define __EXT4_SB_INFO_H__

#include <windows.h>
#include <types.h>
#include "ext4.h"

/*
 * fourth extended-fs super-block data in memory
 */

class ext4_sb_info
{
public:
	unsigned long s_desc_size;	/* Size of a group descriptor in bytes */
	unsigned long s_inodes_per_block;/* Number of inodes per block */
	unsigned long s_blocks_per_group;/* Number of blocks in a group */
	unsigned long s_inodes_per_group;/* Number of inodes in a group */
	unsigned long s_itb_per_group;	/* Number of inode table blocks per group */
	unsigned long s_gdb_count;	/* Number of group descriptor blocks */
	unsigned long s_desc_per_block;	/* Number of group descriptors per block */
	unsigned long s_block_count;
	struct ext4_super_block *sb;
	ext4_inode **inodes_table_cache;
public:
	struct ext4_super_block ext4_sb;
	struct ext4_group_desc *gr_desc;	
	
public:
	ext4_sb_info() { gr_desc=NULL; inodes_table_cache = NULL; }
	~ext4_sb_info()
	{
		int n_groups = this->ext4_sb.s_inodes_count / this->ext4_sb.s_inodes_per_group;
		delete [] gr_desc; 
		for(int i=0;i<n_groups;i++)
		{
			delete [] inodes_table_cache[i];
		}
		delete [] inodes_table_cache;
	}

	void superblock_get(unsigned char *buffer);
	int fill_group_descriptor_table(unsigned char *buffer);
	bool check_if_ext4_compatible(void);
	void initialize_from_sb(void);
	ext4_inode *get_inode_from_cache(unsigned int inode_no);

	void add_inodes_grp_to_cache(ext4_inode *inodes, unsigned int group_block_no)
	{ this->inodes_table_cache[group_block_no] = inodes; }

	void remove_inodes_grp_from_cache(unsigned int group_block_no) 
	{ 
		delete [] ((unsigned char *) this->inodes_table_cache[group_block_no]); 
		this->inodes_table_cache[group_block_no] = NULL;
	}

	void group_descriptor_get(unsigned char *buffer, struct ext4_group_desc *gr_desc)
	{ memcpy(gr_desc, buffer, this->s_desc_size); }
};



#endif