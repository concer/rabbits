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
#ifndef DESTIP_H_
#define DESTIP_H_

#include "systemc.h"
#include <map>
#include <sstream>
#include "config_noc.h"
#include "mwsr_ta_fifo.h"
#include "abstract_noc.h"

template< int FlitWidth >
class DestNI : public sc_module
{
public:
	
/*********************************************************************************************************/
/*									Public  IO Ports													 */	
/*********************************************************************************************************/

	sc_in< bool > Clock ;
	sc_in< sc_bv< FlitWidth > > DataIn ;
	sc_in< bool > ValidIn ;
	sc_out< bool > AckOut ;
	sc_out< bool > FullOut ;

/*********************************************************************************************************/
/*									Public Methods														 */	
/*********************************************************************************************************/

	// void add_response(int source, int dest, vci_response* resp){};

	SC_HAS_PROCESS( DestNI ) ;

	// DestNI( sc_module_name n , int pId,   int qSize,  map<int,string> _mCosiToRabbit_map ) : sc_module( n ) {
	DestNI( sc_module_name n , int pId,   int qSize ) : sc_module( n ) {		
		DDB(__FUNCTION__);
				
		// mCosiToRabbit_map = _mCosiToRabbit_map;
		mState = IDLE ;
		mRstate = VALID ;
		mId = pId ;

		mInitialized = false;
		mQsize = qSize;
		
		SC_THREAD( Fsm ) ;
		sensitive_pos << Clock ;
		
		// SC_THREAD( reader );
	}

	void Fsm( ) ;
	
	// void setIsMaster(bool v){ mIsMaster = v;}
	
	void initialize(bool _isMaster);

	void Report( ) ;

	string GetLatencies( ) ;
	string GetBandwidths( ) ;
	
	void get_request(vci_request &);
	void get_response(vci_response &);
	
	void reader(); // debug only
private:
/*********************************************************************************************************/
/*									Private  Fields														 */	
/*********************************************************************************************************/

	//---- nico --- 
	bool mIsMaster;	
	// blocking queues
  	mwsr_ta_fifo<vci_request> *m_queue_requests;
  	mwsr_ta_fifo<vci_response> *m_queue_responses;
	vci_request mCurrentRequest;
	vci_response mCurrentResponse;
	// map<int, string> mCosiToRabbit_map;

	bool mInitialized;
	int mQsize;
	//---- nico --- 

	map< int , int > GetReceivedBits( ) ;

	enum{ IDLE , FLIT } mState ;
	enum{ VALID , RX } mRstate ;

	int mId ;
	//For each source, stores the number of received packets.
	map< int , int > mReceivedBits ;
	//Sum up the latencies to compute the average at the end
	map< int , double > mLatencies ;

	double GetAverageLatency( ) ;
	double GetMinLatency( ) ;
	double GetMaxLatency( ) ;
	int mPacketSize ;
	int mFlitCount ;
	int mCurrentSource ;
	int mCurrentDest ;
	
	int mCurrentPacketID;

	sc_bv< FlitWidth > mCurrentIn ;
/*********************************************************************************************************/
/*									Private  Methods													 */	
/*********************************************************************************************************/
	
	inline void processIncomingFlit();	

} ;
/*********************************************************************************************************/

/*********************************************************************************************************/
template< int FlitWidth >
void DestNI< FlitWidth >::initialize(bool _isMaster){
/*********************************************************************************************************/

	mIsMaster = _isMaster; 
	mInitialized = true;
	
	if(mIsMaster){
		m_queue_requests = NULL;
		m_queue_responses = new mwsr_ta_fifo<vci_response>(mQsize);	
	}else{
		m_queue_requests = new mwsr_ta_fifo<vci_request>(mQsize);
		m_queue_responses = NULL;
	}
}


