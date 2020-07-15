/* 
 * File:   POSIXOperation.h
 * Author: saw
 *
 * Created on 28 February 2012, 13:24
 */

#ifndef POSIXOPERATION_H
#define	POSIXOPERATION_H

#include <vector>
#include <string>
#include <sstream>

#include "Operation.h"

using namespace std;

class POSIXOperation : public Operation {
public:
    POSIXOperation(Operation::Operations ops, int fileid, double starttime, long startoffset);
    string toString();
    string toCommand();
private:
};

#endif	/* POSIXOPERATION_H */

