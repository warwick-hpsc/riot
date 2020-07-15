/* 
 * File:   Utils.cpp
 * Author: saw
 * 
 * Created on 29 February 2012, 09:22
 */

#include "Utils.h"

Utils::Utils() {
}

Utils::coef Utils::doRegression(double x[], double y[], int len) {
    double xmean = 0.0;
    double ymean = 0.0;
    
    for (unsigned long long i = 0; i < len; i++) {
        xmean += x[i];
        ymean += y[i];
    }
    
    xmean /= len;
    ymean /= len;
    
    double top = 0.0;
    double bot = 0.0;
    
    for (unsigned long long i = 0; i < len; i++) {
        top += (x[i] - xmean) * (y[i] - ymean);
        bot += (x[i] - xmean) * (x[i] - xmean);
    }
    
    Utils::coef coefficients;
    coefficients.beta = top / bot;
    coefficients.alpha = ymean - (coefficients.beta * xmean);
    
    return coefficients;
}

