/* 
 * File:   Trace.cpp
 * Author: saw
 * 
 * Created on 28 February 2012, 11:16
 */

#include "Trace.h"
#include "MPIOperation.h"
#include "MPICollectiveOperation.h"

#include <iostream>

using namespace std;

Trace::Trace(string basename) : basename(basename) {
    int ranks = getRanks();
    parseFileDB();
    printFileDB();
    cout << "Parsing log for " << ranks << " ranks" << endl;
    parseFileLog(ranks);
    printFileLog();
}

void Trace::parseFileDB() {
    ifstream file ((basename + ".filedb").c_str());
    
    if (file.is_open()) {
        while (!file.eof()) {
            int fileid;
            string filename;

            // get file id and any whitespace. Then just read the rest of the line.
            file >> fileid >> ws;
            getline(file, filename);
            
            // Only add if there's an actual filename
            if (filename.length() > 0)
                filedb.insert(pair<int,string>(fileid, filename));
        }
        file.close();
    }
}

void Trace::printFileDB() {
    map<int,string>::iterator my_iter;
    for (my_iter = filedb.begin(); my_iter != filedb.end(); my_iter++) {
        cout << my_iter->first << ", " << my_iter->second << endl;
    }
}

void Trace::parseFileLog(int ranks) {
    map<pair<int,int>, Operation*> pendingPOSIXOperations;
    map<int, Operation*> pendingMPIOperations;
    MPICollectiveOperation *pendingMPICollectiveOperation = NULL;
    
    ifstream file ((basename + ".log").c_str());
    
    if (file.is_open()) {
        while (!file.eof()) {
            double time;
            int rank;
            int op;
            int fileid;
            long offset;
            
            file >> time >> ws >> rank >> ws >> op >> ws >> fileid >> ws >> offset >> ws;
            
            Operation *current_op;
            
            if (Operation::isBeginOp((Operation::LogOperations) op)) {
                
                switch(Operation::getOpType((Operation::LogOperations) op)) {
                    case Operation::POSIX: {
                        current_op = (Operation*) new POSIXOperation(Operation::convertOperation((Operation::LogOperations) op), fileid, time, offset);
                        
                        pendingPOSIXOperations.insert(pair<pair<int,int>,Operation*>(pair<int,int>(rank,op),current_op));                        
                        
                        map<int,Operation*>::iterator it = pendingMPIOperations.find(rank);

                        if (it != pendingMPIOperations.end()) {
                            MPIOperation *mpi_op = (MPIOperation *) it->second;
                            mpi_op->addPOSIXOp((POSIXOperation*)current_op);
                        } else {
                            operations.push_back(current_op);
                        }
                        
                        break;
                    }
                    case Operation::MPI: {
                        current_op = (Operation*) new MPIOperation(Operation::convertOperation((Operation::LogOperations) op), rank, fileid, time, offset);
                        
                        pendingMPIOperations.insert(pair<int,Operation*>(rank,current_op));
                        
                        operations.push_back(current_op);
                        break;
                    }
                    case Operation::MPICollective: {
                        current_op = (Operation*) new MPIOperation(Operation::convertOperation((Operation::LogOperations) op), rank, fileid, time, offset);
                        
                        pendingMPIOperations.insert(pair<int,Operation*>(rank,current_op));

                        if (pendingMPICollectiveOperation == NULL) {
                            pendingMPICollectiveOperation = new MPICollectiveOperation(Operation::convertOperation((Operation::LogOperations) op), fileid);
                            operations.push_back((Operation *) pendingMPICollectiveOperation);
                        }
                        
                        
                        break;
                    }
                    case Operation::MPIShared:
                        // not implemented
                        break;
                    default:
                        // something has gone wrong.
                        break;
                }
            } else {
                switch(Operation::getOpType((Operation::LogOperations) op)) {
                    case Operation::POSIX: {
                        current_op = pendingPOSIXOperations.find(pair<int,int>(rank,op-1))->second;
                        pendingPOSIXOperations.erase(pair<int,int>(rank, op-1));
                        
                        current_op->endOperation(time, offset);
                        break;
                    }
                    case Operation::MPI: {
                        current_op = pendingMPIOperations.find(rank)->second;
                        pendingMPIOperations.erase(rank);
                        
                        current_op->endOperation(time,offset);
                        
                        break;
                    }
                    case Operation::MPICollective: {
                        current_op = pendingMPIOperations.find(rank)->second;
                        pendingMPIOperations.erase(rank);
                        
                        current_op->endOperation(time,offset);
                        
                        pendingMPICollectiveOperation->addRank((MPIOperation*) current_op);
                        
                        if (pendingMPICollectiveOperation->getNumberOfMPIOperations() == ranks) {                          
                            pendingMPICollectiveOperation = NULL;
                        }
                        
                        break;
                    }
                    case Operation::MPIShared:
                        // not yet implemented.
                        break;
                    default:
                        // broken.
                        break;
                }
            }
        }
        file.close();
    }
}

void Trace::printFileLog() {    
    vector<Operation*>::iterator my_iter;
    for (my_iter = operations.begin(); my_iter != operations.end(); my_iter++) {
        Operation *current_op = *my_iter;
        
        if (current_op->isPOSIXOp()) {
            cout << "POSIX:   " << ((POSIXOperation *)current_op)->toString() << endl;
        } else if (current_op->isCollectiveOp()) {
            cout << "MPIColl: " << ((MPICollectiveOperation *)current_op)->toString() << endl;
            cout << " Eq: " << ((MPICollectiveOperation *)current_op)->toCommand() << endl;
        } else {
            cout << "MPI:     " << ((MPIOperation *)current_op)->toString() << endl;
        }  
    }
}

int Trace::getRanks() {
    int ranks = 0;
    ifstream file ((basename + ".log").c_str());
    
    if (file.is_open()) {
        while (!file.eof()) {
            double time;
            int rank;
            int op;
            int fileid;
            long offset;
            
            file >> time >> ws >> rank >> ws >> op >> ws >> fileid >> ws >> offset;
            
            if ((rank+1) > ranks) ranks = rank + 1;
        }
        file.close();
    }  
    return ranks;
}
