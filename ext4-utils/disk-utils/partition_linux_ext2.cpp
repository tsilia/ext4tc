#include "disk.h"
#include "partition_linux_ext2.h"
#include <fs/ext4/ext4.h>
#include <fs/ext4/blocks.h>

/**********************************************************
	PARTITION_LINUX_EXT2 class	
***********************************************************/

bool partition_linux_ext2::get_ext2_sb()
{
	unsigned char *buffer;

	if(this->ext2 == NULL)
	{
		this->ext2 = new ext4_sb_info;
		if(this->ext2 == NULL) return false;
		
		buffer = this->read_sectors(2, 2);
		if(buffer==NULL)
		{
			delete this->ext2;
			return false;
		}
		this->ext2->superblock_get(buffer);
		delete [] buffer;
		if(ext2->check_if_ext4_compatible())
		{
			this->ext2->initialize_from_sb();
			if (get_ext2_gr_descriptors() == true)
			{
				return true;
			}			
		}

		delete this->ext2;
		this->ext2 = NULL;
	}
	else if (this->ext2->check_if_ext4_compatible() && this->ext2->gr_desc != NULL)
	{
		return true;
	}	
	return false;
}

/**********************************************************
***********************************************************/

bool partition_linux_ext2::get_ext2_gr_descriptors()
{
	unsigned char *buffer;

	if(this->ext2 != NULL)
	{
		struct ext4_super_block *sb = &ext2->ext4_sb;
		int block_size_sect = EXT4_BLOCK_SIZE(sb) / this->sector_size;		
		int block_sect_to_read;		
		int ret = -1;

		block_sect_to_read = this->ext2->s_gdb_count * block_size_sect;
		buffer = this->read_sectors(block_size_sect, block_sect_to_read);
		if(buffer==NULL)
		{
			return false;
		}		
		ret = this->ext2->fill_group_descriptor_table(buffer);		
		delete [] buffer;
		if (ret < 0) 
		{
			return false;
		}
		return true;
	}
	return false;
}

/**********************************************************
***********************************************************/

ext4_inode *partition_linux_ext2::get_ext4_inode(unsigned int n_inode)
{
	unsigned int block_size_sect, inode_per, nr_group_block, inode_index = 0;
	unsigned long long inode_table_offset; // w sektorach
	unsigned long long inode_offset;
	ext4_inode *inode_ptr = NULL;

	if(ext2==NULL) return NULL;
	if(n_inode==0) return NULL;
	
	struct ext4_super_block *sb = &ext2->ext4_sb;

	block_size_sect = EXT4_BLOCK_SIZE(sb) / this->sector_size; //in sectors
	inode_per = sb->s_inodes_per_group;

	nr_group_block = (n_inode - 1) / inode_per;
	inode_index = (n_inode - 1) % inode_per;
	inode_offset = ((n_inode - 1) % inode_per) * sb->s_inode_size;

	if ((inode_ptr = this->ext2->get_inode_from_cache(n_inode)) != NULL)
	{
		return inode_ptr;
	}
	if(this->group_desc_queue.size() >= MAX_GROUP_DESC_QUEUE_LEN)
	{			
		int index = this->group_desc_queue.front();
		this->ext2->remove_inodes_grp_from_cache(index);
		this->group_desc_queue.pop();
	}
	
	inode_table_offset = 0;
	if (EXT4_HAS_INCOMPAT_FEATURE(sb, EXT4_FEATURE_INCOMPAT_64BIT))
	{
		inode_table_offset = (((unsigned long long)ext2->gr_desc[nr_group_block].bg_inode_table_hi) << 31) << 1;
	}
	
	inode_table_offset |= ext2->gr_desc[nr_group_block].bg_inode_table_lo;
	inode_table_offset *= block_size_sect;

	inode_ptr = (ext4_inode *)this->read_sectors_ex(inode_table_offset, 
		(sb->s_inode_size * inode_per + this->sector_size -1) / this->sector_size);	
	if(inode_ptr == NULL)
	{
		return NULL;
	}
	this->ext2->add_inodes_grp_to_cache(inode_ptr, nr_group_block);
	this->group_desc_queue.push(nr_group_block);
	return (ext4_inode *)&(((char *)inode_ptr)[inode_offset]);
}