/*********************************************************************************************************/
template< int FlitWidth >
void DestNI< FlitWidth >::Fsm( ){
/*********************************************************************************************************/
	DDB(__FUNCTION__);
 	// cout << "Destination " << name( ) << " running " << endl ;
	FullOut.write(false) ;
	AckOut.write(false) ;

	bool valid_in;

	if(mInitialized == false){
		cout << this->name()<< ": Error, object should be initialized with the method 'initialize(isMaster)"<<endl;
		exit(1);
	}

	while( true ) {

	wait( ) ;

	switch( mState ) { // 	enum{ IDLE , FLIT } mState ;
	case IDLE :
		// DDB("Destination IDLE");
	 	switch( mRstate ) {
	 	case VALID :
	
			if ( ValidIn.read() ) { //Get the header
				// cout << "=== new header ===" << this->name() << endl;
				DDB("Destination VALID, reading HEADER");				
				mCurrentIn = DataIn.read() ;

				mCurrentSource =  mCurrentIn.range( 7 , 0 ).to_int( ) ;
				mCurrentDest = ( mCurrentIn.range( 15 , 8 ) ).to_int( ) ;
				mPacketSize =  ( mCurrentIn.range( 23 , 16 ) ).to_int( ) ;
				mCurrentPacketID = mCurrentIn.range( 31 , 24 ).to_int( );
				mFlitCount = 1 ;
								
				if( mCurrentDest != mId ){
					cout<<endl << "Dest_"<< mId <<" (" << this->name()<< "): ERROR: packet delivered to wrong destinatary: "
					<< mCurrentSource
					<< "-->c"<< mCurrentDest<< endl;
					exit(1);
				}	

				// cout << "----" << endl  
				//  << this->name() << " Destination VALID, reading HEADER f:"<< mFlitCount << "/" << mPacketSize << endl;
				// cout << "D h: ------ packet size: " << mPacketSize << endl ;
				
				if ( mReceivedBits.find( mCurrentSource ) == mReceivedBits.end( ) ) {
					mReceivedBits[ mCurrentSource ] = FlitWidth ;
				} else {
					mReceivedBits[ mCurrentSource ] = mReceivedBits[ mCurrentSource ] + FlitWidth ;
				}
				mRstate = RX ;
				
				AckOut.write(true) ;
			}
		break ;
	 	case RX :
			// DDB("source RX");
		AckOut.write(false) ;
			mRstate = VALID ;
			mState = FLIT ;
		break ;
	 	}
	
    break;

    case FLIT :
		// DDB("source FLIT");
    	switch( mRstate ) { // enum{ VALID , RX } mRstate ;
      	case VALID :
			// DDB("source VALID");
			if ( ValidIn.read() ) { //Get the Flit
		  		mFlitCount ++ ;	
				// cout << this->name() << " f:"<< mFlitCount << "/" << mPacketSize << endl;
				// printf("mFlitCount:%d\n",mFlitCount);						
				processIncomingFlit();	
			  	mReceivedBits[ mCurrentSource ] = mReceivedBits[ mCurrentSource ] + FlitWidth ;

				//If it is the tail flit, compute latency
				 if( mFlitCount == mPacketSize ) {
					DDB("received FULL PACKET");

					//delivery the packet
					if(mIsMaster){
						// cout<<"received FULL PACKET writing!" << endl << mCurrentResponse <<endl;
						DDB("calling m_queue_responses->Write");
						m_queue_responses->Write(mCurrentResponse);
					} else{
						// cout<<this->name()<< ":received FULL PACKET writing!" << endl << mCurrentRequest <<endl;
						DDB("calling m_queue_requests->Write");
						m_queue_requests->Write(mCurrentRequest);
					}
					
				   	if ( mLatencies.find( mCurrentSource ) == mLatencies.end( ) ){
				    	mCurrentIn = DataIn.read() ;
				     	mLatencies[mCurrentSource] = sc_simulation_time( ) - mCurrentIn.to_int( )  ;
				   	} else {
				     	mCurrentIn = DataIn.read() ;
				     	mLatencies[mCurrentSource] += sc_simulation_time( ) - mCurrentIn.to_int( ) ;
				   	}
					mState = IDLE ;
					mRstate = VALID ;
					mFlitCount=0;				
				 }
				else {  
					// cout << "D: #:" << DataIn << endl ;		
				}
				mRstate = RX ;
				AckOut.write( true) ;
			}
		break ;
      	
		case RX :
			AckOut.write(false) ;
			mRstate = VALID ;
			if( mFlitCount == mPacketSize ) {
			 	mState = IDLE ;
			}else{
			  mState = FLIT ;
			}
		break ;
	}
    break;
    }
  }
}

