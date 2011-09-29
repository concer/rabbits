/**
 * @filename traffic_tracjer.cpp
 *
 * @brief allows to print on screen and on a text file the traffic (in terms of Bytes per second)
measured on the NoC
 *
 * @author Nicola Concer
 * @sa        traffic_tracjer.h
 */

#include <traffic_tracker.h>

/// Constructor
c_traffic::c_traffic(int _n_masters,int _n_slaves):n_masters(_n_masters), n_slaves(_n_slaves){
	nodes = _n_masters + _n_slaves;
	// vec= new std::vector<t_traffic_cell>;
	vec.resize(nodes);

	
	#ifdef PRINT_VECTORS
		n_flows=0;
		f = new ofstream( "traffic_vector.vec" );
	#endif
	#ifdef PRINT_MAX_TRAFFIC
		maxf = new ofstream( "traffic_max_flows.txt" );
	#endif
};

/// Destructor
c_traffic::~c_traffic()
{
	vec.resize(0);
	#ifdef PRINT_VECTORS
		f->close();
		
		t_traffic_cell::const_iterator it;
		std::pair<int,int> p;
	#endif
	#ifdef PRINT_MAX_TRAFFIC
		maxf->close();
	#endif
	
}

/**
 * Records a request for a given <master,slave> and its type
 * forwards them on the output.
 */
void c_traffic::add_master_transaction(vci_request& req){	
	if(req.srcid >= n_masters || req.slave_id >= nodes ){
		printf("error add_master_transaction %d >= %d || %d >= %d\n",req.srcid,n_masters,req.slave_id , nodes);
		exit(1);
	}

	int slave_id = n_masters + req.slave_id;
	vec[req.srcid][slave_id].transactions+=1;
	vec[req.srcid][slave_id].bytes += TRAFFIC_SHORT_MSG_SIZE;

	if(req.cmd == CMD_READ || req.cmd == CMD_LOCKED_READ){
		vec[req.srcid][slave_id].read_reqs++;
	}
		
	else if(req.cmd == CMD_WRITE || req.cmd == CMD_STORE_COND){
		vec[req.srcid][slave_id].write_reqs++;
		vec[req.srcid][slave_id].bytes += req.n_bytes;
	}
		
	else{
		vec[req.srcid][slave_id].noops++;		
	}
	// printf("m:%d s:%d d:%lu\n",req.srcid,slave_id,vec[req.srcid][slave_id].bytes);
}

/**
 * Records a response for a given <master,slave> and its type
 * forwards them on the output.
 */

void c_traffic::add_slave_transaction(vci_response& resp){
	
	int s = n_masters+resp.slave_id;
	int d = resp.rsrcid;
	
	if(resp.rsrcid >= n_masters || resp.rsrcid+resp.rsrcid >= nodes ){
		printf("error add_slave_transaction: m:%d s:%d nm:%d nn:%d s:%d d:%d \n",
			resp.rsrcid,resp.slave_id,n_masters,nodes,s,d);
		exit(1);
	}
	vec[s][d].bytes+=TRAFFIC_SHORT_MSG_SIZE;
		
	vec[s][d].transactions+=1;
	if(resp.rcmd == CMD_READ || resp.rcmd == CMD_LOCKED_READ){
		vec[s][d].read_reqs++;
		vec[s][d].bytes+=resp.n_bytes;
	}

	else if(resp.rcmd == CMD_WRITE || resp.rcmd == CMD_STORE_COND){
		vec[s][d].write_reqs++;
	}

	else{
		vec[s][d].noops++;
	}
	// printf("s:%d m:%d d:%lu\n",s,d,vec[s][d].bytes);
}

#ifdef PRINT_VECTORS
/**************************************************************************
 *  prints in the format for OMNeT++ output vectors
**************************************************************************/
void c_traffic::print_values(){ 
	t_traffic_cell::const_iterator it;
	std::pair<int,int> p;

	for(int i=0; i< vec.size(); i++){
		for(it = vec[i].begin(); it != vec[i].end(); ++it){
			p.first=i; // source
			p.second= (*it).first; // dest. Note in this way a lave has an id as id_s=nmasters+id_slave

			//check wether the flow is known and has an id
			if(m_vec_id[p]==0){
				m_vec_id[p]= ++ n_flows;
				if(i<n_masters)
					(*f) << "vector "<< n_flows << " \"\" \"m:"<< i << " s:"<< 	p.second <<"\" 1"<<endl;
					// cout << "vector "<< n_flows << " \"\" \"m:"<< i << " s:"<< (*it).second.bytes <<"\" 1"<<endl;
					else
					(*f) << "vector "<< n_flows << " \"\" \"s:"<< i << " m:"<< 	p.second <<"\" 1"<<endl;
					// cout << "vector "<< n_flows << " \"\" \"s:"<< i << " m:"<< (*it).second.bytes <<"\" 1"<<endl;
			}	
			// flow id, time, value
			current_t = sc_time_stamp().value()/1000000000; //millisec (sec the number is too large)
			(*f) << m_vec_id[p] << "\t" <<  current_t << "\t" << (*it).second.bytes << endl;

			//check the maximum value of byte/sec for each flow
			#ifdef PRINT_MAX_TRAFFIC
			if(max_flow[p] < (*it).second.bytes) max_flow[p] = (*it).second.bytes;
			#endif
			// cout << m[p] << "\t" <<  current_t << "\t" << (*it).second.bytes << endl;			
		}			
	}

	#ifdef PRINT_MAX_TRAFFIC
		(*maxf)<<"-----------------------"<<endl;
		(*maxf)<<"flow max values"<<endl;
		(*maxf)<<"-----------------------"<<endl;
		
		std::map<std::pair<int,int>,unsigned long>::const_iterator ite; 
		for(ite = max_flow.begin(); ite != max_flow.end(); ++ite){
			p =(*ite).first; // <s.d>				
			(*maxf) << p.first  << " -> " << p.second << " : " << max_flow[p] << endl;
		}
	#endif
}

#elif defined(PRINT_SCREEN)
/**************************************************************************
 *  prints all the info on the screen
**************************************************************************/
void c_traffic::print_values(){ 
	t_traffic_cell::const_iterator it;
	printf("--------------------------------------------\n");
	for(int i=0; i< vec.size(); i++){
		for(it = vec[i].begin(); it != vec[i].end(); ++it){
		
			if(i<n_masters)
			printf("m:%d -> s:%d r:%u  w:%u  n:%u  t:%u  b:%lu\n", i, (*it).first,
					(*it).second.read_reqs,(*it).second.write_reqs,(*it).second.noops,
					(*it).second.transactions, (*it).second.bytes);	
			else
				// Note in this way a lave has an id as id_s=nmasters+id_slave. Use i-n_masters to get the real id
			printf("s:%d -> m:%d r:%u  w:%u  n:%u  t:%u  b:%lu\n", i, (*it).first, 
				(*it).second.read_resp,(*it).second.write_resp,(*it).second.noops,
				(*it).second.transactions,(*it).second.bytes);	
		}			
	}
}
#else
 void c_traffic::print_values(){}
#endif

void c_traffic::reset_values(){
	for(int i=0; i< vec.size(); i++){
		vec[i].clear();
	}
}



