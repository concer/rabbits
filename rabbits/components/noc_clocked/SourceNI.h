/*
 Copyright (c) 2007-2008 The Regents of the University of California.
 All rights reserved.

 Permission is hereby granted, without written agreement and without
 license or royalty fees, to use, copy, modify, and distribute this
 software and its documentation for any purpose, provided that the
 above copyright notice and the following two paragraphs appear in all
 copies of this software and that appropriate acknowledgments are made
 to the research of the COSI group.

 IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

 THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
 PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 ENHANCEMENTS, OR MODIFICATIONS.

 Author : Alessandro Pinto <apinto@eecs.berkeley.edu>
 University of California, Berkeley
 545 Cory Hall, Berkeley, CA 94720
 */
#ifndef SOURCEIP_H_
#define SOURCEIP_H_

#include <vector>
#include <queue>
#include "systemc.h"
#include "float.h"
#include "Types.h"
#include "config_noc.h" 
#include "mwsr_ta_fifo.h"
#include "abstract_noc.h"
 

using namespace std ;

#define DEBUG

template< int FlitWidth >
class SourceNI : public sc_module
{

public:

/*********************************************************************************************************/
/*									Public  IO Ports													 */	
/*********************************************************************************************************/

	sc_in<bool> Clock ;
	sc_out< sc_bv< FlitWidth > > DataOut ;
	sc_out< bool > ValidOut;
	sc_in< bool > AckIn ;
	sc_in< bool > FullIn ;

/*********************************************************************************************************/
/*									Public Methods														 */	
/*********************************************************************************************************/

  	SC_HAS_PROCESS( SourceNI ) ;
	
	// SourceNI( sc_module_name pName , int pId, int qSize,   map<string,int> _rabbitToCosi_map ) : sc_module( pName )  {		
	SourceNI( sc_module_name pName , int pId, int qSize ) : sc_module( pName )  {				
		DDB(__FUNCTION__);
		mId = pId ;

		mState = IDLE ;
		mWstate = WIDLE ;	
		mIsMaster = false; // modified by the constructor of the NoC
		mFlitCount=0; 
		mSentPackets=0;

		mInitialized = false;
		mQsize = qSize;

		SC_THREAD( Fsm ) ;
		sensitive_pos << Clock ;
		
		// SC_THREAD( go ) ;
	};
	
	void go(); // debug traffic source

	void initialize(bool _isMaster );
	
	// blocking request write from the core to the source
	void put_request(vci_request &req);
	void put_response(vci_response &resp);
	
 private:

/*********************************************************************************************************/
/*									Private  Fields														 */	
/*********************************************************************************************************/

	enum{ IDLE, FLIT } mState;
	enum{ WIDLE , TX } mWstate ;

	// blocking queues
  	mwsr_ta_fifo<vci_request> *m_queue_requests;
  	mwsr_ta_fifo<vci_response> *m_queue_responses;

	vci_request mCurrentRequest;
	vci_response mCurrentResponse;

	// vector<unsigned int> & mMasterRabbitsToCosiNode;
	// vector<unsigned int> & mSlaveRabbitsToCosiNode;
	
	vector< sc_bv< FlitWidth > > mCurrentPacket ;
	int mCurrentDest ;
   
	int mId ;
	bool mIsMaster;	
	int mFlitCount; // sent flit counter
	unsigned int mSentBits;
	unsigned int mSentPackets;
	
	bool mInitialized;
	int mQsize;
	

/*********************************************************************************************************/
/*									Private  Methods													 */	
/*********************************************************************************************************/
	inline void BuildPacketRequest();
	inline void BuildPacketResponse();

	void Fsm( );

} ;

