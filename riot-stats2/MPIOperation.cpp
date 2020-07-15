/* 
 * File:   MPIOperation.cpp
 * Author: saw
 * 
 * Created on 28 February 2012, 13:46
 */

#include "MPIOperation.h"

MPIOperation::MPIOperation(Operation::Operations ops, int rank, int fileid, double starttime, long startoffset) : 
        Operation(ops, fileid, starttime, startoffset), rank(rank) {
    if (ops < 100) {
        throw 30;
    }
}


string MPIOperation::toString() {
    stringstream ss;
    ss << toOpString(op) << "," << rank << "," << starttime << "," << endtime;
    return ss.str();
}

string MPIOperation::toCommand() {
    stringstream ss;
    ss << "if (rank == " << rank << ") MPI_File_write_at(fh" << fileid << ", " << startoffset << ", buf, " << endoffset - startoffset << ", MPI_BYTE, &status);";
    return ss.str();
}

void MPIOperation::addPOSIXOp(POSIXOperation *o) {
    posix_ops.push_back(o);
}

long MPIOperation::getRealStartOffset() {
    long offset = -1;
    
    vector<POSIXOperation *>::iterator it;
    for (it = posix_ops.begin(); it != posix_ops.end(); it++) {
        POSIXOperation *p = *it;
        long curr_offset = p->getStartOffset();
        offset = ((offset == -1) || (curr_offset < offset)) ? curr_offset : offset;
    }
    
    return offset;
}