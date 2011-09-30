/**
 * @filename  RouterTLM.cpp
 *
 * @brief     The file that includes TLM model of SystemC router implementation.
 *
 * @author    Nicola Concer
 *
 * @note      This file will be included in RouterTLM.h
 * 
 * @sa        RouterTLM.h
 */


/**
 * Constructor for the class
 */
template <int Nin, int Nout, int QDepth>
RouterTLM<Nin, Nout, QDepth>::RouterTLM (sc_module_name _n)
  : sc_module( _n )
{
    for(int i=0; i<Nout; i++){ 
      ((input_port[i])(fifo_input_port[i]));
      fifo_input_port[i].nb_expand(QDepth);
    }
    SC_THREAD( arbiter ) ;
}

/**
 * Arbiter function that takes flits from the input and
 * forwards them on the output.
 *
 * @warning The current implementation does not correctly route 
 * packets to the destination.
 * 
 * @todo implement the arbiter function
 */
template <int Nin, int Nout, int QDepth>
void RouterTLM<Nin, Nout, QDepth>::arbiter()
{
	int i=0;
	FlitTLM msg;
	while(true) {	
		wait(1,SC_NS);
		
		for(i=0; i<Nin;i++){
			msg = fifo_input_port[i].get();
			cout << this->name() << ": " << msg <<endl;
      		output_port[i]->put( msg );
  		}
	}
};

