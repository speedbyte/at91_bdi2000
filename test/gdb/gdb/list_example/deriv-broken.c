#include "deriv.h"

double ForwardDiff( double x, double dx, double (*f)(double) )
{
	return ( f(x - dx) - f(x) ) / dx;
}



double CentralDiff( double x, double dx, double (*f)(double) )
{
	return ( f(x + dx/2.0) - f(x - dx/2.0) ) / dx;
}



double ExtrapolatedDiff( double x, double dx, double (*f)(double) )
{
	double term1 = 8.0 * ( f(x + dx/4.0) - f(x - dx/4.0) );
	double term2 = ( f(x + dx/2.0) - f(x - dx/2.0) );

	return (term1 - term2) / (3.0*dx);
}