/*********************************************************************************************************/
template< int FlitWidth >
void DestNI< FlitWidth >::get_request(vci_request & req){		
/*********************************************************************************************************/
	DDB(__FUNCTION__);
	// cout << "noc gettting req for:"<< ni <<endl;
	if(mIsMaster == false)
		req =  m_queue_requests->Read();
	else{
		printf("Master error, called  get_request() illegal\n ");
		exit(1);
	}	 	
};

/*********************************************************************************************************/
template< int FlitWidth >
void DestNI< FlitWidth >::get_response(vci_response &resp){
/*********************************************************************************************************/
	DDB(__FUNCTION__);
	if(mIsMaster)
 		resp =  m_queue_responses->Read();
	else{
		printf("Slave (%s) error, called  get_response() illegal\n ",this->name());
		exit(1);
	}	 	
};


/*********************************************************************************************************/
template< int FlitWidth >
inline void  DestNI< FlitWidth >::processIncomingFlit(){
/*********************************************************************************************************/
	DDB("------"<<__FUNCTION__<<" nf:"<< mFlitCount<< "------");
	mCurrentIn = DataIn.read() ;	
	
	if(mIsMaster){ // received a response
		switch(mFlitCount){
			case 2: DDB("resp 2");
			mCurrentResponse.rsrcid = ( mCurrentIn.range( 15 , 0 ) ).to_int( ) ;
			mCurrentResponse.rtrdid = ( mCurrentIn.range( 23 , 16 ) ).to_int( ) ;
			mCurrentResponse.rpktid = ( mCurrentIn.range( 31 , 24 ) ).to_int( ) ;
			break;
		case 3: DDB("resp 3");
			mCurrentResponse.reop = ( mCurrentIn.range( 0 , 0 ) ).to_int( ) ;
			mCurrentResponse.rerror = ( mCurrentIn.range( 1 , 1 ) ).to_int( ) ;
			mCurrentResponse.rbe = ( mCurrentIn.range(9 ,2 ) ).to_int( ) ;
			break;
		case 4: DDB("resp 4");
			mCurrentResponse.slave_id = ( mCurrentIn.range( 15 , 0 ) ).to_int( ) ;
			mCurrentResponse.rcmd = ( mCurrentIn.range( 23 , 16 ) ).to_int( ) ;
			mCurrentResponse.n_bytes = ( mCurrentIn.range( 31 , 24 ) ).to_int( ) ;		
			break;
		case 5: DDB("resp 5");
			mCurrentResponse.rdata[0] = ( mCurrentIn.range( 7 , 0 ) ).to_int( ) ;
			mCurrentResponse.rdata[1] = ( mCurrentIn.range( 15 , 8 ) ).to_int( ) ;			
			mCurrentResponse.rdata[2] = ( mCurrentIn.range( 23 , 16 ) ).to_int( ) ;
			mCurrentResponse.rdata[3] = ( mCurrentIn.range( 31 , 24 ) ).to_int( ) ;
			break;
		case 6: DDB("resp 6");
			mCurrentResponse.rdata[4] = ( mCurrentIn.range( 7 , 0 ) ).to_int( ) ;
			mCurrentResponse.rdata[5] = ( mCurrentIn.range( 15 , 8 ) ).to_int( ) ;			
			mCurrentResponse.rdata[6] = ( mCurrentIn.range( 23 , 16 ) ).to_int( ) ;
			mCurrentResponse.rdata[7] = ( mCurrentIn.range( 31 , 24 ) ).to_int( ) ;
			break;
			case 7:DDB("tail flit - latency");	break;
		}
	} else { // request
		
		switch(mFlitCount){
		case 2: //DDB("reQ 2: ");
			mCurrentRequest.address = (uint32_t) ( mCurrentIn.range( 31 , 0 ) ).to_uint( ) ;
			// DDB("->address: " << hex <<mCurrentRequest.address);
			break;
		case 3: //DDB("reQ 3: ");
			mCurrentRequest.be =  mCurrentIn.range(7,0).to_uint( ) ;	
			mCurrentRequest.cmd = mCurrentIn.range(15,8).to_uint( ) ;
			mCurrentRequest.plen = (uint8_t) ( mCurrentIn.range( 23 ,16 ) ).to_int( ) ;
			mCurrentRequest.trdid = (uint8_t) ( mCurrentIn.range( 31 ,24 ) ).to_int( ) ;			
			break;

		case 4: // DDB("reQ 4");
			mCurrentRequest.contig = (bool) ( mCurrentIn.get_bit( 0 ) );
			mCurrentRequest.eop = (bool) ( mCurrentIn.range( 1 , 1 ) ).to_int( ) ;
			mCurrentRequest.cons = (bool) ( mCurrentIn.range( 2 , 2 ) ).to_int( ) ;					
			mCurrentRequest.wrap =  (bool)( mCurrentIn.range( 3 , 3 ) ).to_int( ) ;			
			mCurrentRequest.cfixed = (bool) ( mCurrentIn.range( 4 , 4 ) ).to_int( ) ;			
			mCurrentRequest.clen = (bool) ( mCurrentIn.range( 5 , 5 ) ).to_int( ) ;					
			mCurrentRequest.srcid = (uint16_t) ( mCurrentIn.range( 21 , 6 ) ).to_int( ) ;		
			mCurrentRequest.pktid = (uint8_t) ( mCurrentIn.range( 29 , 22 ) ).to_int( ) ;	
			break;

		case 5: // DDB("reQ 5");
			mCurrentRequest.initial_address = ( mCurrentIn.range( 31 , 0 ) ).to_int( ) ;
			break;
		
		case 6: // DDB("reQ 6");
			mCurrentRequest.slave_id = (uint16_t) mCurrentIn.range( 15 , 0 ).to_int( ) ;
			mCurrentRequest.n_bytes = (uint16_t) mCurrentIn.range( 31 , 16 ).to_int( ) ;	
			break;

		case 7: // DDB("reQ 7");
			mCurrentRequest.wdata[0] = ( mCurrentIn.range( 7 , 0 ) ).to_int( ) ;
			mCurrentRequest.wdata[1] = ( mCurrentIn.range( 15 , 8 ) ).to_int( ) ;			
			mCurrentRequest.wdata[2] = ( mCurrentIn.range( 23 , 16 ) ).to_int( ) ;
			mCurrentRequest.wdata[3] = ( mCurrentIn.range( 31 , 24 ) ).to_int( ) ;
			break;
			
		case 8: // DDB("reQ 8");
			mCurrentRequest.wdata[4] = ( mCurrentIn.range( 7 , 0 ) ).to_int( ) ;
			mCurrentRequest.wdata[5] = ( mCurrentIn.range( 15 , 8 ) ).to_int( ) ;			
			mCurrentRequest.wdata[6] = ( mCurrentIn.range( 23 , 16 ) ).to_int( ) ;
			mCurrentRequest.wdata[7] = ( mCurrentIn.range( 31 , 24 ) ).to_int( ) ;
			break;
		case 9: // DDB("tail flit - latency\n---------");	
		break;			
		}
	}
}
/******************************/
template< int FlitWidth >
void DestNI< FlitWidth >::reader(){
			// cout << "DDDD "<< mId<< endl;
	while(1){

		wait (1,SC_NS);
		
		if(mIsMaster == false)
			mCurrentRequest = m_queue_requests->Read();
		else
			mCurrentResponse = m_queue_responses->Read();

		// printf(">D%d(%s) rcv: %d-->%d pktid:%d\n",mId, this->name(), 
		// 	mCosiToRabbit_map[mCurrentRequest.srcid], mCosiToRabbit_map[mCurrentRequest.slave_id], mCurrentRequest.pktid);
	}
};
	/******************************/
