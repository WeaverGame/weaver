/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "qbsp.h"

/*
==============================================================================

LEAF FILE GENERATION

Save out name.line for qe3 to read
==============================================================================
*/


/*
=============
LeakFile

Finds the shortest possible chain of portals
that leads from the outside leaf to a specifically
occupied leaf

TTimo: builds a polyline xml node
=============
*/
xmlNodePtr LeakFile(tree_t * tree)
{
	vec3_t          mid;
	FILE           *linefile;
	char            filename[1024];
	node_t         *node;
	int             count;
	xmlNodePtr      xml_node, point;

	if(!tree->outside_node.occupied)
		return NULL;

	Sys_FPrintf(SYS_VRB, "--- LeakFile ---\n");

	//
	// write the points to the file
	//
	sprintf(filename, "%s.lin", source);
	linefile = fopen(filename, "w");
	if(!linefile)
		Error("Couldn't open %s\n", filename);

	xml_node = xmlNewNode(NULL, "polyline");

	count = 0;
	node = &tree->outside_node;
	while(node->occupied > 1)
	{
		int             next;
		portal_t       *p, *nextportal;
		node_t         *nextnode;
		int             s;

		// find the best portal exit
		next = node->occupied;
		for(p = node->portals; p; p = p->next[!s])
		{
			s = (p->nodes[0] == node);
			if(p->nodes[s]->occupied && p->nodes[s]->occupied < next)
			{
				nextportal = p;
				nextnode = p->nodes[s];
				next = nextnode->occupied;
			}
		}
		node = nextnode;
		WindingCenter(nextportal->winding, mid);
		fprintf(linefile, "%f %f %f\n", mid[0], mid[1], mid[2]);
		point = xml_NodeForVec(mid);
		xmlAddChild(xml_node, point);
		count++;
	}
	// add the occupant center
	GetVectorForKey(node->occupant, "origin", mid);

	fprintf(linefile, "%f %f %f\n", mid[0], mid[1], mid[2]);
	point = xml_NodeForVec(mid);
	xmlAddChild(xml_node, point);
	Sys_FPrintf(SYS_VRB, "%9d point linefile\n", count + 1);

	fclose(linefile);

	return xml_node;
}
