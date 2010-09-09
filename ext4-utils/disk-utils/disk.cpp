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

#include <windows.h>
#include "disk.h"

/**********************************************************
	DISK class	
***********************************************************/

/**************************************
 set_handler()
 - private
***************************************/


BOOL disk::set_handler(char *device)
{
	this->disk_handler = CreateFile(device,
                    GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
                    NULL, OPEN_EXISTING, 0, NULL);


	if(this->disk_handler == INVALID_HANDLE_VALUE) return FALSE;

	return TRUE;
}


/***********
	public
***********/

disk::disk(char *device)
{
	if(device!=NULL)
	{
		DISK_GEOMETRY_EX dg;
		set_handler(device);
		if (this->get_disk_geometry(&dg) == -1)
		{
			this->sector_size = SECTOR_SIZE;
		}
		else
		{
			this->sector_size = (dg.Geometry.BytesPerSector < SECTOR_SIZE) ? SECTOR_SIZE : dg.Geometry.BytesPerSector;
		}
	}
	else
		this->disk_handler = INVALID_HANDLE_VALUE;

	this->disk_pos = 0;
	this->partitions = NULL;
	this->partitions_ext2 = NULL;
	this->partitions_ext2_count = 0;
}


disk::~disk()
{	
	if(this->disk_handler != INVALID_HANDLE_VALUE) CloseHandle(this->disk_handler);
	delete this->partitions;
	delete [] this->partitions_ext2;
}

int disk::get_disk_geometry(PDISK_GEOMETRY_EX pdg)
{
	DWORD n;	
	BOOL bResult;

	bResult = DeviceIoControl(this->disk_handler, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, 
					pdg, sizeof(*pdg), &n, NULL);
	if (bResult == FALSE)
		return -1;
	return 0;
}


/**************************************
 read_sectors_data
 - public
***************************************/

