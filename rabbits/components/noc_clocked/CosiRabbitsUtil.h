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
#ifndef COSISYSCUTIL_H_
#define COSISYSCUTIL_H_

#define FLITWIDTH 32
#define CONN_DEBUG 0

#define DEF_COSI_SOURCE_TO_IF_CONNECTION( s , d , inp , layer , length )  \
    PtP< FLITWIDTH > *S##s##_Sif##d##_in##inp; \
    sc_signal< sc_bv< FLITWIDTH > > *S##s##_Data ;				\
    sc_signal< bool > *S##s##_Valid ;					\
    sc_signal< bool > *S##s##_Ack ;					\
    sc_signal< bool > *S##s##_Full ;					\
    sc_signal< sc_bv< FLITWIDTH > > *Sif##d##_in##inp##_Data ;			\
    sc_signal< bool > *Sif##d##_in##inp##_Valid ;				\
    sc_signal< bool > *Sif##d##_in##inp##_Ack ;				\
    sc_signal< bool > *Sif##d##_in##inp##_Full ;				


#define RAB_COSI_SOURCE_TO_IF_CONNECTION( s , d , ss,  inp , layer , length ) if(CONN_DEBUG)cout << "Connecting source " << ss << " to interface " << d << endl ; \
    _cosiptpname_ << "S" << ss << "_Sif" << #d << "_in" << #inp ; \
    S##s##_Sif##d##_in##inp = new PtP< FLITWIDTH >( _cosiptpname_.str( ).c_str( ) ,  TechParam , layer , length ) ; \
    S##s##_Data = new sc_signal< sc_bv< FLITWIDTH > > ;		\
    S##s##_Valid = new sc_signal< bool > ;					\
    S##s##_Ack = new sc_signal< bool > ;					\
    S##s##_Full = new sc_signal< bool > ;				\
    m_src[ss]->DataOut( (*S##s##_Data) ) ;		\
    m_src[ss]->ValidOut( (*S##s##_Valid) ) ;		\
    m_src[ss]->AckIn( (*S##s##_Ack) ) ;						\
    m_src[ss]->FullIn( (*S##s##_Full) ) ;					\
    S##s##_Sif##d##_in##inp->DataIn( (*S##s##_Data) ) ;	\
    S##s##_Sif##d##_in##inp->ValidIn( (*S##s##_Valid) ) ;			\
    S##s##_Sif##d##_in##inp->AckOut( (*S##s##_Ack) ) ;				\
    S##s##_Sif##d##_in##inp->FullOut( (*S##s##_Full) ) ;				\
    Sif##d##_in##inp##_Data=new sc_signal< sc_bv< FLITWIDTH > >  ;			\
    Sif##d##_in##inp##_Valid = new sc_signal< bool > ;					\
    Sif##d##_in##inp##_Ack = new sc_signal< bool > ;				\
    Sif##d##_in##inp##_Full = new sc_signal< bool > ;				\
    Sif##d->DataIn[ inp ]( (*Sif##d##_in##inp##_Data) ) ;				\
    Sif##d->ValidIn[ inp ]( (*Sif##d##_in##inp##_Valid) ) ;				\
    Sif##d->AckOut[ inp ]( (*Sif##d##_in##inp##_Ack) ) ;				\
    Sif##d->FullOut[ inp ]( (*Sif##d##_in##inp##_Full) ) ;				\
    S##s##_Sif##d##_in##inp->DataOut( (*Sif##d##_in##inp##_Data) ) ;		\
    S##s##_Sif##d##_in##inp->ValidOut( (*Sif##d##_in##inp##_Valid) ) ;	\
    S##s##_Sif##d##_in##inp->AckIn( (*Sif##d##_in##inp##_Ack )) ;		\
    S##s##_Sif##d##_in##inp->FullIn( (*Sif##d##_in##inp##_Full )) ;

#define DEF_COSI_IF_TO_DEST_CONNECTION( s , outp , d  , layer , length )  \
    PtP< FLITWIDTH > *Dif##s##_out##outp##_D##d; \
    sc_signal< sc_bv< FLITWIDTH > >* Dif##s##_out##outp##_Data ;		\
    sc_signal< bool > *Dif##s##_out##outp##_Valid ;				\
    sc_signal< bool >* Dif##s##_out##outp##_Ack ;				\
    sc_signal< bool >* Dif##s##_out##outp##_Full ;				\
    sc_signal< sc_bv< FLITWIDTH > > *D##d##_Data ;			\
    sc_signal< bool > *D##d##_Valid ;					\
    sc_signal< bool >* D##d##_Ack ;					\
    sc_signal< bool > *D##d##_Full ;					

#define RAB_COSI_IF_TO_DEST_CONNECTION( s , outp , d , ds , layer , length ) if(CONN_DEBUG)cout << "Connecting interface " << s << " to destination " << ds << endl ; \
    _cosiptpname_ << "Dif" << #s <<  "_out" << #outp << "_D"  << ds; \
    Dif##s##_out##outp##_D##d=new PtP< FLITWIDTH >( _cosiptpname_.str().c_str( )  ,  TechParam , layer , length ) ; \
    Dif##s##_out##outp##_Data=new sc_signal< sc_bv< FLITWIDTH > >  ;		\
    Dif##s##_out##outp##_Valid = new sc_signal< bool >  ;				\
    Dif##s##_out##outp##_Ack  = new sc_signal< bool >;				\
    Dif##s##_out##outp##_Full = new sc_signal< bool > ;				\
    Dif##s->DataOut[ outp ]( (*Dif##s##_out##outp##_Data) ) ;			\
    Dif##s->ValidOut[ outp ]( (*Dif##s##_out##outp##_Valid) ) ;			\
    Dif##s->AckIn[ outp ]( (*Dif##s##_out##outp##_Ack )) ;			\
    Dif##s->FullIn[ outp ]( (*Dif##s##_out##outp##_Full) ) ;			\
    Dif##s##_out##outp##_D##d->DataIn( (*Dif##s##_out##outp##_Data) ) ;		\
    Dif##s##_out##outp##_D##d->ValidIn( (*Dif##s##_out##outp##_Valid) ) ;	\
    Dif##s##_out##outp##_D##d->AckOut( (*Dif##s##_out##outp##_Ack )) ;		\
    Dif##s##_out##outp##_D##d->FullOut( (*Dif##s##_out##outp##_Full) ) ;	\
    D##d##_Data = new sc_signal< sc_bv< FLITWIDTH > >  ;			\
    D##d##_Valid  = new sc_signal< bool >;					\
    D##d##_Ack = new sc_signal< bool > ;					\
    D##d##_Full = new sc_signal< bool > ;					\
    m_dst[ds]->DataIn( (*D##d##_Data) ) ;					\
    m_dst[ds]->ValidIn((* D##d##_Valid) ) ;					\
    m_dst[ds]->AckOut( (*D##d##_Ack )) ;						\
    m_dst[ds]->FullOut( (*D##d##_Full) ) ;					\
    Dif##s##_out##outp##_D##d->DataOut( (*D##d##_Data) ) ;			\
    Dif##s##_out##outp##_D##d->ValidOut( (*D##d##_Valid )) ;			\
    Dif##s##_out##outp##_D##d->AckIn( (*D##d##_Ack )) ;			\
    Dif##s##_out##outp##_D##d->FullIn( (*D##d##_Full) ) ;



#define DEF_COSI_ROUTER_TO_ROUTER_CONNECTION( s , outp , d , inp , layer , length )  \
    PtP< FLITWIDTH > *R##s##_out##outp##_R##d##_in##inp; \
    sc_signal< sc_bv< FLITWIDTH > > *R##s##_out##outp##_Data ;		\
    sc_signal< bool >* R##s##_out##outp##_Valid ;			\
    sc_signal< bool >* R##s##_out##outp##_Ack ;				\
    sc_signal< bool >* R##s##_out##outp##_Full ;				\
    sc_signal< sc_bv< FLITWIDTH > > *R##d##_in##inp##_Data ;		\
    sc_signal< bool > *R##d##_in##inp##_Valid ;				\
    sc_signal< bool >* R##d##_in##inp##_Ack ;				\
    sc_signal< bool >* R##d##_in##inp##_Full ;				


#define RAB_COSI_ROUTER_TO_ROUTER_CONNECTION( s , outp , d , inp , layer , length ) if(CONN_DEBUG)cout << "Connecting router " << s << " to router " << d << endl ; \
    _cosiptpname_ << "R" << #s << "_out" << #outp << "_R" << #d << "_in" << #inp ; \
    R##s##_out##outp##_R##d##_in##inp=new PtP< FLITWIDTH >( _cosiptpname_.str().c_str( ) ,  TechParam , layer , length ) ; \
    R##s##_out##outp##_Data = new sc_signal< sc_bv< FLITWIDTH > > ;		\
    R##s##_out##outp##_Valid = new sc_signal< bool > ;			\
    R##s##_out##outp##_Ack  = new sc_signal< bool > ;			\
    R##s##_out##outp##_Full = new sc_signal< bool > ;			\
    R##s->DataOut[ outp ]( (*R##s##_out##outp##_Data) ) ;			\
    R##s->ValidOut[ outp ]( (*R##s##_out##outp##_Valid) ) ;			\
    R##s->AckIn[ outp ]( (*R##s##_out##outp##_Ack) ) ;			\
    R##s->FullIn[ outp ]( (*R##s##_out##outp##_Full) ) ;			\
    R##s##_out##outp##_R##d##_in##inp->DataIn( (*R##s##_out##outp##_Data) ) ; \
    R##s##_out##outp##_R##d##_in##inp->ValidIn( (*R##s##_out##outp##_Valid) ) ; \
    R##s##_out##outp##_R##d##_in##inp->AckOut( (*R##s##_out##outp##_Ack )) ; \
    R##s##_out##outp##_R##d##_in##inp->FullOut( (*R##s##_out##outp##_Full) ) ; \
    R##d##_in##inp##_Data = new sc_signal< sc_bv< FLITWIDTH > >  ;		\
    R##d##_in##inp##_Valid = new sc_signal< bool >  ;				\
    R##d##_in##inp##_Ack = new sc_signal< bool >  ;				\
    R##d##_in##inp##_Full  = new sc_signal< bool >  ;				\
    R##d->DataIn[ inp ]( (*R##d##_in##inp##_Data) ) ;			\
    R##d->ValidIn[ inp ]( (*R##d##_in##inp##_Valid) ) ;			\
    R##d->AckOut[ inp ]( (*R##d##_in##inp##_Ack )) ;			\
    R##d->FullOut[ inp ]( (*R##d##_in##inp##_Full) ) ;			\
    R##s##_out##outp##_R##d##_in##inp->DataOut( (*R##d##_in##inp##_Data) ) ; \
    R##s##_out##outp##_R##d##_in##inp->ValidOut((* R##d##_in##inp##_Valid) ) ; \
    R##s##_out##outp##_R##d##_in##inp->AckIn( (*R##d##_in##inp##_Ack )) ;	\
    R##s##_out##outp##_R##d##_in##inp->FullIn( (*R##d##_in##inp##_Full) ) ;


#define DEF_COSI_IF_TO_ROUTER_CONNECTION( s , outp , d , inp , layer , length ) \
    PtP< FLITWIDTH > *Sif##s##_out##outp##_R##d##_in##inp; \
    sc_signal< sc_bv< FLITWIDTH > > *Sif##s##_out##outp##_Data ;		\
    sc_signal< bool >* Sif##s##_out##outp##_Valid ;			\
    sc_signal< bool >* Sif##s##_out##outp##_Ack ;				\
    sc_signal< bool >* Sif##s##_out##outp##_Full ;				\
    sc_signal< sc_bv< FLITWIDTH > > *R##d##_in##inp##_Data ;		\
    sc_signal< bool > *R##d##_in##inp##_Valid ;				\
    sc_signal< bool >* R##d##_in##inp##_Ack ;				\
    sc_signal< bool >* R##d##_in##inp##_Full ;				

#define RAB_COSI_IF_TO_ROUTER_CONNECTION( s , outp , d , inp , layer , length ) \
    _cosiptpname_ << "Sif" << #s << "_out" << #outp << "_R" << #d << "_in" << #inp ; \
    Sif##s##_out##outp##_R##d##_in##inp = new PtP< FLITWIDTH >( _cosiptpname_.str().c_str( ) ,  TechParam , layer , length ) ; \
    Sif##s##_out##outp##_Data =new sc_signal< sc_bv< FLITWIDTH > > ;		\
    Sif##s##_out##outp##_Valid = new sc_signal< bool > ;			\
    Sif##s##_out##outp##_Ack= new sc_signal< bool > ;			\
    Sif##s##_out##outp##_Full= new sc_signal< bool > ;			\
    Sif##s->DataOut[ outp ]((*Sif##s##_out##outp##_Data )) ;			\
    Sif##s->ValidOut[ outp ]((* Sif##s##_out##outp##_Valid )) ;			\
    Sif##s->AckIn[ outp ]((*Sif##s##_out##outp##_Ack) ) ;			\
    Sif##s->FullIn[ outp ]((*Sif##s##_out##outp##_Full )) ;			\
    Sif##s##_out##outp##_R##d##_in##inp->DataIn((*Sif##s##_out##outp##_Data) ) ; \
    Sif##s##_out##outp##_R##d##_in##inp->ValidIn((* Sif##s##_out##outp##_Valid )) ; \
    Sif##s##_out##outp##_R##d##_in##inp->AckOut((*Sif##s##_out##outp##_Ack )) ; \
    Sif##s##_out##outp##_R##d##_in##inp->FullOut((* Sif##s##_out##outp##_Full) ) ; \
    R##d##_in##inp##_Data = new sc_signal< sc_bv< FLITWIDTH > >;		\
    R##d##_in##inp##_Valid = new sc_signal< bool > ;				\
    R##d##_in##inp##_Ack = new sc_signal< bool >  ;				\
    R##d##_in##inp##_Full = new sc_signal< bool >  ;				\
    R##d->DataIn[ inp ]( (*R##d##_in##inp##_Data )) ;			\
    R##d->ValidIn[ inp ]( (*R##d##_in##inp##_Valid) ) ;			\
    R##d->AckOut[ inp ]( (*R##d##_in##inp##_Ack )) ;			\
    R##d->FullOut[ inp ]( (*R##d##_in##inp##_Full )) ;			\
    Sif##s##_out##outp##_R##d##_in##inp->DataOut((* R##d##_in##inp##_Data) ) ; \
    Sif##s##_out##outp##_R##d##_in##inp->ValidOut( (*R##d##_in##inp##_Valid) ) ; \
    Sif##s##_out##outp##_R##d##_in##inp->AckIn( (*R##d##_in##inp##_Ack )) ;	\
    Sif##s##_out##outp##_R##d##_in##inp->FullIn( (* R##d##_in##inp##_Full )) ;


#define DEF_COSI_ROUTER_TO_IF_CONNECTION( s , outp , d , inp , layer , length )  \
    PtP< FLITWIDTH > *R##s##_out##outp##_Dif##d##_in##inp; \
    sc_signal< sc_bv< FLITWIDTH > > *R##s##_out##outp##_Data ;		\
    sc_signal< bool >* R##s##_out##outp##_Valid ;			\
    sc_signal< bool >* R##s##_out##outp##_Ack ;				\
    sc_signal< bool >* R##s##_out##outp##_Full ;				\
    sc_signal< sc_bv< FLITWIDTH > > *Dif##d##_in##inp##_Data ;		\
    sc_signal< bool > *Dif##d##_in##inp##_Valid ;				\
    sc_signal< bool > *Dif##d##_in##inp##_Ack ;				\
    sc_signal< bool > *Dif##d##_in##inp##_Full ;				

#define RAB_COSI_ROUTER_TO_IF_CONNECTION( s , outp , d , inp , layer , length ) if(CONN_DEBUG)cout << "Connecting router " << s << " to interface " << d << endl ; \
    _cosiptpname_ << "R" << #s << "_out" << #outp << "_Dif" << #d << "_in" << #inp ; \
    R##s##_out##outp##_Dif##d##_in##inp=new PtP< FLITWIDTH > ( _cosiptpname_.str().c_str( ) ,  TechParam , layer , length ) ; \
    R##s##_out##outp##_Data=new sc_signal< sc_bv< FLITWIDTH > > ;		\
    R##s##_out##outp##_Valid= new sc_signal< bool >  ;			\
    R##s##_out##outp##_Ack = new sc_signal< bool > ;				\
    R##s##_out##outp##_Full= new sc_signal< bool >  ;				\
    R##s->DataOut[ outp ]( (*R##s##_out##outp##_Data) ) ;			\
    R##s->ValidOut[ outp ]( (*R##s##_out##outp##_Valid) ) ;			\
    R##s->AckIn[ outp ]( (*R##s##_out##outp##_Ack) ) ;			\
    R##s->FullIn[ outp ]( (*R##s##_out##outp##_Full) ) ;			\
    R##s##_out##outp##_Dif##d##_in##inp->DataIn( (*R##s##_out##outp##_Data) ) ; \
    R##s##_out##outp##_Dif##d##_in##inp->ValidIn( (*R##s##_out##outp##_Valid) ) ; \
    R##s##_out##outp##_Dif##d##_in##inp->AckOut( (*R##s##_out##outp##_Ack) ) ; \
    R##s##_out##outp##_Dif##d##_in##inp->FullOut( (*R##s##_out##outp##_Full) ) ; \
    Dif##d##_in##inp##_Data = new sc_signal< sc_bv< FLITWIDTH > >  ;		\
    Dif##d##_in##inp##_Valid=new sc_signal< bool >    ;				\
    Dif##d##_in##inp##_Ack =new sc_signal< bool > ;				\
    Dif##d##_in##inp##_Full=new sc_signal< bool >  ;				\
    Dif##d->DataIn[ inp ]( (*Dif##d##_in##inp##_Data) ) ;			\
    Dif##d->ValidIn[ inp ]( (*Dif##d##_in##inp##_Valid) ) ;			\
    Dif##d->AckOut[ inp ]( (*Dif##d##_in##inp##_Ack) ) ;			\
    Dif##d->FullOut[ inp ]((* Dif##d##_in##inp##_Full) ) ;			\
    R##s##_out##outp##_Dif##d##_in##inp->DataOut( (*Dif##d##_in##inp##_Data) ) ; \
    R##s##_out##outp##_Dif##d##_in##inp->ValidOut( (*Dif##d##_in##inp##_Valid) ) ; \
    R##s##_out##outp##_Dif##d##_in##inp->AckIn( (*Dif##d##_in##inp##_Ack )) ;	\
    R##s##_out##outp##_Dif##d##_in##inp->FullIn( (*Dif##d##_in##inp##_Full) ) ;


// #define RAB_COSI_IF_TO_DEST_CONNECTION( s , outp , d , ds , layer , length ) if(CONN_DEBUG)cout << "Connecting interface " << s << " to destination " << ds << endl ; 
    // _cosiptpname_ << "Dif" << #s <<  "_out" << #outp << "_D"  << #ds; \
    // Dif##s##_out##outp##_D##d=new PtP< FLITWIDTH >( _cosiptpname_.str().c_str( )  ,  TechParam , layer , length ) ; \
    // Dif##s##_out##outp##_Data=new sc_signal< sc_bv< FLITWIDTH > >  ;		\
    // Dif##s##_out##outp##_Valid = new sc_signal< bool >  ;				\
    // Dif##s##_out##outp##_Ack  = new sc_signal< bool >;				\
    // Dif##s##_out##outp##_Full = new sc_signal< bool > ;				\
    // Dif##s->DataOut[ outp ]( (*Dif##s##_out##outp##_Data) ) ;			\
    // Dif##s->ValidOut[ outp ]( (*Dif##s##_out##outp##_Valid) ) ;			\
    // Dif##s->AckIn[ outp ]( (*Dif##s##_out##outp##_Ack )) ;			\
    // Dif##s->FullIn[ outp ]( (*Dif##s##_out##outp##_Full) ) ;			\
    // Dif##s##_out##outp##_D##d->DataIn( (*Dif##s##_out##outp##_Data) ) ;		\
    // Dif##s##_out##outp##_D##d->ValidIn( (*Dif##s##_out##outp##_Valid) ) ;	\
    // Dif##s##_out##outp##_D##d->AckOut( (*Dif##s##_out##outp##_Ack )) ;		\
    // Dif##s##_out##outp##_D##d->FullOut( (*Dif##s##_out##outp##_Full) ) ;	\
    // D##d##_Data = new sc_signal< sc_bv< FLITWIDTH > >  ;			\
    // D##d##_Valid  = new sc_signal< bool >;					\
    // D##d##_Ack = new sc_signal< bool > ;					\
    // D##d##_Full = new sc_signal< bool > ;					\
    // m_dst[ds]->DataIn( (*D##d##_Data) ) ;					\
    // m_dst[ds]->ValidIn((* D##d##_Valid) ) ;					\
    // m_dst[ds]->AckOut( (*D##d##_Ack )) ;						\
    // m_dst[ds]->FullOut( (*D##d##_Full) ) ;					\
    // Dif##s##_out##outp##_D##d->DataOut( (*D##d##_Data) ) ;			\
    // Dif##s##_out##outp##_D##d->ValidOut( (*D##d##_Valid )) ;			\
    // Dif##s##_out##outp##_D##d->AckIn( (*D##d##_Ack )) ;			\
    // Dif##s##_out##outp##_D##d->FullIn( (*D##d##_Full) ) ;


// void _CosiAddDestIp_( DestIp<FLITWIDTH>* D ) {
//   _CosiDestIpVector_.push_back( D ) ;
// }

#endif
// mSpec->GetName(*U)