/**********************************************************
***********************************************************/
/*
int partition_linux_ext2::get_file_inode_nr(char *file_name, unsigned int cur_inode_nr)
{
	ext2_inode *inode_cur;
	ext2_dir_entry_2 dir_entry;
	unsigned int  file_inode_nr=0;
	unsigned char *bufor, len;
	unsigned int offset, n_block;
	unsigned short block_size_sect=(2 << ext2->ext2_sb.s_log_block_size);
	unsigned int block_size_byte = block_size_sect * 512;
	
	inode_cur = get_ext2_inode(cur_inode_nr);
	if(inode_cur==NULL) return 0;

	len=(int)strlen(file_name);
	
	n_block = inode_cur->i_size / block_size_byte;
	for(unsigned int i=0; i<n_block; i++)
	{
		offset=0;
		bufor=read_sectors(block_size_sect * inode_cur->i_block[0], block_size_sect);
		if(bufor==NULL){return 0;}

		while(offset < block_size_byte )
		{
			get_ext2_dir_entry(&dir_entry, bufor+offset);
			if(len==dir_entry.name_len)
			{
				if(!strncmp(file_name, dir_entry.name, len))
				{
					file_inode_nr = dir_entry.inode;
					delete [] bufor;
					return file_inode_nr;
				}
			}
			offset += dir_entry.rec_len;
		}	
		delete [] bufor;
	}

	return 0;
}
*/

/**********************************************************
***********************************************************/

ext4_dir_entry_2 **partition_linux_ext2::get_dir_entry(unsigned int inode_nr, int *read_dirs)
{
	struct ext4_super_block *sb = &ext2->ext4_sb;
	unsigned int block_size_byte = EXT4_BLOCK_SIZE(sb);
	unsigned short block_size_sect = block_size_byte / this->sector_size;
	unsigned char *buffer;
	unsigned int n_block;
	__u64 nr_blk;
	unsigned int offset;
	ext4_dir_entry_2 *dir_entry = NULL;
	ext4_dir_entry_2 **tab_dir=NULL, **tmp_tab; 
	ext4_inode *inode;
	unsigned int n_dir=0;
	const unsigned int CHUNK_SIZE = 256;
	unsigned int nChunks = 0;
	unsigned long long inode_size = 0L;

	inode = get_ext4_inode(inode_nr);
	if (inode==NULL) return 0;
	blocks dir_blk(this, inode);

	inode_size = (unsigned long long)inode->i_size_high << 32;
	inode_size |= (unsigned long long)inode->i_size_lo;
	n_block = (unsigned int)((inode_size + block_size_byte - 1L) / (unsigned long long)block_size_byte);
		
	for (unsigned int i = 0; i < n_block; i++)
	{
		if (dir_blk.get_ext2_blk_nr(i, &nr_blk) < 0) 
		{
			if (n_dir > 0)
			{
				for(unsigned int i=0; i < n_dir-1; i++)
					delete tab_dir[i];
			}
			delete [] tab_dir;
			return NULL;
		}
		buffer = read_sectors_ex(block_size_sect * nr_blk, block_size_sect);
		if(buffer == NULL)
		{
			if (n_dir > 0)
			{
				for(unsigned int i=0; i < n_dir-1; i++)
					delete tab_dir[i];
			}
			delete [] tab_dir;
			return NULL;
		}
		//todo flush other dir
		offset = 0;				
		while(offset < block_size_byte)
		{
			dir_entry = new ext4_dir_entry_2;
			get_ext4_dir_entry(dir_entry, buffer + offset);

			n_dir++;			
			if(n_dir >= CHUNK_SIZE * nChunks)
			{				
				nChunks++;
				tmp_tab = tab_dir;
				tab_dir = new ext4_dir_entry_2 * [CHUNK_SIZE * nChunks];
				if(tab_dir==NULL)
				{
					delete dir_entry;
					delete [] buffer;
					*read_dirs=0;
					if (n_dir > 0)
					{
						for(unsigned int i=0; i < n_dir-1; i++)
							delete tmp_tab[i];
					}
					delete [] tmp_tab;
					return NULL;
				}
				//copying old entries
				memcpy(tab_dir, tmp_tab, sizeof(ext4_dir_entry_2 *)*(n_dir -1));
				delete [] tmp_tab;
			}

			tab_dir[n_dir-1] = dir_entry;						
			offset += dir_entry->rec_len;
		}
		delete [] buffer;
	}

	*read_dirs = n_dir;
	return tab_dir;
}


