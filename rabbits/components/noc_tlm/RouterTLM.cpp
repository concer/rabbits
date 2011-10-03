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
 *
 * @param[in] _n          name of the router module
 * @param[in] clk_period  one clock cycle delay represented by a sc_time object
 */
template <int Nin, int Nout, int QDepth>
RouterTLM<Nin, Nout, QDepth>::RouterTLM (sc_module_name _n, sc_time clk_period, map< pair<int,int>, int > routing_table)
  : sc_module( _n )
{
    for(int i=0; i<Nin; i++) { 
      ((input_port[i])(fifo_input_port[i]));
      //fifo_input_port[i].nb_expand(QDepth);
      fifo_input_port[i].nb_bound(QDepth);
    }
   
    this->clk_period = clk_period;
    this->routing_table = routing_table;

    for(int i=0; i<Nout; i++) {
      output_arbiter[i].setClk(clk_period);
    }
    SC_THREAD( process_main ) ;
}

/**
 * Main function that spawns sub-threads per input port.
 *
 * @warning The current implementation uses sc_spawn(), which is clearly not synthesizable.
 * 
 */
template <int Nin, int Nout, int QDepth>
void RouterTLM<Nin, Nout, QDepth>::process_main()
{
  for(int i=0; i<Nin; i++) {
    arbiter_handler[i] = sc_spawn(sc_bind(&(RouterTLM<Nin, Nout, QDepth>::process_per_input), this, i));
  }
  //Arbiter main dies after spawning procoess_per_input() per input port.
}

/**
 * sub-thread function that gets the flit from an input port, and deals with the
 * output arbitration. This implements 1 clock cycle wormhole flow-control router 
 *
 * @param[in]   ip    input port identifier for the sub-thread
 * 
 * @note The implementation of finding output port is in get_output_port() function.
 * This function simply called get_output_port to find the correct output.
 */
template <int Nin, int Nout, int QDepth>
void RouterTLM<Nin, Nout, QDepth>::process_per_input(int ip)
{
  FlitTLM msg;
  int op;

  while(true) {
    msg = fifo_input_port[ip].get();      // Get the head flit of the first packet.
    std::cout << "[" << name() << "/" << sc_time_stamp() << "]: Received a flit " << msg << " from input port " << ip <<"." << std::endl;
    op = get_output_port(msg.srcID, msg.destID); // Get the output port based on destID, we may want to automate this.
    
    assert(msg.flitID == 0);  //current message should be the header flit of a transaction
    std::cout << "[" << name() << "/" << sc_time_stamp() << "]: Request (" << ip << " ==> " << op <<") requested." << std::endl;
    output_arbiter[op].lock(); // Try to request the output port.
    std::cout << "[" << name() << "/" << sc_time_stamp() << "]: Request (" << ip << " ==> " << op <<") granted." << std::endl;
    output_port[op]->put( msg );
    std::cout << "[" << name() << "/" << sc_time_stamp() << "]: Transfering the HEAD flit "<< msg << " from input port " << ip << " to output port " << op << "." << std::endl;
    while (msg.isTail == false) {
      wait( clk_period );
      msg = fifo_input_port[ip].get();
      std::cout << "[" << name() << "/" << sc_time_stamp() << "]: Received a flit " << msg << " from input port " << ip <<"." << std::endl;
      output_port[op]->put( msg );
      std::cout << "[" << name() << "/" << sc_time_stamp() << "]: Transfering flit "<< msg << " from input port " << ip << " to output port " << op << "." << std::endl;
    } while (msg.isTail == false);
    
    output_arbiter[op].unlock(); 
    std::cout << "[" << name() << "/" << sc_time_stamp() << "]: Request (" << ip << " ==> " << op <<") released." << std::endl;
  }
}

/**
 * inline function that returns output with given source and destination identifiers.
 *
 * @param[in]   srcID     source identifier
 * @param[out]  destID    destination identifier
 * @return                the correct output port identifier
 * 
 * @note the current implementation is only based on destID, and we may want to automate 
 * this function. 
 */
template <int Nin, int Nout, int QDepth>
inline int RouterTLM<Nin, Nout, QDepth>::get_output_port(int srcID, int destID)
{
  assert(routing_table.find(pair<int,int>(srcID,destID)) != routing_table.end());
  return routing_table.find(pair<int,int>(srcID,destID))->second;
}
