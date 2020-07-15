/* 
 * File:   Operation.cpp
 * Author: saw
 * 
 * Created on 28 February 2012, 11:23
 */

#include "Operation.h"

/**
 * Constructor
 */
Operation::Operation(Operations op, int fileid, double starttime, int startoffset) : op(op), fileid(fileid), starttime(starttime), startoffset(startoffset) {

}

/**
 * Destructor method
 */
Operation::~Operation() {
    
}

double Operation::getBandwidth() {
    if (((endoffset - startoffset) != 0) && ((endtime - starttime) < 0.0)) {
        return (endoffset - startoffset) / (endtime - starttime);
    }
    return -1.0;
}

void Operation::endOperation(double endtime, int endoffset) {
    this->endtime = endtime;
    this->endoffset = endoffset;
}

Operation::OpType Operation::getOpType(LogOperations op) {
    if (op < 100) return POSIX;
    else if ((op == BEGIN_MPI_WRITE_ALL) || (op == BEGIN_MPI_READ_ALL) ||
            (op == END_MPI_WRITE_ALL) || (op == END_MPI_READ_ALL) ||
            (op == BEGIN_MPI_OPEN) || (op == END_MPI_OPEN) ||
            (op == BEGIN_MPI_CLOSE) || (op == END_MPI_CLOSE)) return MPICollective;
    else if ((op == BEGIN_MPI_WRITE_SHARED) || (op == BEGIN_MPI_READ_SHARED) ||
            (op == END_MPI_WRITE_SHARED) || (op == END_MPI_READ_SHARED)) return MPIShared;
    else return MPI;
}

Operation::Operations Operation::convertOperation(LogOperations o) {
    switch (o) {
        case Operation::BEGIN_CLOSE:
        case Operation::END_CLOSE:
            return Operation::CLOSE;
        case Operation::BEGIN_OPEN:
        case Operation::END_OPEN:
            return Operation::OPEN;
        case Operation::BEGIN_READ:
        case Operation::END_READ:
            return Operation::READ;
        case Operation::BEGIN_WRITE:
        case Operation::END_WRITE:
            return Operation::WRITE;
        case Operation::BEGIN_LOCK:
        case Operation::END_LOCK:
            return Operation::LOCK;
        case Operation::BEGIN_UNLOCK:
        case Operation::END_UNLOCK:
            return Operation::UNLOCK;
            
        case Operation::BEGIN_MPI_OPEN:
        case Operation::END_MPI_OPEN:
            return Operation::MPI_OPEN;
        case Operation::BEGIN_MPI_CLOSE:
        case Operation::END_MPI_CLOSE:
            return Operation::MPI_CLOSE;
        case Operation::BEGIN_MPI_READ:
        case Operation::END_MPI_READ:
            return Operation::MPI_READ;
        case Operation::BEGIN_MPI_READ_ALL:
        case Operation::END_MPI_READ_ALL:
            return Operation::MPI_READ_ALL;
        case Operation::BEGIN_MPI_READ_SHARED:
        case Operation::END_MPI_READ_SHARED:
            return Operation::MPI_READ_SHARED;
        case Operation::BEGIN_MPI_WRITE:
        case Operation::END_MPI_WRITE:
            return Operation::MPI_WRITE;
        case Operation::BEGIN_MPI_WRITE_ALL:
        case Operation::END_MPI_WRITE_ALL:
            return Operation::MPI_WRITE_ALL;
        case Operation::BEGIN_MPI_WRITE_SHARED:
        case Operation::END_MPI_WRITE_SHARED:
            return Operation::MPI_WRITE_SHARED;
    }
    // Shouldn't happen.
    return (Operation::Operations) -1;
}

bool Operation::isPOSIXOp() {
    if (op < 100) return true;
    else return false;
}

bool Operation::isMPIOp() {
    return !isPOSIXOp();
}

bool Operation::isCollectiveOp() {
    if (op >= 200) return true;
    else return false;
}

string Operation::toOpString(Operations op) {
    switch (op) {
        case Operation::OPEN: return "open";
        case Operation::CLOSE: return "close";
        case Operation::WRITE: return "write";
        case Operation::READ: return "read";
        case Operation::LOCK: return "lock";
        case Operation::UNLOCK: return "unlock";
        case Operation::MPI_OPEN: return "MPI_File_open";
        case Operation::MPI_CLOSE: return "MPI_File_close";
        case Operation::MPI_WRITE: return "MPI_File_write";
        case Operation::MPI_WRITE_SHARED: return "MPI_File_write_shared";
        case Operation::MPI_WRITE_ALL: return "MPI_File_write_all";
        case Operation::MPI_READ: return "MPI_File_read";
        case Operation::MPI_READ_SHARED: return "MPI_File_read_shared";
        case Operation::MPI_READ_ALL: return "MPI_File_read_all";
    }
    return "";
}