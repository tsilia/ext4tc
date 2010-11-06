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

#include "ext4_sb_info.h"

#include <string.h>
#include <time.h>
#include <magic.h>

void ext4_sb_info::superblock_get(unsigned char *buffer)
{
	memcpy(&(this->ext4_sb), buffer, sizeof(struct ext4_super_block));
}

void ext4_sb_info::initialize_from_sb(void)
{
		struct ext4_super_block *sb = &this->ext4_sb;
		
		//int block_size_sect = EXT4_BLOCK_SIZE(sb) * this->sector_size;
		if (EXT4_HAS_INCOMPAT_FEATURE(sb, EXT4_FEATURE_INCOMPAT_64BIT))
		{
			this->s_desc_size = sb->s_desc_size;
		}
		else
		{
			this->s_desc_size = EXT4_MIN_DESC_SIZE;
		}
		this->s_desc_per_block = EXT4_BLOCK_SIZE(sb) / this->s_desc_size; //EXT4_DESC_PER_BLOCK(sb);
/*TODO EXT4_FEATURE_COMPAT_64BIT*/
		this->s_group_block_count = ((sb->s_blocks_count_lo - sb->s_first_data_block - 1)
 					/ EXT4_BLOCKS_PER_GROUP(sb)) + 1;
		this->s_gdb_count = (this->s_group_block_count + this->s_desc_per_block - 1) / this->s_desc_per_block;
		this->s_inodes_per_block = EXT4_INODES_PER_GROUP(sb);
}

int ext4_sb_info::fill_group_descriptor_table(unsigned char *buffer)
{
		int offset=0;
		this->gr_desc = new ext4_group_desc [this->s_group_block_count];
		if(this->gr_desc == NULL)
			return -1;

		this->inodes_table_cache = new ext4_inode *[this->s_group_block_count];
		if(this->inodes_table_cache == NULL) 
		{
			delete [] this->gr_desc;
			return -1;
		}
		memset(this->inodes_table_cache, 0, this->s_group_block_count * sizeof(ext4_inode *));

		for(unsigned int i=0; i < this->s_group_block_count; i++)
		{
			this->group_descriptor_get(buffer + offset, &(this->gr_desc[i]));
			offset += this->s_desc_size;
		}
		return 0;
}

ext4_inode *ext4_sb_info::get_inode_from_cache(unsigned int inode_no)
{
		unsigned int group_no = (inode_no - 1) / this->s_inodes_per_block;
		unsigned int inode_index = (inode_no - 1) % this->s_inodes_per_block;
		unsigned int inode_offset = ((inode_no - 1) % this->s_inodes_per_block) * ext4_sb.s_inode_size;
		
		if(this->inodes_table_cache[group_no]!=NULL)
		{
			return (ext4_inode *)&((char *)this->inodes_table_cache[group_no])[inode_offset];
		}
		return NULL;
}

bool ext4_sb_info::check_if_ext4_compatible(void)
{
	return this->ext4_sb.s_magic == EXT4_SUPER_MAGIC;
}


void get_ext4_dir_entry(struct ext4_dir_entry_2 *dir_entry, unsigned char *buffer)
{
	memcpy(dir_entry, buffer, sizeof(struct ext4_dir_entry_2));
}
