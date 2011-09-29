/**
 * @filename traffic_tracjer.h
 *
 * @brief allows to print on screen and on a text file the traffic (in terms of Bytes per second)
measured on the NoC
 *
 * @author Nicola Concer
 * @sa        traffic_tracjer.cpp
 */

#ifndef _TRAFFIC_MONITOR_H
#define _TRAFFIC_MONITOR_H

#include <abstract_noc.h>
#include <systemc.h>
#include <string.h>
#include <vector>
#include <map>
#include "config_noc.h"

/// turn on/off the on-screen printing
// #define PRINT_SCREEN

/// plot the maximum values recorded so far
// #define PRINT_MAX_TRAFFIC



#define TRAFFIC_MONITOR_DELAY 1000
#define TRAFFIC_SHORT_MSG_SIZE 64

/// types of operations measured
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


/**
 * @brief Class that hold the vector of registered traffic
 * the traffic is recorded per pair <source-dest>  per second
 *
 * @tparam      _n_masters number of master nodes
 * @tparam      _n_slaves  number of slaves
 * 
 */
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