/*********************************************************************************************************/
template< int FlitWidth >
// void SourceNI< FlitWidth >::initialize(bool _isMaster,vector<unsigned int> &vmaster, vector<unsigned int> &vslave ){
void SourceNI< FlitWidth >::initialize(bool _isMaster){	
/*********************************************************************************************************/

	mIsMaster = _isMaster; 
	// mMasterRabbitsToCosiNode = vmaster;
	// mSlaveRabbitsToCosiNode = vslave;
	mInitialized = true;
	
	if(mIsMaster){
		m_queue_requests = new mwsr_ta_fifo<vci_request>(mQsize);
		m_queue_responses = NULL;
	}else{
		m_queue_requests = NULL;
		m_queue_responses = new mwsr_ta_fifo<vci_response>(mQsize);
	}
}

/*********************************************************************************************************/
template< int FlitWidth >
void  SourceNI< FlitWidth >::Fsm(){
/*********************************************************************************************************/	
int Index ;
	DDB(__FUNCTION__);
	
	if(mInitialized == false){
		cout << this->name()<< ": Error, object should be initialized with the method 'initialize(isMaster)"<<endl;
		exit(1);
	}
	
	
 	while( true ){
   
	wait( ) ;

   	switch( mState ) {
    case IDLE:
	
		// blocking read, this should block the thread if no data are available
		// and the thread should be resumed only on data arrival and not the clock event...
		// TODO:test it
		
		// build a packet and savet in the list of flits to send
		if( mIsMaster ){
			mCurrentRequest = m_queue_requests->Read(); 
			
			BuildPacketRequest();
		}
		else{
			mCurrentResponse = m_queue_responses->Read(); 
			
			BuildPacketResponse();
		}
		mSentPackets++;
		// move to the new state
		mState = FLIT ;
 		break;

    case FLIT :
		switch( mWstate ) {
			
		case WIDLE:
		if ( ! FullIn.read() ) {
	  		DataOut.write(mCurrentPacket[ mFlitCount - 1 ]);	
			
			ValidOut.write(true) ;
		  	mWstate = TX ;
		}
		break;
 			case TX:
		if ( AckIn.read() ) {
			
			ValidOut.write(false) ;
			mSentBits += FlitWidth;
			
			mFlitCount--;
			if (  mFlitCount == 0 ){
				mState = IDLE ;
			}
			else
				mState = FLIT ;
			 	mWstate = WIDLE ;
		}
		break;
     		} // end of switch( mWstate )
    } // end of switch( mState ) 
  }
}
	


/*********************************************************************************************************/
// blocking request write from the core to the source
template< int FlitWidth >
void  SourceNI< FlitWidth >::put_request(vci_request& req){
/*********************************************************************************************************/		
	DDB(__FUNCTION__);
	// cout << "Source s:"<<source<< " adding request:"<< &req <<endl;
	if(mIsMaster)
 		m_queue_requests->Write(req);
	else{
		printf("Illegal operation: Slave (%s) calling a %s() \n ",this->name(),__FUNCTION__);
		exit(1);
	}
};

/*********************************************************************************************************/
template< int FlitWidth >
void  SourceNI< FlitWidth >::put_response(vci_response &resp){
/*********************************************************************************************************/		
	DDB(__FUNCTION__);
	// cout << "noc adding resp for s:"<<source<< " d:"<< dest <<endl;
	if(mIsMaster == false)
 		m_queue_responses->Write(resp);
	else{
		printf("Illegal operation: Master (%s) calling a %s() \n ",this->name(),__FUNCTION__);
		exit(1);
	}	 	
};