template< int FlitWidth >
map< int , int > DestNI< FlitWidth >::GetReceivedBits( ) {
	DDB(__FUNCTION__);
	return mReceivedBits;
}


template< int FlitWidth >
void DestNI< FlitWidth >::Report( ) {

	//Report bandwidth
	cout << "D Bandwidth report for " << name( ) << endl ;
	map<int,int>::iterator It ;
	for( It = mReceivedBits.begin( ) ; It != mReceivedBits.end( ) ; It++ ) {
		cout << "\t Average bandwidth from source " << It->first << " = " << It->second /( sc_simulation_time( ) *1e-9 ) << endl ;
	}
	cout << "Delay report for " << name( ) << endl ;
	map<int,double>::iterator Lit ;
	for( Lit = mLatencies.begin( ) ; Lit != mLatencies.end( ) ; Lit++ ) {
		cout << "\t Average latency from source " << Lit->first << " = " << FlitWidth * Lit->second / ( double )mReceivedBits[Lit->first] << endl ;
	}

}
	/******************************/
template< int FlitWidth >
string DestNI< FlitWidth >::GetLatencies( ) {
	stringstream s ;
	map<int,double>::iterator Lit ;
	for( Lit = mLatencies.begin( ) ; Lit != mLatencies.end( ) ; Lit++ ) {
		s << FlitWidth * Lit->second / ( double )mReceivedBits[Lit->first] << endl ;
	}
	return s.str() ;
}
	/******************************/
