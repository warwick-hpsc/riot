/* 
 * File:   Utils.h
 * Author: saw
 *
 * Created on 29 February 2012, 09:22
 */

#ifndef UTILS_H
#define	UTILS_H

class Utils {
public:
    struct coef {
        double alpha;
        double beta;
    };
    
    Utils();
    static coef doRegression(double x[], double y[], int len);
private:

};

#endif	/* UTILS_H */