/*********************************************************************************************************/
template< int FlitWidth >
inline void  SourceNI< FlitWidth >::BuildPacketRequest(){
/*********************************************************************************************************/	
	DDB(__FUNCTION__);
	// n flits including header: header+ 3flit req fields + 2 data + tail (time)
	mFlitCount = 9;
	int flit = mFlitCount-1;
	
	//Build up the packet
 	mCurrentPacket.clear( ) ;
	mCurrentPacket.resize( mFlitCount , 0 ) ;
	
	// F1: HEADER
	mCurrentPacket[ flit ].range( 7, 0) = mCurrentRequest.srcid ;
	mCurrentPacket[ flit ].range(15 , 8) = mCurrentRequest.slave_id;
	mCurrentPacket[ flit ].range( 23,16) = mFlitCount;
	mCurrentPacket[ flit ].range(31,24) = 0;	
	// cout << "defined heaer :" << mCurrentPacket[ flit ] << "flit: "<< flit <<endl;
	// F2
	flit--;
	mCurrentPacket[ flit ].range(31,0) = mCurrentRequest.address;
	// mCurrentPacket[ flit ].range(31,0) = mSentPackets;	
	
	// F3
	flit--;
	mCurrentPacket[ flit ].range(7,0) = (uint8_t) mCurrentRequest.be;
	mCurrentPacket[ flit ].range(15,8) = mCurrentRequest.cmd;
	mCurrentPacket[ flit ].range(23,16) = mCurrentRequest.plen;
	mCurrentPacket[ flit ].range(31,24) = mCurrentRequest.trdid;

	// F4
	flit--;
	mCurrentPacket[ flit ].range(0,0) = mCurrentRequest.contig;
	mCurrentPacket[ flit ].range(1,1) = mCurrentRequest.eop;
	mCurrentPacket[ flit ].range(2,2) = mCurrentRequest.cons;
	mCurrentPacket[ flit ].range(3,3) = mCurrentRequest.wrap;
	mCurrentPacket[ flit ].range(4,4) = mCurrentRequest.cfixed;
	mCurrentPacket[ flit ].range(5,5) = mCurrentRequest.clen;
	mCurrentPacket[ flit ].range(21,6) = mCurrentRequest.srcid;
	mCurrentPacket[ flit ].range(29,22) = mCurrentRequest.pktid;		
	mCurrentPacket[ flit ].range(31,30) = 0;			

	// F5
	flit--;
	mCurrentPacket[ flit ].range(31,0) = (uint32_t)mCurrentRequest.initial_address;
	
	// F6
	flit--;
	mCurrentPacket[ flit ].range(15,0) = mCurrentRequest.slave_id;
	mCurrentPacket[ flit ].range(31,16) = mCurrentRequest.n_bytes;

	// F7 wdata[0] .. wdata[3]
	flit--;
	mCurrentPacket[ flit ].range(7,0) = mCurrentRequest.wdata[0];
	mCurrentPacket[ flit ].range(15,8) = mCurrentRequest.wdata[1];
	mCurrentPacket[ flit ].range(23,16) = mCurrentRequest.wdata[2];
	mCurrentPacket[ flit ].range(31,24) = mCurrentRequest.wdata[3];

	// F8 wdata[4] .. wdata[7]
	flit--;
	mCurrentPacket[ flit ].range(7,0) = mCurrentRequest.wdata[4];
	mCurrentPacket[ flit ].range(15,8) = mCurrentRequest.wdata[5];
	mCurrentPacket[ flit ].range(23,16) = mCurrentRequest.wdata[6];
	mCurrentPacket[ flit ].range(31,24) = mCurrentRequest.wdata[7];

	//F 9 Tail flit has a time stamp
	flit--;
	// mCurrentPacket[ flit ].range(31,0) = (int) sc_simulation_time( ) ;
	mCurrentPacket[ flit ].range(31,0) = (int) sc_time_stamp( ).to_double() ;	
	
	if(flit != 0)printf("%s->%s: Error flits count not correct!\n",this->name(),__FUNCTION__);
}

