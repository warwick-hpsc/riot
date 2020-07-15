/* 
 * File:   MPICollectiveOperation.cpp
 * Author: saw
 * 
 * Created on 28 February 2012, 13:46
 */

#include "MPICollectiveOperation.h"
#include "MPIOperation.h"
#include "Trace.h"
#include "Utils.h"

MPICollectiveOperation::MPICollectiveOperation(Operation::Operations op, int fileid) : Operation(op, fileid, 0.0, 0) {
    if (op < 200) {
        throw 30;
    }
}

// Clear up the memory.
MPICollectiveOperation::~MPICollectiveOperation() {
        mpi_ops.clear();
}

void MPICollectiveOperation::addRank(MPIOperation *op) {
    mpi_ops.insert(pair<int, MPIOperation *>(op->getRank(), op));
    
    starttime = ((starttime == 0.0) || starttime > op->getStartTime()) ? op->getStartTime() : starttime;
    endtime = ((endtime == 0.0) || endtime < op->getEndTime()) ? op->getEndTime() : endtime;

    startoffset = ((startoffset == 0) || startoffset > op->getStartOffset()) ? op->getStartOffset() : startoffset;
    endoffset = ((endoffset == 0) || endoffset < op->getEndOffset()) ? op->getEndOffset() : endoffset;
}


MPIOperation* MPICollectiveOperation::getRank(int i) {
    // need to check if its found before trying to return?
    
    map<int, MPIOperation *>::iterator it = mpi_ops.find(i);
    
    if (it == mpi_ops.end()) {
        cout << "Something's gone wrong" << endl;
        return NULL;
    } else
        return it->second;
}



string MPICollectiveOperation::toString() {
    stringstream ss;
    ss << toOpString(op) << "," << starttime << "," << endtime;
    
    return ss.str();
}

string MPICollectiveOperation::toCommand() {
    stringstream ss;
    
    
    if (op == MPI_OPEN) {
        ss << "MPI_File_open()...";
    } else if (op == MPI_CLOSE) {
        ss << "MPI_File_close()...";
    } else {
        int size = mpi_ops.size();

        double *x = new double[size];
        double *y = new double[size];

        int i = 0;
        int j = 0;
        while (i < size) {
            long curroffset = mpi_ops.find(i)->second->getRealStartOffset();

            x[j] = i;
            //y[i] = mpi_ops.find(i)->second->getStartOffset();
            y[j] = curroffset;

            if (curroffset != -1) j++;

            i++;
        }

        Utils::coef cf = Utils::doRegression(x, y, j);

        //ss << "Equation is probably: " << cf.alpha << " + (" << cf.beta << " * rank) = y";
        
        // calculate a count.... 
        long count = (this->getEndOffset() - this->getStartOffset()) / mpi_ops.size();

        ss << "MPI_File_write_at_all(fh" << fileid << ", " << (long)cf.alpha << "+(rank*" << (long)cf.beta << "), buf, " << count << ", MPI_BYTE, &status);";
        delete x;
        delete y;
    }
    
    return ss.str();
}

int MPICollectiveOperation::getNumberOfMPIOperations() {
    return mpi_ops.size();
}