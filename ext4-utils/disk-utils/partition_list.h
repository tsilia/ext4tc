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