/**
 * @filename  sc_clked_mutex.h
 *
 * @brief     The file that includes the extended version of sc_mutex primitive
 *            channel class to support delayed (or clocked) mutex.
 *
 * @author    Young Jin Yoon <youngjin@cs.columbia.edu>
 *
 * @note      The following code inspired by and derived from the sc_mutex 
 *            implementation of the SystemC source code.
 * 
 * @sa        sc_mutex.h
 */


#ifndef SC_CLKED_MUTEX_H
#define SC_CLKED_MUTEX_H

#include <vector>
#include <map>

#include "systemc.h"

namespace sc_ext {

using namespace sc_core;

/**
 * provides mutual exclusion of resource access, but with some amount of delay 
 * and arbitration algorithm. The class first collects requests access by waiting
 * some amounts of sc_time, and use its own arbitration algorithm to grant one
 * request.
 *
 * @note: Since this class is not derived from sc_module, there is no description
 * about inputs and outputs as we typically do for the sc_module-derived classes.
 */
class sc_clked_mutex
: public sc_mutex 
{


  public:

    // constructors and destructor
    explicit sc_clked_mutex( const char* name_=sc_gen_unique_name( "sc_clked_mutex"), sc_time clk = sc_time(1, SC_PS) );
    virtual ~sc_clked_mutex();

    // sc_mutex_if-derived functions (interfaces)
    virtual int lock();
    virtual int trylock();
    virtual int unlock();

    /** returns the character string of the class name. */
    virtual const char* kind() const
        { return "sc_clked_mutex"; }
    
    /** adjust delay for collecting requests. */
    virtual void setClk(sc_time clk) 
        { m_clk = clk; }

  protected:

    /** returns whether the resource is in use */
    bool in_use() const
        { return ( m_owner != 0 ); }

    void register_process(sc_process_b* curProc);
    void yield_master();


    sc_process_b* m_owner;    //!< indicates the owner of the resource
    sc_event      m_free;     //!< notifies the resource become free
    
    std::map<sc_process_b*, int> m_process_map;       //!< table from registered process to internal id
    std::vector<sc_process_b*> m_id_map;              //!< table from internal id to registered process
    std::map<sc_process_b*, sc_event *> m_granted;    //!< table of sc_events that notifies the request of the process is finally granted
    std::vector<bool> m_requested;                    //!< table of boolean that indicates whether the corresponding process requested the resource access or not.
    
    sc_process_b* m_master;     //!< indicates the process that executes arbitration algorithm. 
    sc_time       m_clk;        //!< delay for collecting requests
    sc_event      m_clk_event;  //!< notifies that the delay is over i.e. one clock cycle has passed.


protected:    
    // For arbitration
    virtual void arbitrate();    
    int           m_last_granted; //!< internal id that represents the last granted process

private:

    // disabled
    //sc_clked_mutex( const sc_clked_mutex& );
    //sc_clked_mutex& operator = ( const sc_clked_mutex& );
};


} // namespace sc_ext

#endif
