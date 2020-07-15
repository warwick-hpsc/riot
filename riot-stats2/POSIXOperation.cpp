/* 
 * File:   POSIXOperation.cpp
 * Author: saw
 * 
 * Created on 28 February 2012, 13:24
 */

#include "POSIXOperation.h"

POSIXOperation::POSIXOperation(Operation::Operations ops, int fileid, double starttime, long startoffset) : Operation(ops, fileid, starttime, startoffset) {

}

string POSIXOperation::toString() {
    stringstream ss;
    ss << toOpString(op) << "," << starttime << "," << endtime;
    return ss.str();
}

string POSIXOperation::toCommand() {
    return "";
}
