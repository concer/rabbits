/**
 * @filename  sc_clked_mutex.cpp
 *
 * @brief     The file that includes the extended version of sc_mutex primitive
 *            channel class to support delayed (or clocked) mutex.
 *
 * @author    Young Jin Yoon <youngjin@cs.columbia.edu>
 *
 * @note      The following code inspired by and derived from the sc_mutex 
 *            implementation of the SystemC source code.
 * 
 * @sa        sc_clked_mutex.h
 * @sa        sc_mutex.h
 */

#include "sc_clked_mutex.h"
#include "sysc/kernel/sc_simcontext.h"

namespace sc_ext {

/**
 * constructor
 *
 * @param[in] name_ name for the mutex
 * @param[in] clk   delay to collect all requests for the resource
 */
sc_clked_mutex::sc_clked_mutex( const char* name_ , sc_time clk )
: sc_mutex( name_ ),
  m_last_granted( -1 ),
  m_owner( 0 ),
  m_master( 0 ),
  m_clk ( clk )
{
  m_process_map.clear();
  m_id_map.clear();
  m_granted.clear();
  m_requested.clear();
}


/**
 * destructor
 */
sc_clked_mutex::~sc_clked_mutex()
{
  std::vector<sc_process_b*>::iterator it;
  for(it = m_id_map.begin(); it != m_id_map.end(); it++) {
    delete m_granted[*it];
  }
}


/**
 * claim an exclusive access of the resource. This function blocks the
 * process until mutex could be locked.
 *
 * @return 0 for success
 */
int
sc_clked_mutex::lock()
{
    sc_process_b* curProc = sc_get_current_process_b();
    // Register the process in m_process_map
    if(m_process_map.find(curProc) == m_process_map.end())
      register_process(curProc);
    
    // Enable the request signal
    m_requested[m_process_map[curProc]] = true;
 
    // Register the current process as a master
    if(m_master == 0)
      m_master = curProc;

    // Repeat until the current process get the grant
    do {
#ifdef DEBUG
      std::cout << "[DEBUG]: beginning of do while loop in process " << std::hex << curProc << std::endl;
#endif
      // wait for the lock to be free
      while( in_use() ) {
#ifdef DEBUG
        std::cout << "[DEBUG]: wait for m_free signal in process " << std::hex << curProc << std::endl;
#endif
	wait( m_free );
      }
      // if the current process is the master, call arbitrate, otherwise wait for the grant
      if( m_master == curProc ) {
#ifdef DEBUG
        std::cout << "[DEBUG]: wait for " << m_clk << " in process " << std::hex << curProc << std::endl;
#endif
        wait ( m_clk );
        arbitrate();              //If the current processor is not the one accepted, 
                                  // master will be changed into the accepted one.
        m_clk_event.notify();
      } else {
#ifdef DEBUG
        std::cout << "[DEBUG]: wait for m_granted signal in process " << std::hex << curProc << std::endl;
#endif
        wait( *m_granted[curProc] );
      }
#ifdef DEBUG
      std::cout << "[DEBUG]: end of do while loop in process " << std::hex << curProc << std::endl;
#endif
    } while (m_owner != curProc);

    return 0;
}



/**
 * claim an exclusive access of the resource. This function does not block 
 * the process until mutex could be locked.
 *
 * @return 0 for success, -1 for failure of locking the resource.
 *
 * @note Although this function does not block the process, it still delays the process about
 * a clock cycle.
 */
int
sc_clked_mutex::trylock()
{
    sc_process_b* curProc = sc_get_current_process_b();
    // Register the process in m_process_map
    if(m_process_map.find(curProc) == m_process_map.end())
      register_process(curProc);
    
  
    // if the lock is not free, simply return -1;
    if( in_use() ) {
	return -1;
    } 
    
    // Now we know that the lock is free, so enable the request signal
    // to wait for a clock cycle.
    m_requested[m_process_map[curProc]] = true;
    
    // if the lock is not in use and there is no master, register 
    // the current process as a master to wait for a clock cycle.
    if(m_master == 0)
      m_master = curProc;
    
    if ( m_master == curProc ) {
      wait( m_clk );
      arbitrate();              //If the current processor is not the one accepted, 
                                // master will be changed into the accepted one.
      m_clk_event.notify();
    } else {
      wait( m_clk_event );
    }

    // No matter the current process succeeded or not, 
    // it will not attempt to send the request for the next clock cycle,
    // i.e. it tries for a clock cycle.
    m_requested[m_process_map[curProc]] = false;

    if (m_owner != curProc)
      return -1;
    else
      return 0;
}

/**
 * release the claimed resource. 
 *
 * @return 0 for success, -1 if mutex was not locked by the caller.
 */
int
sc_clked_mutex::unlock()
{
    sc_process_b* curProc = sc_get_current_process_b();
    //if mutex was not locked by the caller, return -1
    if( m_owner != curProc ) {
	return -1;
    }
    
    //m_owner should be m_master, and should be the current process.
    sc_assert((m_owner == m_master) && (m_master == curProc));

    //Turn off the request, and yield master to another process.
    m_requested[m_process_map[curProc]] = false;
    yield_master();
   
    //Make sure there is no owner, and try to wake up the new master after yielding it.
    m_owner = 0;
    if(m_master != 0)
      m_granted[m_master]->notify(); //wake up the new master
    
    //sc_event that states "now the resource is free."
    m_free.notify();

    return 0;
}

/**
 * register the process which uses this mutex in order to provide "fair" access.
 *
 * @param[in] curProc process handler with sc_process_b class to register.
 */
inline void
sc_clked_mutex::register_process(sc_process_b * curProc)
{
#ifdef DEBUG
      std::cout << "[DEBUG]: register process "<< std::hex << curProc << " to id " << m_id_map.size() << std::endl;
#endif
      m_process_map[curProc] = m_id_map.size();
      //initialize sc_event for current process
      m_granted[curProc] = new sc_event();
      m_id_map.push_back(curProc);
      m_requested.push_back(false);
}

/**
 * identify the winner from multiple access requests.
 *
 * @note the current implementation is based on round-robin policy. 
 */
void
sc_clked_mutex::arbitrate()
{
    assert(m_master != 0 && m_owner == 0);
    // basic round-robin: starting from the previously granted point, iterate all registered process
    // in order to find the closest requested process.
    for(int i=(m_last_granted+1)%m_id_map.size(); i != m_last_granted; i=(i+1)%m_id_map.size()) {
      if(m_requested[i] == true) {
        m_owner = m_id_map[i];
#ifdef DEBUG
        std::cout << "[DEBUG]: changing a master from " << std::hex << m_master << " to " << m_id_map[i] << "in arbitrate(), owner= " << std::hex << m_owner << std::endl;
#endif
        m_master = m_id_map[i];
        m_last_granted = i;
        return;
      }
    }
    //The current process, which is the master, should be request the access of the resource
    sc_assert(m_requested[m_last_granted] == true);
    m_owner = m_id_map[m_last_granted];
#ifdef DEBUG
    std::cout << "[DEBUG]: use the same master "<< m_master << "in arbitrate(), owner= " << std::hex << m_owner << std::endl;
#endif
    assert(m_master == m_id_map[m_last_granted]);
}

/**
 * yield master to another requested module.
 * 
 * @note if all processes requested with trylock, it will keep yielding masters 
 * until there is nothing left.
 */

inline void
sc_clked_mutex::yield_master()
{
  int i;
  //Simply find the closest requsted process and assign the master into it.
  for(i=0; i < m_requested.size(); i++) {
    if(m_requested[i] == true) {
#ifdef DEBUG
      std::cout << "[DEBUG]: Master yielded to id " << i << std::endl;
#endif
      m_master = m_id_map[i];
      m_granted[m_id_map[i]]->notify();
      return;
    }
  }
  m_master = 0;
}

} // namespace sc_ext

