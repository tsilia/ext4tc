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

#include "ext4PluginDescr.h"

ext4PluginDescr::~ext4PluginDescr()
{
	delete [] this->hard_disks;
	delete [] this->ext4_partitions_id_map;
}

int ext4PluginDescr::get_hdd_count()
{
	HANDLE hnd = INVALID_HANDLE_VALUE;
	int n = 0;
	char buffer[30];
	do
	{
		sprintf(buffer, "%s%d", DEVICE_HDD_PREFIX, n);
		hnd = CreateFile(buffer,
                    GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
                    NULL, OPEN_EXISTING, 0, NULL);


		if(hnd == INVALID_HANDLE_VALUE) 
			break;
		CloseHandle(hnd);
		n++;
	} while(1);
	return n;
}

int ext4PluginDescr::search_system_for_linux_ext2_partitions()
{
	int p = 0;
	if ((this->hard_disks_cnt = this->get_hdd_count()) <= 0)
	{
		return this->hard_disks_cnt;
	}

#ifdef _DEBUG
	this->dWin.appendText("found %d hard disk\n", this->hard_disks_cnt);
#endif

	this->hard_disks = new disk *[this->hard_disks_cnt];
	for (int i = 0; i < this->hard_disks_cnt; i++)
	{
		char device[30];
		sprintf(device, "%s%d", DEVICE_HDD_PREFIX, i);
		disk *hd = new disk(device);
		hd->init();
		int n = hd->get_partitions_ext2_count();
#ifdef _DEBUG
		dWin.appendText("on [disk %d] found %d ext2 compatible partitions\n", i, n);
#endif

		if (n > 0)
		{
			partition_linux_ext2 **linux_part = hd->get_partitions_ext2();
			p += linux_part[n-1]->get_part_no(); //getting max id of ext2 compatible partitions
#ifdef _DEBUG
			for (int j = 0; j < n; j++)
			{
				ext4_super_block *sb = &linux_part[j]->ext2->ext4_sb;
				dWin.appendText("(%d) type: %d offset:%lld\n", linux_part[j]->get_part_no(), 
					linux_part[j]->get_type(), linux_part[j]->get_offset());
				
				dWin.appendText("(%d) features EXT4_FEATURE_INCOMPAT_64BIT: %x \n", linux_part[j]->get_part_no(), 
					!!(sb->s_feature_incompat & EXT4_FEATURE_INCOMPAT_64BIT));
			}
#endif
		}
		this->hard_disks[i] = hd;
	}

	this->init_ext2_partitions_id_map(this->hard_disks_cnt + p);
	return p;
}

/**
 * ext2_partitions_id_map - first this->hard_disks_cnt indexes contain indexes to positions in which 
 *		there are partitions real id to index mapping.
 */

void ext4PluginDescr::init_ext2_partitions_id_map(int partitions_id_map_elem)
{
	delete [] this->ext4_partitions_id_map;
	this->ext4_partitions_id_map = new int[partitions_id_map_elem];
	memset(this->ext4_partitions_id_map, 0, sizeof(int) * partitions_id_map_elem);

	
	int pos = this->hard_disks_cnt; //first partitions map is in index which is equal to hard disk count.
	for (int i = 0; i < this->hard_disks_cnt; i++)
	{
		disk *hd = this->hard_disks[i];
		
		int n = hd->get_partitions_ext2_count();
		if (n > 0)
		{
			this->ext4_partitions_id_map[i] = pos;
			partition_linux_ext2 **ext2_partitions = hd->get_partitions_ext2();
			for (int j = 0; j < n; j++)
			{
				//real partition index in the system combined with position
				int index = pos + ext2_partitions[j]->get_part_no() - 1; 
				this->ext4_partitions_id_map[index] = j;
			}
			pos += ext2_partitions[n - 1]->get_part_no(); //position is increased about maximal ext2 partition index
		}
		else //no ext2 compatible partitions
		{
			this->ext4_partitions_id_map[i] = 0;
		}
	}
}

