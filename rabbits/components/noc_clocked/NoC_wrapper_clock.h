

#ifndef __2NOC_CLOCK2__
#define __2NOC_CLOCK2__
#include <map>
#include <vector>
#include "NoC_cosi_implementation_clock.h"
#include "abstract_noc.h"
#include "SourceNI.h"
#include "DestNI.h"

#include "config_noc.h"

// #define RtC( x ) this->rabbitToCosi_map[x]
// #define CtT( x ) this->cosiToRabbit_map[x]

using namespace std;



template< int FlitWidth >
class NoC_wrapper_clock : public NoC_cosi_implementation_clock< FlitWidth >
{
// typedef SourceNI<FlitWidth>* source_t;
	stringstream mName;
	int nmasters;
	int nslaves;
	vector<unsigned int> mMasterRabbitsToCosiNode;
	vector<unsigned int> mSlaveRabbitsToCosiNode;	
	vector<unsigned int> mCosiNodeToRabbitsId;		
public:

	/******************************/
	~NoC_wrapper_clock(){};
	/******************************/
	NoC_wrapper_clock( sc_module_name pName, int nmaster, int nslave, int ni_qsize, int r_qsize ):
		NoC_cosi_implementation_clock< FlitWidth >( pName, ni_qsize,r_qsize){

		DDB(__FUNCTION__);
		nmasters = nmaster;
		nslaves = nslave;

		// initialize the Rabbits to Cosi ID translation vectors
		int i=0;
		for (typename std::map<string, SourceNI<FlitWidth>*>::iterator m = this->m_src.begin();m != this->m_src.end(); ++m ){
		 		if(i < nmaster){
					mMasterRabbitsToCosiNode.push_back(i);
					mCosiNodeToRabbitsId.push_back(i);
				}
		 		else{
		 			mSlaveRabbitsToCosiNode.push_back(i); // so slave0 -> 6 ...
					mCosiNodeToRabbitsId.push_back(i-nmaster); // so 6 -> 0
				}
		 		i++;
		}
		//
		i=0;
		for (typename std::map<string, SourceNI<FlitWidth>*>::iterator m = this->m_src.begin();m != this->m_src.end(); ++m ){
			 if(N_DEBUG)cout<< "s initializing :"<< i<< " =>" << (*m).first<< endl;
		 		if(i < nmaster)
					(*m).second->initialize(true);
		 		else
		 			(*m).second->initialize(false);
		 		i++;
		}
		//
		i=0;
		cout << "---\n";
		for ( typename std::map<string, DestNI<FlitWidth>*>::iterator im = this->m_dst.begin();	im != this->m_dst.end(); ++im ){
			if(N_DEBUG) cout<< "d initializing :"<< i<< " =>" << (*im).first<< endl;
		  if(i < nmaster)
		    (*im).second->initialize(true);
		  else
		    (*im).second->initialize(false);
		  i++;
		}
	};
	
	/******************************/
	void put_request(const string &module,vci_request &req){
		DDB(__FUNCTION__);

		int s = req.srcid;
		int d = req.slave_id;
		
		req.srcid = mMasterRabbitsToCosiNode[req.srcid];
		req.slave_id = mSlaveRabbitsToCosiNode[req.slave_id];		
		
	 	if(N_DEBUG)	cout << "-----" << "noc " << __FUNCTION__ << "  for: "<< module << endl;
			// << req << endl;	
			
		this->m_src[module]->put_request(req);			
	};
	
	/******************************/
	void put_response(const string &module, vci_response& resp){
		DDB(__FUNCTION__);

		resp.rsrcid = mMasterRabbitsToCosiNode[resp.rsrcid];
		resp.slave_id = mSlaveRabbitsToCosiNode[resp.slave_id];		

	 	 // cout << "noc " << __FUNCTION__ << "  for: "<< module << endl;		

	 	if(N_DEBUG)	cout << "noc " << __FUNCTION__ << "  for: "<< module << endl;
			// << resp << endl;	

			
	 	this->m_src[module]->put_response(resp); 	
	};
	
	/******************************/
	inline
	void get_request(const string &module, vci_request &req){		
		DDB(__FUNCTION__);
	 	if(N_DEBUG) 	cout << "noc " << __FUNCTION__ << " for: "<< module <<endl;
		
		this->m_dst[module]->get_request(req);		
		
		// convert the cosi IDs to Rabbits IDs
		req.srcid = mCosiNodeToRabbitsId[req.srcid];
		req.slave_id = mCosiNodeToRabbitsId[req.slave_id];		
		
		// cout << "noc : request read! by: "<< module <<endl;
	};
	
	/******************************/
	inline
	void get_response(const string &module, vci_response &resp){
		DDB(__FUNCTION__);
	 	if(N_DEBUG)	cout << "noc " << __FUNCTION__ << "  for: "<< module <<endl;

		// convert the cosi IDs to Rabbits IDs
		resp.rsrcid = mCosiNodeToRabbitsId[resp.rsrcid];
		resp.slave_id = mCosiNodeToRabbitsId[resp.slave_id];		
	
		this->m_dst[module]->get_response(resp);
		// cout << "noc : response read! by: "<< module <<endl;
	};
	
	/******************************/
	void hallo(){
		cout << "Hallo world" << endl;
	}


};

#endif