/**********************************************************
***********************************************************/
/*
ext2_dir_entry_2 **partition_linux_ext2::get_dir_entry(char* path, unsigned int current_inode_nr)
{
	struct ext2_inode *inode=NULL;
	ext2_dir_entry_2 **dir_tab;
	char **p_frag=NULL;

	if(path!=NULL)
	{
		p_frag=path_frag_get(path, '/');
		for(int i=0; p_frag[i]!=NULL && current_inode_nr != 0; i++)
		{
			current_inode_nr = get_file_inode_nr(p_frag[i], current_inode_nr);
		}
	}
	
	if(!current_inode_nr) return NULL;
	int n_dirs;
	dir_tab = get_dir_entry(current_inode_nr, &n_dirs);

	for(int i=0; p_frag[i]!=NULL; i++)
		delete [] p_frag[i];
	delete [] p_frag;

	return NULL;
}
*/
/**********************************************************
***********************************************************/

unsigned char *partition_linux_ext2::read_file(unsigned int inode_nr, HANDLE hnd, int *error)
{
	struct ext4_super_block *sb = &ext2->ext4_sb;
	unsigned int block_size_byte = EXT4_BLOCK_SIZE(sb);
	unsigned short block_size_sect = block_size_byte / this->sector_size;
	unsigned int n_blocks;
	unsigned int offset_max;
	__u64 nr_blk;
	ext4_inode *inode;
	unsigned char *buffer;
	const int index_ind_blk=12;
	unsigned long long inode_size = 0L;
	int n_write;
	DWORD n;
	
	*error = 0;
	if (hnd == INVALID_HANDLE_VALUE)
	{
		*error = -1;
		return NULL;
	}

	inode = get_ext4_inode(inode_nr);
	if(inode == NULL) {*error = -1; return NULL;}
	if(inode->i_size_lo == 0 && inode->i_size_high == 0){*error = -1; return NULL;}
	blocks file_blk(this, inode);
	
	inode_size = (unsigned long long)inode->i_size_high << 32;
	inode_size |= (unsigned long long)inode->i_size_lo;
	n_blocks = (unsigned int)((inode_size + block_size_byte - 1L) / (unsigned long long)block_size_byte);

	for(unsigned int i=0; i < n_blocks -1; i++)
	{
		if (file_blk.get_ext2_blk_nr(i, &nr_blk) < 0)
		{
			*error = -1; return NULL;
			return NULL;
		}
		buffer = read_sectors_ex(block_size_sect * nr_blk, block_size_sect);		
		if (buffer==NULL){*error = -1; return NULL;}
		n = 0;
		n_write = block_size_byte;		
		if (!WriteFile(hnd, buffer, n_write, &n, NULL)) 
		{
			delete [] buffer;
			*error = -1;
			return NULL;
		}
		delete [] buffer;
	}

	offset_max = (unsigned int)(inode_size - (unsigned long long)(n_blocks-1) * (unsigned long long)block_size_byte);
	if (offset_max>0)
	{
		if (file_blk.get_ext2_blk_nr(n_blocks-1, &nr_blk) < 0)
		{
			*error = -1;
			return NULL;
		}
		buffer = read_sectors_ex(block_size_sect * nr_blk, block_size_sect);
		if(buffer==NULL){*error = -1; return NULL;}
		n = 0;
		n_write = offset_max;
		if (!WriteFile(hnd, buffer + n, n_write, &n, NULL)) 
		{
			delete [] buffer;
			*error = -1;
			return NULL;
		}
		delete [] buffer;
	}
	return NULL;
}
