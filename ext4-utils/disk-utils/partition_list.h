#ifndef __PARTITION_LIST_H__
#define __PARTITION_LIST_H__

#include "partition.h"

/**********************************************************
	LIST_PARTITION class
***********************************************************/

class partition_list
{
	partition *p;
	partition_list *next;
public:
	partition_list():p(NULL), next(NULL) {}
	~partition_list();
	partition_list *get_next(){return next;}
	partition *get_partition(){return p;}
	void add_part(partition *new_part);
};

#endif