int ext4PluginDescr::extract_disk_and_part_no(char *path, int *disk_no, int *part_no)
{
	char *DEVICE_PREFIX = "sd";
	char *expr = "\\sd";
	char *p_end = NULL;
	char *p = strstr(path, expr);
	int len = 0;
	bool was_alpha = false, was_digit = false;
	int l_disk_no = 0, l_part_no = 0;

	if (p == NULL)
	{
		return -1;
	}
	p += strlen(expr);
#ifdef _DEBUG
	dWin.appendText("%s(): path %s\n", __FUNCTION__, p);
#endif

	//substracting only disk and partition id from path
	if ((p_end = strchr(p, '\\')) == NULL)
	{
		len = strlen(p);
	}
	else
	{
		len = p_end - p; //length without '\'
	}

	//minimal disk and partition id len is 2
	if (len < 2)
	{
		return -1;
	}

	for (int i = 0; i < len; i++)
	{
		if (isalpha(p[i]))
		{
			if (was_digit)
			{
				return -1;
			}
			was_alpha = true;
			l_disk_no = l_disk_no * ('z'-'a' + 1) + p[i] - 'a' + 1;
		}
		else if (isdigit(p[i]))
		{
			if (!was_alpha)
			{
				return -1;
			}
			was_digit = true;
			l_part_no = l_part_no * 10 + p[i] - '0';
		}
		else
		{
			return -1;
		}
	}

	if (!was_digit || !was_alpha)
	{
		return -1;
	}
	l_disk_no--;
	if (this->validate_disk_and_part_no(l_disk_no, l_part_no) == false)
	{
		return -1;
	}
	*disk_no = l_disk_no;
	*part_no = l_part_no;
	return p + len - path;
}

bool ext4PluginDescr::validate_disk_and_part_no(int disk_no, int part_no)
{
	if (disk_no >=0 && disk_no < this->hard_disks_cnt && part_no >= 0)
	{
		int n = this->hard_disks[disk_no]->get_partitions_ext2_count();
		if (n > 0)
		{
			partition_linux_ext2 **ext4_partiotions = this->hard_disks[disk_no]->get_partitions_ext2();
			if (ext4_partiotions != NULL && part_no <= (int)ext4_partiotions[n-1]->get_part_no())
			{
				return true;
			}
		}
	}
	return false;
}

int ext4PluginDescr::get_first_ext4_disk_and_part_no(int *disk_no, int *part_no)
{
	int n = 0;

	for (int i = 0; i < this->hard_disks_cnt; i++)
	{
		if((n = this->hard_disks[i]->get_partitions_ext2_count()) > 0)
		{
			*disk_no = i;
			*part_no = this->hard_disks[i]->get_partitions_ext2()[0]->get_part_no();
			return 0;
		}
	}
	return -1;
}


int ext4PluginDescr::get_next_ext4_disk_and_part_no(int *disk_no, int *part_no_map)
{
	int n = 0;
	int j = *part_no_map + 1;
	for (int i = *disk_no; i < this->hard_disks_cnt; i++)
	{
		if((n = this->hard_disks[i]->get_partitions_ext2_count()) > 0)
		{
			if (j < n)
			{
				*disk_no = i;
				*part_no_map = j;
				return 0;
			}
			j = 0;
		}
	}
	return -1;
}

/**
 * load_ext2_dir_entry
 */

int ext4PluginDescr::load_ext2_dir_entry(int disk_no, int part_no_map, int inode_num)
{				
	if (this->dir_entries)
	{
		for(int i=0; i < this->dir_entry_num; i++)
		{
			delete this->dir_entries[i];
		}
		delete [] this->dir_entries;
	}
	//this->dir_entries = this->ext2_part[part_num]->get_dir_entry(inode_num, &this->dir_entry_num);
	partition_linux_ext2 **ext2_partitions = this->hard_disks[disk_no]->get_partitions_ext2();
	this->dir_entries = ext2_partitions[part_no_map]->get_dir_entry(inode_num, &this->dir_entry_num);
	return (this->dir_entries != NULL)? 0 : -1;
}

