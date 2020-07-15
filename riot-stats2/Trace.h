/* 
 * File:   Trace.h
 * Author: saw
 *
 * Created on 28 February 2012, 11:16
 */

#ifndef TRACE_H
#define	TRACE_H

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

#include "Operation.h"

using namespace std;

class Trace {
public:
    Trace(string basename);
    
    void parseFileDB();
    void printFileDB();
    void parseFileLog(int ranks);
    void printFileLog();
    void printBenchmark();
    // calculate the number of ranks in the file...?
    int getRanks();
private:
    string basename;
    
    vector<Operation *> operations;
    map<int, string> filedb;
};

#endif	/* TRACE_H */

