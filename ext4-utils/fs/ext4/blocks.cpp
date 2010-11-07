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

#include "blocks.h"
#include "ext4_extents.h"

/**********************************************************
***********************************************************/

unsigned int blocks::indirect_blk_nr(unsigned int ind_blk_nr)
{
	unsigned int nr_blk;

	if(buffer==NULL)
	{
		buffer = part->read_sectors_ex(block_size_sect * (u64)ind_blk_nr, block_size_sect);
		nr_blk = ((unsigned int *)buffer)[ind_index];		
		if((ind_index++) == n_block-1){delete [] buffer; buffer = NULL; ind_index=0;}
	}
	else
	{
		nr_blk = ((unsigned int *)buffer)[ind_index++];
		if(ind_index==addr_blk_per_block || ind_index==n_block)
		{	
			delete [] buffer;
			buffer = NULL;
			ind_index = 0;
		}
	}
	
	return nr_blk;	
}

unsigned int blocks::get_ext2_blk_nr(unsigned int blk_index, __u64 *physical_blk_addr)
{
	if (this->inode->i_flags & EXT4_EXTENTS_FL)
	{
		return this->get_ext2_extent_blk(blk_index, physical_blk_addr);
	}
	else
	{
		return this->get_ext2_non_extent_blk(blk_index, physical_blk_addr);
	}
}

/**
 *
 */
unsigned int blocks::get_ext2_extent_blk(unsigned int blk_index, __u64 *physical_blk_addr)
{
	ext4_extent_header *ext_i_hdr = ext_inode_hdr(this->inode);
	ext4_extent *ext = NULL;
	//TODO return error
	if (ext_i_hdr->eh_magic != EXT4_EXT_MAGIC)
	{
		return -1;
	}

	this->extent_path = this->ext4_ext_find_extent(this->inode, blk_index, this->extent_path);
	ext = this->extent_path[ext_i_hdr->eh_depth].p_ext;
	if (ext == NULL)
	{
		return -1;
	}
	
	*physical_blk_addr = this->extent_path[ext_i_hdr->eh_depth].p_block + blk_index - ext->ee_block;
	return 0;
}

unsigned int blocks::get_ext2_non_extent_blk(unsigned int blk_index, __u64 *physical_blk_addr)
{
//direct
	if(blk_index<EXT4_IND_BLOCK)
	{
		*physical_blk_addr = this->inode->i_block[blk_index];
		return 0;
	}
//indirect 
	if(blk_index<addr_blk_per_block + EXT4_IND_BLOCK)
	{
		*physical_blk_addr = indirect_blk_nr(this->inode->i_block[EXT4_IND_BLOCK]);
		return 0;
	}
//double indirect
	unsigned int ind_blk_nr, nr_blk;
	if(blk_index < dind_n_blk + addr_blk_per_block + EXT4_IND_BLOCK)
	{
		if(dindirect_buf==NULL)
		{
			dindirect_buf = part->read_sectors_ex(block_size_sect * (u64)this->inode->i_block[EXT4_DIND_BLOCK], block_size_sect);
			if(dindirect_buf==NULL) return -1;
            ind_blk_nr = ((unsigned int *)dindirect_buf)[0];

			nr_blk = indirect_blk_nr(ind_blk_nr);
			if(blk_index == n_block-1){delete [] dindirect_buf; dindirect_buf = NULL; dind_index=0;}
		}
		else
		{
			ind_blk_nr = ((unsigned int *)dindirect_buf)[dind_index];
			nr_blk = indirect_blk_nr(ind_blk_nr);

			if(ind_index == addr_blk_per_block -1){dind_index++;}
			if(blk_index==dind_n_blk + addr_blk_per_block + EXT4_IND_BLOCK - 1 || blk_index ==n_block -1)
			{delete [] dindirect_buf; dindirect_buf = NULL; dind_index=0;}
		}
		
		*physical_blk_addr = nr_blk;
		return 0;
	}
	return -1;
}


ext4_ext_path *blocks::ext4_ext_find_extent(ext4_inode *inode, __u64 block_no, ext4_ext_path *path)
{
	ext4_extent_header *eh = ext_inode_hdr(this->inode);
	__u16 depth = eh->eh_depth;
	int i, ppos = 0;

	if (path == NULL)
	{
		path = (struct ext4_ext_path *)(new unsigned char[sizeof(struct ext4_ext_path) * (depth + 2)]);
		if (path == NULL)
			return NULL;
		memset(path, 0, sizeof(struct ext4_ext_path) * (depth + 2));
	}

	path[0].p_hdr = eh;
	i = depth;
	while (i)
	{
		this->ext4_ext_binsearch_idx(inode, path + ppos, block_no);
		__u64 block_addr = (path[ppos].p_idx->ei_leaf_hi << 31) << 1;
		block_addr |= path[ppos].p_idx->ei_leaf_lo;
		if (block_addr != path[ppos].p_block)
		{
			path[ppos].p_block = block_addr;
			path[ppos].p_ext = NULL;
			path[ppos].p_depth = i;
			//needs to be free after stop work with path
			eh = (ext4_extent_header *) part->read_sectors_ex(block_size_sect * path[ppos].p_block, block_size_sect);
			//needs validate
			ppos++;		
			if (path[ppos].p_hdr != NULL)
			{
				delete [] ((unsigned char *)path[ppos].p_hdr);
			}
			path[ppos].p_hdr = eh;
		}
		else
		{
			ppos++;
		}

		i--;
	}

	path[ppos].p_depth = 0;
	path[ppos].p_ext = NULL;
	path[ppos].p_idx = NULL;

	this->ext4_ext_binsearch(inode, path + ppos, block_no);
	if (path[ppos].p_ext != NULL)
	{
		path[ppos].p_block = (path[ppos].p_ext->ee_start_hi << 31) << 1;
		path[ppos].p_block |= path[ppos].p_ext->ee_start_lo;
	}

	return path;
}


void blocks::ext4_ext_binsearch_idx(struct ext4_inode *inode,
			struct ext4_ext_path *path, __u64 block_no)
{
	struct ext4_extent_header *eh = path->p_hdr;
	struct ext4_extent_idx *r, *l, *m;

	l = EXT_FIRST_INDEX(eh) + 1;
	r = EXT_LAST_INDEX(eh);
	while (l <= r)
	{
		m = l + (r - l) / 2;
		if (block_no < m->ei_block)
			r = m - 1;
		else
			l = m + 1;
	}

	path->p_idx = l - 1;
}


void blocks::ext4_ext_binsearch(struct ext4_inode *inode,
		struct ext4_ext_path *path, __u64 block_no)
{
	struct ext4_extent_header *eh = path->p_hdr;
	struct ext4_extent *r, *l, *m;

	if (eh->eh_entries == 0) {
		return;
	}
	l = EXT_FIRST_EXTENT(eh) + 1;
	r = EXT_LAST_EXTENT(eh);

	while (l <= r) {
		m = l + (r - l) / 2;
		if (block_no < m->ee_block)
			r = m - 1;
		else
			l = m + 1;
	}
	path->p_ext = l - 1;
}