/*********************************************************************************************************/
template< int FlitWidth >
inline void  SourceNI< FlitWidth >::BuildPacketResponse(){
/*********************************************************************************************************/	
	DDB(__FUNCTION__);
	// n flits including header: header+ 2flit rep fields+ 1 nico info + 2 data + tail (time)
	mFlitCount = 7;
	int flit = mFlitCount-1;
	
	//Build up the packet
 	mCurrentPacket.clear( ) ;
	mCurrentPacket.resize( mFlitCount , 0 ) ;
	
	// HEADER
	mCurrentPacket[ flit ].range( 7, 0) = mId ;
	mCurrentPacket[ flit ].range(15 , 8) = mCurrentResponse.rsrcid;
	mCurrentPacket[ flit ].range( 23,16) = mFlitCount;
	mCurrentPacket[ flit ].range(31,24) = 0;
	// F1
	flit--;
	mCurrentPacket[ flit ].range(15,0) = mCurrentResponse.rsrcid;
	mCurrentPacket[ flit ].range(23,16) = mCurrentResponse.rtrdid;
	mCurrentPacket[ flit ].range(31,24) = mCurrentResponse.rpktid;	
	// F2
	flit--;
	mCurrentPacket[ flit ].range(0,0) = mCurrentResponse.reop;
	mCurrentPacket[ flit ].range(1,1) = mCurrentResponse.rerror;
	mCurrentPacket[ flit ].range(9,2) = mCurrentResponse.rbe;
	mCurrentPacket[ flit ].range(31,3) = 0;

	// F3
	flit--;
	mCurrentPacket[ flit ].range(15,0) = mCurrentResponse.slave_id;
	mCurrentPacket[ flit ].range(23,16) = mCurrentResponse.rcmd;
	mCurrentPacket[ flit ].range(31,24) = mCurrentResponse.n_bytes;
	
	// wdata[0] .. wdata[3]
	flit--;
	mCurrentPacket[ flit ].range(7,0) = mCurrentResponse.rdata[0];
	mCurrentPacket[ flit ].range(15,8) = mCurrentResponse.rdata[1];
	mCurrentPacket[ flit ].range(23,16) = mCurrentResponse.rdata[2];
	mCurrentPacket[ flit ].range(31,24) = mCurrentResponse.rdata[3];
	// wdata[4] .. wdata[7]
	flit--;
	mCurrentPacket[ flit ].range(7,0) = mCurrentResponse.rdata[4];
	mCurrentPacket[ flit ].range(15,8) = mCurrentResponse.rdata[5];
	mCurrentPacket[ flit ].range(23,16) = mCurrentResponse.rdata[6];
	mCurrentPacket[ flit ].range(31,24) = mCurrentResponse.rdata[7];
	//Tail flit has a time stamp
	flit--;
	if(flit != 0){
		printf("%s->%s: Error flits count not correct!\n",this->name(),__FUNCTION__);
	}		
	mCurrentPacket[0] = (int) sc_simulation_time( ) ;
 	
	// mFlitCount[mCurrentDest] = mFlows[mCurrentDest].PacketSize ;
}

