#ifndef __FS_GENERIC_H__
#define __FS_GENERIC_H__

class fs_generic
{
private:
	void *superblock;
public:
	virtual int read_super_block() = 0;
};

#endif
