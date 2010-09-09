#ifndef __EXT2_PLUGIN_DESCR__
#define __EXT2_PLUGIN_DESCR__

#include "windows/debugWindowCls.h"
#include "ext4plugin.h"

#include <disk-utils/disk.h>
#include <disk-utils/partition_list.h>
#include <fs/ext4/blocks.h>
#include <fs/ext4/ext4.h>
#include <fs/ext4/ext4_extents.h>

#define PART_PREFIX "sda"
#define DEVICE_HDD_DISPLAY_PREFIX "sd"
#define DEVICE_HDD_PREFIX "\\\\.\\PhysicalDrive"

#define RETURN_FILE(FindDATA, DIR_ENTRY)\
do \
{\
	strncpy((FindDATA)->cFileName, (DIR_ENTRY)->name, (DIR_ENTRY)->name_len); \
	(FindDATA)->cFileName[(DIR_ENTRY)->name_len] = '\0'; \
	if (EXT4_FT_DIR == (DIR_ENTRY)->file_type) \
	{\
		(FindDATA)->dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY; \
	}\
	else \
	{\
		(FindDATA)->dwFileAttributes=FILE_ATTRIBUTE_NORMAL; \
	}\
	(FindDATA)->ftLastWriteTime.dwHighDateTime=0xFFFFFFFF; \
	(FindDATA)->ftLastWriteTime.dwLowDateTime=0xFFFFFFFE; \
} while(0)

class ext4PluginDescr
{
public:
	int PluginNumber;
	tProgressProc ProgressProc;
	tLogProc LogProc;
	tRequestProc RequestProc;	

	//linux
	disk **hard_disks;
	int hard_disks_cnt;
	int *ext4_partitions_id_map;
	/*disk *hd;
	partition_linux_ext2 **ext2_part;*/
	//debug
#ifdef _DEBUG
	debugWindowCls dWin;
#endif
private:
	ext4_dir_entry_2 **dir_entries;	
	char *current_path;
	int current_partition;
	int dir_entry_num;
	bool plugin_initialized;
private:
	void init_ext2_partitions_id_map(int partitions_id_map_elem);
	int load_ext2_dir_entry(int disk_no, int part_no_map, int inode_num);
public:
#ifdef _DEBUG
	ext4PluginDescr() : dWin(0, SW_NORMAL, "Debug", "Debug")
#else
	ext4PluginDescr()
#endif
	{
		this->hard_disks = NULL;
		this->hard_disks_cnt = 0;
		this->ext4_partitions_id_map = NULL;
		this->dir_entries = NULL;
		this->dir_entry_num = 0;
		this->current_path = NULL;
		this->current_partition = -1;	
		this->plugin_initialized = false;
	}

	~ext4PluginDescr();

	int get_hdd_count();
	int search_system_for_linux_ext2_partitions();	
	int extract_disk_and_part_no(char *path, int *disk_no, int *part_no);
	bool validate_disk_and_part_no(int disk_no, int part_no);
	int get_first_ext4_disk_and_part_no(int *disk_no, int *part_no);
	int get_next_ext4_disk_and_part_no(int *disk_no, int *part_no_map);
	int get_partition_index_via_real_number(int disk_no, int part_no)
	{return this->ext4_partitions_id_map[this->ext4_partitions_id_map[disk_no] + part_no - 1];}
	int get_current_partition(){ return this->current_partition; }
	void set_current_partition(int n){ this->current_partition = n; }
	char *get_current_path() {return this->current_path;}	
	bool get_plugin_initialized() { return this->plugin_initialized; }
	void set_plugin_initialized(bool val) { this->plugin_initialized = val; }	
	/**
	 * set_current_path
	 */
	void set_current_path(char *path) 
	{
		delete [] this->current_path;
		this->current_path = new char[strlen(path) + 1];
		strncpy(this->current_path, path, strlen(path));
	}

	/**
	 * clear_current_path
	 */
	void clear_current_path(){delete [] this->current_path; this->current_path = NULL;}

	/**
	 * get_first_dir_entry
	 */
	ext4_dir_entry_2 *get_first_dir_entry(char *path, int disk_no, int part_no_map, int *num_entry);

	/**
	 * get_first_dir_entry2
	 */
	ext4_dir_entry_2 *get_first_dir_entry2(char *path, int disk_no, int part_no_map, int *num_entry);

	/**
	 * get_next_dir_entry
	 */ 

	ext4_dir_entry_2 *get_next_dir_entry(int *num_last_entry)	
	{
		ext4_dir_entry_2 *dir = NULL;
		int local_last_entry = *num_last_entry;

		if(this->dir_entries != NULL)
		{			
			if (local_last_entry >= this->dir_entry_num)
				return NULL;

			for(int i = local_last_entry + 1; i < this->dir_entry_num; i++)
			{
				dir = this->dir_entries[i];
				if (dir == NULL || dir->inode == 0 || !strncmp(".", dir->name, dir->name_len)
					|| !strncmp("..", dir->name, dir->name_len))
				{
					continue;
				}								
				*num_last_entry = i;
				return dir;
			}
		}
		return NULL;		
	}

	/**
	 * get_ext2_inode
	 */ 

	ext4_inode *get_ext2_inode(int disk_no, int part_no, char *path, int *inode_num)
	{
		ext4_dir_entry_2 *dir = NULL;
		char *ptr = NULL;
		size_t len = 0;
		char *localPath = NULL;

		ptr = strrchr(path, '\\');
		ptr++;
		len = strlen(ptr);

		for(int i = 0; i < this->dir_entry_num; i++)
		{
			dir = this->dir_entries[i];
			if (len == dir->name_len && !strncmp(ptr, dir->name, len))
			{
#ifdef _DEBUG2
				this->dWin.appendText("get_inode: %d\n", dir->inode);
#endif
				*inode_num = dir->inode;
				partition_linux_ext2 **ext2_partitions = this->hard_disks[disk_no]->get_partitions_ext2();
				return ext2_partitions[part_no]->get_ext4_inode(dir->inode);
			}
		}

		return NULL;
	}

	/**
	 * get_ext2_inode
	 */ 

	ext4_inode *get_ext2_inode(int disk_no, int part_no, int inode_num)
	{
		partition_linux_ext2 **ext2_partitions = this->hard_disks[disk_no]->get_partitions_ext2();
		return ext2_partitions[part_no]->get_ext4_inode(inode_num);
	}
};

#endif