/* 
 * File:   MPIOperation.h
 * Author: saw
 *
 * Created on 28 February 2012, 13:46
 */

#ifndef MPIOPERATION_H
#define	MPIOPERATION_H

#include <vector>
#include <string>
#include <sstream>


#include "Operation.h"
#include "POSIXOperation.h"

using namespace std;

class MPIOperation : public Operation {
public:
    MPIOperation(Operation::Operations ops, int rank, int fileid, double starttime, long startoffset);
    string toString();
    string toCommand();
    
    int getRank() { return rank; };
    void setRank(int r) { rank = r; };
    
    long getRealStartOffset();
    
    void addPOSIXOp(POSIXOperation *o);
private:
    int rank;
    vector<POSIXOperation*> posix_ops;
};

#endif	/* MPIOPERATION_H */

