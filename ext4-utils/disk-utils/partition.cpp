#include "partition.h"
#include "disk.h"

/**********************************************************
  PARTITION class
***********************************************************/

partition::partition()
{
	active = FALSE;
	H_start = H_end = 0;
	S_start = S_end = 0;
	C_start = C_end = 0;
	LBA = 0;
	type = 0;
	n_sectors = 0;
	primary = TRUE;
	offset = 0;
	part_handler = INVALID_HANDLE_VALUE;
}


/**************************************
 set_part_info
 - public
***************************************/

void partition::set_part_info(unsigned char *buffer)
{
		active = (buffer[0] == 0x80) ? TRUE : FALSE;
		H_start = buffer[1];
		S_start = buffer[2]; 
		S_start<<= 2;
		S_start>>= 2;
		C_start = buffer[3]; //fix
		type = buffer[4];
		H_end = buffer[5];
		S_end = buffer[6];
		S_end<<= 2;
		S_end>>= 2;
		C_end = buffer[7];   //fix
		LBA = *(unsigned int *)(buffer+8);
		n_sectors = *(unsigned int *)(buffer+12);
}


/**********************************************************
***********************************************************/

unsigned char *partition::read_sectors(DWORD s_start, unsigned int n_sectors)
{
	DWORD bytes_recv;
	unsigned char *buffer=NULL;
	LARGE_INTEGER l_offset;
	
	if(this->part_handler != INVALID_HANDLE_VALUE)
	{	
		/*l_offset.QuadPart = s_start + this->offset;
		l_offset.QuadPart <<= 9;*/
		l_offset.QuadPart = s_start * this->sector_size;
		l_offset.QuadPart += this->offset;
		if(SetFilePointerEx(this->part_handler, l_offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
			return NULL;	
		buffer = new unsigned char[n_sectors * this->sector_size];
		if(!ReadFile(this->part_handler, buffer, n_sectors * this->sector_size, &bytes_recv, NULL))
		{
			delete [] buffer;
			return NULL;
		}
	}
	return buffer;
}


unsigned char *partition::read_sectors_ex(unsigned long long s_start, unsigned int n_sectors)
{
	DWORD bytes_recv;
	unsigned char *buffer = NULL;
	LARGE_INTEGER l_offset;
	
	if(this->part_handler != INVALID_HANDLE_VALUE)
	{	
		/*l_offset.QuadPart = s_start + this->offset;
		l_offset.QuadPart <<= 9;*/
		l_offset.QuadPart = s_start * this->sector_size;
		l_offset.QuadPart += this->offset;
		if(SetFilePointerEx(this->part_handler, l_offset, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
			return NULL;	
		buffer=new unsigned char[n_sectors * this->sector_size];
		if(!ReadFile(this->part_handler, buffer, n_sectors * this->sector_size, &bytes_recv, NULL))
		{
			delete [] buffer;
			return NULL;
		}
	}
	return buffer;
}