template< int FlitWidth >
string DestNI< FlitWidth >::GetBandwidths( ) {
	stringstream s ;
	//Report bandwidth
	map<int,int>::iterator It ;
	for( It = mReceivedBits.begin( ) ; It != mReceivedBits.end( ) ; It++ ) {
		s  << It->second /( sc_simulation_time( ) *1e-9 ) << endl ;
	}

	return s ;
}
	/******************************/
template< int FlitWidth >
double  DestNI< FlitWidth >::GetAverageLatency( ) {

	map<int,double>::iterator Lit ;
	double Average = 0 ;
	for( Lit = mLatencies.begin( ) ; Lit != mLatencies.end( ) ; Lit++ ) {
		Average+= FlitWidth * Lit->second / ( double ) mReceivedBits[Lit->first]  ;
	}
	return Average/(double)( mLatencies.size( ) ) ;

}
	/******************************/
template< int FlitWidth >
double  DestNI< FlitWidth >::GetMinLatency( ) {

	map<int,double>::iterator Lit ;
	double Min = DBL_MAX ;
	for( Lit = mLatencies.begin( ) ; Lit != mLatencies.end( ) ; Lit++ ) {
		if ( FlitWidth * Lit->second / ( double ) mReceivedBits[Lit->first] < Min ) {
			Min = FlitWidth * Lit->second / ( double ) mReceivedBits[Lit->first]  ;
	  	}
	}
	return Min ;
}
	/******************************/
template< int FlitWidth >
double  DestNI< FlitWidth >::GetMaxLatency( ) {

	map<int,double>::iterator Lit ;
	double Max = 0 ;
	for( Lit = mLatencies.begin( ) ; Lit != mLatencies.end( ) ; Lit++ ) {
		if ( FlitWidth * Lit->second / ( double ) mReceivedBits[Lit->first] > Max ) {
	    	Max = FlitWidth * Lit->second / ( double ) mReceivedBits[Lit->first]  ;
	  	}
	}
	return Max ;
}

#endif /*DESTIP_H_*/
