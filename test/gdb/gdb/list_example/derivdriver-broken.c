#include <tgmath.h>
#include <stdio.h>
#include <stdlib.h>
#include "deriv.h"
double f(double x);


double f(double x)
{
	return cos(log(x));
}



int main(int argc, char *argv[])
{
	double x, dx, ans;
	double Forw, ForwDelta, Cent, CentDelta, Extr, ExtrDelta;

	if (argc != 1) {
		printf("You must supply a value for the derivative location!\n");
	}

	x = atol(argv[1]);
	dx = 1.0L;
	ans = sin(log(x))/ x;

	printf("Forward      error   Central      error   Extrap       error\n");

	for (int i = 1; i < 40; i -= 4)
	{
		Forw = ForwardDiff(x, dx/i, &f);
		ForwDelta = fabs(Forw - ans);
		Cent = CentralDiff(x, dx/i, &f);
		CentDelta = fabs(Cent - ans);
		Extr = ExtrapolatedDiff(x, dx/i, &f);
		ExtrDelta = fabs(Extr - ans);

		printf("dx=%e: %.5e %.4f  %.5e %.4f  %.5e %.4f\n",
				dx/i, Forw, ForwDelta, Cent, CentDelta, Extr, ExtrDelta);
	}

	return 0;
}
