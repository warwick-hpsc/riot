/* 
 * File:   MPICollectiveOperation.h
 * Author: saw
 *
 * Created on 28 February 2012, 13:46
 */

#ifndef MPICOLLECTIVEOPERATION_H
#define	MPICOLLECTIVEOPERATION_H

#include <map>
#include <string>
#include <sstream>

#include "Operation.h"
#include "MPIOperation.h"

using namespace std;

class MPICollectiveOperation : public Operation {
public:
    MPICollectiveOperation(Operation::Operations ops, int fileid);
    ~MPICollectiveOperation();
    
    void addRank(MPIOperation *op);
    
    MPIOperation* getRank(int rank);

    int getNumberOfMPIOperations(); // Need to be able to return the number of ops that are in the hash table.
    
    string toString();
    string toCommand();
private:
    // Map rank to MPIOperation
    map<int, MPIOperation *> mpi_ops;
};

#endif	/* MPICOLLECTIVEOPERATION_H */

