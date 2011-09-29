/*
 *  Copyright (c) 2010 TIMA Laboratory
 *
 *  This file is part of Rabbits.
 *
 *  Rabbits is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Rabbits is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Rabbits.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TRAFFIC_MONITOR_H
#define _TRAFFIC_MONITOR_H

#include <abstract_noc.h>
#include <systemc.h>
#include <string.h>
#include <vector>
#include <map>
#include "config_noc.h"
// #define PRINT_SCREEN


/**************** 
	Nicola 
*****************/

#define TRAFFIC_MONITOR_DELAY 1000
#define TRAFFIC_SHORT_MSG_SIZE 64

typedef struct{
	unsigned long bytes;
	unsigned int read_reqs;
	unsigned int read_resp;	
	unsigned int write_reqs;
	unsigned int write_resp;
	unsigned int noops;
	unsigned int transactions;
	}	t_cell;

	
typedef std::map<int,t_cell>   t_traffic_cell;
// typedef std::map<int,unsigned long>   t_traffic_cell;

class c_traffic{

public:
	c_traffic(int _n_masters,int _n_slaves);

	~c_traffic();

	// void add_master_transaction(int master, int slave,  unsigned long data);
	void add_master_transaction(vci_request& req);
	void add_slave_transaction(vci_response& resp);
	void print_values(); 
	void reset_values();

 private:
	int n_masters;
	int n_slaves;
	int nodes;
	
	// ----------------------
	#ifdef PRINT_VECTORS
	
	int n_flows;
	uint64_t current_t;
	std::map<std::pair<int,int>,int> m_vec_id; // <source,dest>->vec id;
	ofstream* f; // vector file 

	#ifdef PRINT_MAX_TRAFFIC
	ofstream* maxf; // vector file 	
	std::map<std::pair<int,int>,unsigned long> max_flow; // <source,dest>->max flow;
	#endif
	
	#endif
	// ----------------------	
	std::vector<t_traffic_cell> vec;
};


/**************** 
	End Nicola 
*****************/


#endif

/*
 * Vim standard variables
 * vim:set ts=4 expandtab tw=80 cindent syntax=c:
 *
 * Emacs standard variables
 * Local Variables:
 * mode: c
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