unsigned char *disk::read_sectors_data(DWORD start_sector, int n_sectors)
{
	DWORD bytes_recv;
	unsigned char *buffer=NULL;
	LARGE_INTEGER l_offset;

	
	if(this->disk_handler != INVALID_HANDLE_VALUE)
	{	
		l_offset.QuadPart =0;
		l_offset.QuadPart = start_sector;
		l_offset.QuadPart *= this->sector_size;
		
		if(SetFilePointerEx(this->disk_handler, l_offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
			return NULL;
		

		buffer=new unsigned char[n_sectors * this->sector_size];

		if(!ReadFile(this->disk_handler, buffer, n_sectors * this->sector_size, &bytes_recv, NULL))
		{
			delete [] buffer;
			return NULL;
		}

	}

	return buffer;
}

bool disk::init()
{
	if (this->make_partitions_list() < 0)
		return false;
	this->partitions_ext2 = this->ext2_partitions_init();

	return true;
}

partition_list *disk::get_partitions()
{
	return this->partitions;
}

partition_linux_ext2 **disk::get_partitions_ext2()
{
	return this->partitions_ext2;
}
/**************************************
 make_part_list
 - public
***************************************/

int disk::make_partitions_list()
{
	int size=0;
	unsigned char *sector;
	partition part_tmp;
	BOOL is_extended = FALSE;
	unsigned char p_type=0;
	unsigned int offset_ext_part;
	const unsigned short magic_nr = 0xaa55;
	int n = 1;

	sector = this->read_sectors_data(0, 1);
	if(sector == NULL) 
		return -1;
	if((*(unsigned short *)(sector + 510) != magic_nr))
	{
		delete [] sector;
		return -1;
	}

	for(int i=0; i < 4; i++, n++)
	{
		part_tmp.set_part_info(sector + 446 + i*16);
		
		p_type = part_tmp.get_type();
		if(p_type)
		{
			if(this->partitions==NULL)
				this->partitions = new partition_list;

			//part_tmp.set_offset(part_tmp.get_LBA());
			part_tmp.set_offset((unsigned long long)part_tmp.get_LBA() * this->sector_size);
			part_tmp.set_part_handler(this->disk_handler);
			part_tmp.set_sector_size(this->sector_size);
			part_tmp.set_part_no(n);
			(this->partitions)->add_part(&part_tmp);

			if(p_type==0x5 || p_type==0xf)
			{
				is_extended = TRUE;
				offset_ext_part = part_tmp.get_LBA();
			}
		}
	}
	delete [] sector;
	
	if(is_extended)
	{
		unsigned int offset_logic_ext = 0;
			
		do
		{
			sector = this->read_sectors_data(offset_ext_part + offset_logic_ext, 1);
			if(sector == NULL) 
				return -1;
			if((*(unsigned short *)(sector + 510) != magic_nr))
			{
				delete this->partitions;
				this->partitions = NULL;
				delete []sector;
				return -1;
			}

			part_tmp.set_part_info(sector + 446);
			part_tmp.set_logic();
			//part_tmp.set_offset(offset_ext_part + offset_logic_ext + part_tmp.get_LBA());
			part_tmp.set_offset((unsigned long long)(offset_ext_part + offset_logic_ext + part_tmp.get_LBA()) * this->sector_size);
			part_tmp.set_part_handler(this->disk_handler);
			part_tmp.set_sector_size(this->sector_size);
			part_tmp.set_part_no(n);
			(this->partitions)->add_part(&part_tmp);			
			n++;
			if(*(sector + 466) == 0x5)
			{
				part_tmp.set_part_info(sector + 462);
				part_tmp.set_logic();
				//part_tmp.set_offset(offset_ext_part + offset_logic_ext + part_tmp.get_LBA());
				part_tmp.set_offset((unsigned long long)(offset_ext_part + offset_logic_ext + part_tmp.get_LBA()) * this->sector_size);
				part_tmp.set_part_no(0);
				part_tmp.set_sector_size(this->sector_size);
				(this->partitions)->add_part(&part_tmp);
				offset_logic_ext = part_tmp.get_LBA();
			}
			else is_extended = FALSE;
			delete [] sector;

		} while(is_extended);
	}

	return 0;
}

int disk::make_partitions_list_winapi()
{
	unsigned char buffer[sizeof(DRIVE_LAYOUT_INFORMATION) + sizeof(PARTITION_INFORMATION) * 100];
	int len = sizeof(buffer);
	DWORD n;
	DRIVE_LAYOUT_INFORMATION *pdg;
	BOOL bResult;
	partition part;

	bResult = DeviceIoControl(this->disk_handler, IOCTL_DISK_GET_DRIVE_LAYOUT, NULL, 0, buffer, len,     
                         &n, NULL);
	if (bResult) 
	{
		delete this->partitions;
		this->partitions = new partition_list;

		//process partition list
		pdg = (DRIVE_LAYOUT_INFORMATION *)buffer;
		for (unsigned int i = 0; i < pdg->PartitionCount; i++)
		{
			if (pdg->PartitionEntry[i].PartitionNumber == 0 || pdg->PartitionEntry[i].PartitionType == PARTITION_ENTRY_UNUSED)
				continue;
			part.set_offset(pdg->PartitionEntry[i].StartingOffset.QuadPart);
			part.set_part_no(pdg->PartitionEntry[i].PartitionNumber);
			part.set_type(pdg->PartitionEntry[i].PartitionType);
			part.set_part_handler(this->disk_handler);
			part.set_sector_size(this->sector_size);
			this->partitions->add_part(&part);
		}
	}
	return 0;
}

/***************************************
****************************************/

int disk::find_linux_partitions()
{
	partition_list *next_p;
	partition *p;
	int n=0;

	if(this->partitions == NULL)
	{
		this->partitions_ext2_count = 0;
		return -1;
	}
	
	next_p = this->partitions;
	while(next_p!=NULL)
	{
		p=next_p->get_partition();
		if(p->get_type() == 0x83)
		{
			partition_linux_ext2 *p_ext2 = (partition_linux_ext2 *)p;
			if (p_ext2->get_ext2_sb())
			{
				n++;
			}
		}	
		next_p = next_p->get_next();
	}

	this->partitions_ext2_count = n;
	return n;
}

/***************************************
****************************************/

partition_linux_ext2 **disk::ext2_partitions_init()
{
	partition_list *next_p;
	partition *p;
	partition_linux_ext2 *p_ext2=NULL;
	partition_linux_ext2 **ext2_part_tab=NULL;
	int n = 0, i = 0;

	if ((n = this->find_linux_partitions()) <= 0)
		return NULL;

	ext2_part_tab = new partition_linux_ext2 *[n+1];

	next_p = this->partitions;
	while(next_p!=NULL)
	{
		p = next_p->get_partition();
		if(p->get_type() == 0x83)
		{
			p_ext2=(partition_linux_ext2 *)p;
			//check if it is ext2 compatible fs
			if(p_ext2->ext2 != NULL && p_ext2->ext2->check_if_ext4_compatible() 
				&& p_ext2->ext2->gr_desc != NULL)
			{				
				ext2_part_tab[i++] = (partition_linux_ext2 *)p;
			}
		}	
		next_p = next_p->get_next();
	}
	ext2_part_tab[n] = NULL;

	return ext2_part_tab;
}
