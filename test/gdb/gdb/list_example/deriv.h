#ifndef DERIV_H
#define DERIV_H

double CentralDiff(double x, double dx, double (*f)(double));
double ExtrapolatedDiff(double x, double dx, double (*f)(double));
double ForwardDiff(double x, double dx, double (*f)(double));

#endif
