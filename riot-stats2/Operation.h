/* 
 * File:   Operation.h
 * Author: saw
 *
 * Created on 28 February 2012, 11:23
 */

#ifndef OPERATION_H
#define	OPERATION_H

#include <string>

using namespace std;

class Operation {
public:
    enum OpType {
        POSIX, MPI, MPICollective, MPIShared
    };
    
    enum LogOperations {
        BEGIN_OPEN = 0, END_OPEN,
	BEGIN_CLOSE, END_CLOSE,
	BEGIN_READ, END_READ,
	BEGIN_WRITE, END_WRITE,
	BEGIN_LOCK, END_LOCK,
	BEGIN_UNLOCK, END_UNLOCK,
	BEGIN_MPI_OPEN = 100, END_MPI_OPEN,
	BEGIN_MPI_CLOSE, END_MPI_CLOSE,
	BEGIN_MPI_READ, END_MPI_READ,
	BEGIN_MPI_READ_ALL, END_MPI_READ_ALL,
	BEGIN_MPI_READ_SHARED, END_MPI_READ_SHARED,
	BEGIN_MPI_WRITE, END_MPI_WRITE,
	BEGIN_MPI_WRITE_ALL, END_MPI_WRITE_ALL,
	BEGIN_MPI_WRITE_SHARED, END_MPI_WRITE_SHARED
    };
    
    enum Operations {
        OPEN, CLOSE, READ, WRITE, LOCK, UNLOCK,      
        MPI_READ = 100, MPI_READ_SHARED,
        MPI_WRITE, MPI_WRITE_SHARED,
        MPI_OPEN = 200, MPI_CLOSE,
        MPI_WRITE_ALL, MPI_READ_ALL
    };
    
    Operation(Operations op, int fileid, double startime, int startoffset);
    ~Operation();
    
    Operations getOperation() { return op; };
    void setOperation(Operations o) { op = o; };
    
    static Operations convertOperation(LogOperations o);
    
    double getStartTime() { return starttime; };
    double getEndTime() { return endtime; };
    void setStartTime(double t) { starttime = t; };
    void setEndTime(double t) { endtime = t; };

    long getStartOffset() { return startoffset; };
    long getEndOffset() { return endoffset; };
    void setStartOffset(long o) { startoffset = o; };
    void setEndTime(long o) { endtime = o; };
    
    static Operation::OpType getOpType(LogOperations op);
    
    static bool isBeginOp(LogOperations op) { return ((((int)op) % 2) == 0); }
    static bool isEndOp(LogOperations op) { return !isBeginOp(op); }
    
    static string toOpString(Operations op);
    
    bool isPOSIXOp();
    bool isMPIOp();
    bool isCollectiveOp();
    
    virtual void endOperation(double endtime, int endoffset);
    
    virtual double getBandwidth();
    virtual string toString() = 0;
    virtual string toCommand() = 0;

protected:
    Operations op;
    
    int fileid;
    
    double starttime;
    double endtime;
    
    long startoffset;
    long endoffset;   
};
#endif	/* OPERATION_H */

