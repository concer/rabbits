/**
 * @filename FlitTLM.h
 *
 * @brief The file that includes a class of flit definition for the RouterTLM class.
 *
 * @author Young Jin Yoon <youngjin@cs.columbia.edu>
 *
 * @sa RouterTLM.h
 */

#ifndef __FLIT_TLM_H__
#define __FLIT_TLM_H__
#include "systemc.h"
#include <string>
#include <iomanip>

#define MAX_SRC     20
#define MAX_DEST    20
#define MAX_TRANS   20
#define MAX_FLIT    10

/**
 * @brief The flit class that is used by the systemC-TLM router implementation, RouterTLM.
 * 
 */
class FlitTLM {
  public:
    int transID;    //!< transaction id
    int srcID;      //!< source identifier
    int destID;     //!< destination identifier
    int flitID;     //!< flit id in a transaction

    /**
     * constructor
     */
    FlitTLM (int transID=0, int srcID=0, int destID=0, int flitID=0) {
      this->transID = transID;
      this->srcID = srcID;
      this->destID = destID;
      this->flitID = flitID;
    }

    /**
     * equal operator overloading. 
     * @note make sure all member variables of the current object is compared to the rhs object
     */
    inline bool operator == (const FlitTLM & rhs) const {
      return (
              rhs.transID == transID &&
              rhs.srcID == srcID && 
              rhs.destID == destID &&
              rhs.flitID == flitID
             );
    }

    /**
     * assignment operator overloading. 
     * @note make sure all member variables of the rhs object is assigned to the current object
     */
    inline FlitTLM& operator = (const FlitTLM& rhs) {
      srcID = rhs.srcID;
      destID = rhs.destID;
      transID = rhs.transID;
      flitID = rhs.flitID;
      return *this;
    }

    /**
     * assignment operator overloading. 
     * @note this will be helpful later to trace information. Just make sure all 
     * member variables of the v object is used by sc_trace() function.
     */
    inline friend void sc_trace(sc_trace_file *tf, const FlitTLM & v, const std::string& NAME ) {
      sc_trace(tf, v.transID, NAME + ".transID");
      sc_trace(tf, v.srcID, NAME + ".srcID");
      sc_trace(tf, v.destID, NAME + ".destID");
      sc_trace(tf, v.flitID, NAME + ".flitID");
    }

    /**
     * shift-left operator overloading.
     * @note this is necessary to print the contents of an object with std::cout. Just make sure
     * all member variables of the v object is "shift-lefted" with ostream class.
     */
    inline friend ostream& operator << ( ostream& os,  FlitTLM const & v ) {
      os << "(";
      os << "transID=" << v.transID;
      os << ", srcID=" << v.srcID;
      os << ", destID=" << v.destID;
      os << ", flitID=" << v.flitID;
      os << ")";
      return os;
    }

    /**
     * create an object and randomize all variables
     * @note you may want to adjust individual variables after executing this function, especially FlitTLM::transID and FlitTLM::flitID.
     */
    static inline FlitTLM &generateRandom ( FlitTLM &v ) {
        v.srcID = int(rand() % (MAX_SRC+1));
        do {
          v.destID = int(rand() % (MAX_DEST+1));
        } while (v.srcID == v.destID);
        v.transID = int(rand() % (MAX_TRANS+1));
        v.flitID = int(rand() % (MAX_FLIT+1));
    }

};

#endif
