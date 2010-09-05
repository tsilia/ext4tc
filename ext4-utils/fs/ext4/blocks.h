#ifndef BLOCKS_H
#define BLOCKS_H

#include "ext4.h"
#include "ext4_extents.h"
#include <disk-utils/partition_linux_ext2.h>

class blocks
{
private:
	partition_linux_ext2 *part;
	ext4_inode *inode;
	ext4_ext_path *extent_path;
	unsigned short block_size_sect;
	unsigned int block_size_byte;
	unsigned int addr_blk_per_block;
	unsigned int dind_n_blk;
	unsigned int n_block;
	unsigned char *buffer;
	unsigned char *dindirect_buf;
	unsigned int ind_index;
	unsigned int dind_index;
public:
	blocks(partition_linux_ext2 *p, ext4_inode *in)
	{
		this->part = p;
		struct ext4_super_block *sb = &this->part->ext2->ext4_sb;
		this->inode = in;
		this->block_size_sect = EXT4_BLOCK_SIZE(sb) >> 9;
		this->block_size_byte = EXT4_BLOCK_SIZE(sb);
		this->addr_blk_per_block = EXT4_ADDR_PER_BLOCK(sb);
		this->dind_n_blk = addr_blk_per_block * addr_blk_per_block;
		n_block = (inode->i_size_lo + this->block_size_byte - 1) / this->block_size_byte;
		//bufory
		this->buffer=NULL;
		ind_index = 0;
		this->dindirect_buf = NULL;
		this->extent_path = NULL;
		dind_index = 0;
	}
	~blocks(){flush();}
	unsigned int indirect_blk_nr(unsigned int ind_blk_nr);
	unsigned int get_ext2_blk_nr(unsigned int blk_index, __u64 *physical_blk_addr);	
	int flush()
	{
		delete [] buffer; 
		buffer=NULL; 
		ind_index = 0;
		delete [] dindirect_buf; 
		dindirect_buf = NULL;
		dind_index = 0;
		if (this->inode->i_flags & EXT4_EXTENTS_FL)
		{
			ext4_extent_header *eh = ext_inode_hdr(this->inode);
			__u16 depth = eh->eh_depth;
			for (int i = 1; i < depth + 2; i++)
			{
				delete [] ((unsigned char *)this->extent_path[i].p_hdr);
			}
			delete [] ((unsigned char *)this->extent_path);
		}
		return 0;
	}
private:
	unsigned int get_ext2_non_extent_blk(unsigned int blk_index, __u64 *physical_blk_addr);
	unsigned int get_ext2_extent_blk(unsigned int blk_index, __u64 *physical_blk_addr);
	struct ext4_ext_path *ext4_ext_find_extent(struct ext4_inode *, __u64 , struct ext4_ext_path *);
	void ext4_ext_binsearch_idx(struct ext4_inode *inode, struct ext4_ext_path *path, __u64 block_no);
	void ext4_ext_binsearch(struct ext4_inode *inode, struct ext4_ext_path *path, __u64 block_no);
};

#endif