/*********************************************************************************************************/
/*********************************************************************************************************/
template< int FlitWidth >
void  SourceNI< FlitWidth >::go(){
/*********************************************************************************************************/
	DDB(__FUNCTION__);

	map<int, int> cosiToRabbit_map;
	cosiToRabbit_map[0] = 0;
	cosiToRabbit_map[1] = 1;
	cosiToRabbit_map[2] = 10;
	cosiToRabbit_map[3] = 11;
	cosiToRabbit_map[4] = 12;
	cosiToRabbit_map[5] = 13;
	cosiToRabbit_map[6] = 2;
	cosiToRabbit_map[7] = 3;
	cosiToRabbit_map[8] = 4;
	cosiToRabbit_map[9] = 5;
	cosiToRabbit_map[10] = 6;
	cosiToRabbit_map[11] = 7;
	cosiToRabbit_map[12] = 8;
	cosiToRabbit_map[13] = 9;
	
	cout << "SOURCE "<< mId<< " Go Active" << endl;
	bool active = true;
	int d;
	vector<int> ds;
	// switch(cosiToRabbit_map[mId]){
	// // case cosi mID: dest rabbitID] );
	// 	case 0:
	// 		ds.push_back( rabbitToCosi_map[8] );
	// 		ds.push_back( rabbitToCosi_map[9] );
	// 		ds.push_back( rabbitToCosi_map[12] );
	// 		ds.push_back( rabbitToCosi_map[13] );
	// 	break;
	// 	case 1:
	// 		ds.push_back( rabbitToCosi_map[8] );
	// 		ds.push_back( rabbitToCosi_map[12] );
	// 		ds.push_back( rabbitToCosi_map[13] );
	// 	break;
	// 	case 2:
	// 		ds.push_back( rabbitToCosi_map[12] );
	// 		ds.push_back( rabbitToCosi_map[13] );
	// 	break;
	// 	case 3:
	// 		ds.push_back( rabbitToCosi_map[7] );
	// 	break;
	// 	case 4:
	// 		ds.push_back( rabbitToCosi_map[7] );
	// 	break;
	// 	case 7: //r7
	// 		ds.push_back( rabbitToCosi_map[3] );
	// 		ds.push_back( rabbitToCosi_map[4] );
	// 	break;
	// 	case 8: //r8
	// 		ds.push_back( rabbitToCosi_map[0] );
	// 		ds.push_back( rabbitToCosi_map[1] );
	// 	break;
	// 	case 9: //r9
	// 		ds.push_back( rabbitToCosi_map[0] );
	// 	break;
	// 	case 12: //r8
	// 		ds.push_back( rabbitToCosi_map[0] );
	// 		ds.push_back( rabbitToCosi_map[1] );
	// 		ds.push_back( rabbitToCosi_map[2] );
	// 	break;
	// 	case 13://r9
	// 		ds.push_back( rabbitToCosi_map[0] );
	// 		ds.push_back( rabbitToCosi_map[1] );
	// 		ds.push_back( rabbitToCosi_map[2] );
	// 	break;
	// 	default:
	// 	cout << "S:"<< mId << " " << this->name() << " INACTIVE" << endl;
	// 		active=false;
	// }
	
	vci_request req;
	int                     i;
	unsigned char           ofs, mask_be, plen;
	unsigned long nbytes;
	bool bWrite;
	unsigned long addr;

			
	while(1){ 
		wait(15,SC_NS);
		
		// mSentPackets++;

		if( ! active ) continue;
		
		// if(cosiToRabbit_map[mId] != 12) 	continue;

		DDB("GO sending request!");

		nbytes=4;
		bWrite=1;
		addr=0xDEADDEAD;
	
		plen = (nbytes + 3) >> 2;
	    if (nbytes > 4)
	        nbytes = 4;
	    ofs = addr & 0x000000007;
	    addr &= 0xFFFFFFF8;

	 	unsigned char s_operation_mask_be[]={0xDE, 0x01, 0x03, 0xDE, 0x0F, 0xDE, 0xDE, 0xDE, 0xFF};
	
	    mask_be = s_operation_mask_be[nbytes] << ofs;

	    req.cmd     = 0xDD;
	    req.be      = 0xFA;
	    req.address = 0xC1A000;
	    req.trdid   = 25;
	    req.srcid   = mId;
	    req.plen    = 12;
	    req.eop     = true;
		req.n_bytes = 8;
		req.pktid = (uint8_t) mSentPackets;
		req.slave_id = ds[d];
		
		if(d+1 >= ds.size()) 
			d=0 ;
		else 
			d++;
		cout <<endl<< this->name()<<" c"<< mId
			<<"->:c"<< req.slave_id 
			<< " id:"<< (int)req.pktid<< endl;
		// ---------
	    memset (&req.wdata, 0, 8);

	    if (bWrite)
	    {
	        for (i = 0; i < 8; i++)
	            req.wdata[i] = 0xFF;
	    }

		// printf("S: snd :%d\n",mSentPackets);
		put_request(req);   
	}
}
  
#endif /*SOURCEIP_H_*/
