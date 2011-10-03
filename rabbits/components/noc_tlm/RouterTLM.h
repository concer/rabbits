/**
 * @filename RouterTLM.h
 *
 * @brief The file that includes TLM model of SystemC router implementation.
 *
 * @author Nicola Concer
 */

#ifndef __ROUTER_TLM_H__
#define __ROUTER_TLM_H__

//#define SC_INCLUDE_DYNAMIC_PROCESSES // In order to use sc_spawn() function calls.
#include <map>

#include <systemc>
#include <tlm.h>

#include "FlitTLM.h"
#include "sc_clked_mutex.h"

using namespace tlm;
using namespace sc_core;
using namespace sc_ext;
using namespace std;
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

    sc_export< tlm_blocking_put_if<FlitTLM> > input_port[Nin];
    sc_port< tlm_blocking_put_if<FlitTLM> > output_port[Nout];

    /// Constructor
    SC_HAS_PROCESS(RouterTLM);
    RouterTLM(sc_module_name _n = sc_gen_unique_name("RouterTLM"), sc_time clk_period = sc_time(1, SC_NS), map< pair<int,int>, int > routing_table = empty_routing_table());
    /// destructor
    ~RouterTLM() {};

  
  private:
    void process_main();
    void process_per_input(int ip);
    int get_output_port(int srcID, int destID);
    
    /** placeholder for the routing table */
    static map< pair<int,int>, int > empty_routing_table() {
      map< pair<int,int>, int > retVal;
      return retVal; 
    }

    tlm_fifo<FlitTLM> fifo_input_port[Nin];
    sc_clked_mutex    output_arbiter[Nout];
    sc_process_handle arbiter_handler[Nin];
    sc_time clk_period;
    map< pair<int,int>, int > routing_table;
};

#include "RouterTLM.cpp"

#endif