/**
 * get_first_dir_entry
 */

ext4_dir_entry_2 *ext4PluginDescr::get_first_dir_entry(char *path, int disk_no, int part_no_map, int *num_entry)
{
		int inode_num = 2, i = 0;
		ext4_dir_entry_2 *dir = NULL;
		char *word = NULL, *lasts = NULL, sep[] = "\\";
		size_t len = 0;
		
		if (this->load_ext2_dir_entry(disk_no, part_no_map, EXT4_ROOT_INO) == -1)
		{
			return NULL;
		}
		this->set_current_path(path);

		word = strchr(path + 1, '\\');			
		if (word != NULL)
		{
			for (word = strtok_s(word + 1, sep, &lasts);
				word;
				word = strtok_s(NULL, sep, &lasts))
			{		
#ifdef _DEBUG2
				this->dWin.appendText("path divided into tokens: %s\n", word);
				this->dWin.appendText("dir_entry_num:%d\n", this->dir_entry_num);
#endif
				len = strlen(word);
				for(i = 0; i < this->dir_entry_num; i++)
				{
					dir = this->dir_entries[i];						
					if (dir != NULL && len == dir->name_len && !strncmp(word, dir->name, len)) //found dirname
					{
						dir->name[dir->name_len]='\0';
						inode_num = dir->inode;
#ifdef _DEBUG2
						this->dWin.appendText("found: %s =? %s\n", word, dir->name);
						this->dWin.appendText("path divided into tokens: %s\n", word);
#endif
						if ( this->load_ext2_dir_entry(disk_no, part_no_map, inode_num) == -1)
						{
							return NULL;
						}
						break;
					}
				}

			}
		}			
		dir = NULL;
#ifdef _DEBUG2
		this->dWin.appendText("dir_entry_num: %d\n", this->dir_entry_num);
#endif
		for(i = 0; i < this->dir_entry_num; i++)
		{				
			dir = this->dir_entries[i];
			if (dir == NULL || dir->inode == 0 || !strncmp(".", dir->name, dir->name_len)
				|| !strncmp("..", dir->name, dir->name_len))
			{
				dir = NULL;
				continue;
			}										
			break;
		}
		*num_entry = i;
		return dir;
}


/**
 * get_first_dir_entry2
 */

ext4_dir_entry_2 *ext4PluginDescr::get_first_dir_entry2(char *path, int disk_no, int part_no_map, int *num_entry)
{
		ext4_dir_entry_2 *dir = NULL;
		char *ptr = NULL;
		size_t len = 0;
		int j = 0;
		//if goes up
		if (strlen(this->get_current_path()) > strlen(path))
		{
			ptr = "..";
			len = 2;
		}
		else
		{
			ptr = strrchr(path, '\\');
			ptr++;
			len = strlen(ptr);
		}
#ifdef _DEBUG2
		this->dWin.appendText("dirname -  %s\n", ptr);
#endif
		for(int i = 0; i < this->dir_entry_num; i++)
		{
			dir = this->dir_entries[i];
			if (len == dir->name_len && !strncmp(ptr, dir->name, len)) //found dirname
			{
				if (this->load_ext2_dir_entry(disk_no, part_no_map, dir->inode) == -1)
				{
					return NULL;
				}
				dir = NULL;

				for(j = 0; j < this->dir_entry_num; j++)
				{
					dir = this->dir_entries[j];
					if (dir == NULL || dir->inode == 0 || !strncmp(".", dir->name, dir->name_len) 
						|| !strncmp("..", dir->name, dir->name_len))
					{
						dir = NULL;
						continue;						
					}
					break;
				}
				this->set_current_path(path);
				break;
			}

		}
		*num_entry = j;
		return dir;
}
