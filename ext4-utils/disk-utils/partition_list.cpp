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
#include "partition_list.h"
#include "partition_linux_ext2.h"

/**********************************************************
	PARTITION_LIST class
***********************************************************/
partition_list::~partition_list()
{
	partition_list *part_next=this->next;

	if(part_next!=NULL)
		delete part_next;	
	delete this->p;
}

/**********************************************************
***********************************************************/

void partition_list::add_part(partition *new_part)
{
	partition_list *p__next=this, *p__prev=this;
	partition *part_tmp;
	
	if(new_part->get_type() == 0x83)
		part_tmp = new partition_linux_ext2;
	else
		part_tmp = new partition;
	if(part_tmp == NULL)
		return;

	if(this->p == NULL)
	{	
		this->p = part_tmp;
		*(this->p) = *new_part;
		this->next = NULL;
	}
	else
	{
		while(p__next!=NULL)
		{
			p__prev = p__next;
			p__next = p__next->next;
		}

		p__next = new partition_list;
		if(p__next==NULL)
			return;
		p__next->p = part_tmp;

		*(p__next->p) = *new_part;
		p__prev->next = p__next;

	}
		

}
