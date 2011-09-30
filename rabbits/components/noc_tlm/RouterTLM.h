/**
 * @filename RouterTLM.h
 *
 * @brief The file that includes TLM model of SystemC router implementation.
 *
 * @author Nicola Concer
 */

#ifndef __ROUTER_TLM_H__
#define __ROUTER_TLM_H__

#include "systemc.h"
#include <tlm.h>

#include "FlitTLM.h"

using namespace tlm;

/**
 * @brief systemC-TLM router with variable number of input and 
 * output ports. The input port uses an exported tlm_fifo.
 *
 * @tparam      Nin           Total number of input ports
 * @tparam      Nout          Total number of output ports
 * @tparam      QDepth        Depth of input queue depth
 * 
 * @param[in]   input_port    Input ports from routers or network interfaces
 * @param[out]  output_port   Output ports to routers or network interfaces
 */
template <int Nin, int Nout, int QDepth = 4>
class RouterTLM : public sc_module { 

  public:
	SC_HAS_PROCESS (RouterTLM);

	sc_export< tlm_blocking_put_if<FlitTLM> > input_port[Nin];
	sc_port< tlm_blocking_put_if<FlitTLM> > output_port[Nout];

	/// Constructor
	RouterTLM(sc_module_name _n = sc_gen_unique_name("RouterTLM"));
	/// destructor
	~RouterTLM() {};
	
  private:
	void arbiter();
	tlm_fifo<FlitTLM> fifo_input_port[Nin];
};

#include "RouterTLM.cpp"

#endif
