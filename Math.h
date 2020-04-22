#ifndef MATH_H
#define MATH_H
#include <string>
const double EPS_MACHINE = 1e-9;

inline double module(double a) {
    return (a < 0) ? -a : a;
}

inline double ln(double a) {
    double xpt = (a - 1.0) / (a + 1.0);
    double res = 0.0,
	       intermediateRes = -1.0,
	       numerator = xpt,
	       denominator = 1.0;
           xpt *= xpt;
    while (module(res - intermediateRes) > EPS_MACHINE) {
        intermediateRes = res,
	    res += numerator / denominator,
	    numerator *= xpt,
	    denominator += 2.0;
    }

    return 2.0 * res;
}

inline double log(double a, double b) {
    if (a <= 0.0 || b <= 0.0 || module(a - 1.0) <= EPS_MACHINE)
        throw std::string("Logarithm is not defined\n");
    return ln(b)/ln(a);
}

inline double exp(double a) {
    double res = 0.0,
	       intermediateRes = -1.0,
	       numerator = 1.0,
	       denominator = 1.0,
	       cnt = 1.0;
    while (module(res - intermediateRes) > EPS_MACHINE) {
	    intermediateRes = res,
        res += numerator/denominator,
        numerator *= a,
       	denominator *= cnt,
	    cnt += 1.0;
    }
    return res;
}

inline double peng_pow(double a, double b) {
    if (a < 0)
	    throw std::string("ERROR : raising a negative number to a fractional power");
    if (a == 0) {
        if (b == 0)
	        return 1;
	else
        return 0;
					        }
    if (b < 0)
	    return exp(ln(a) * (-b));
    else
        return exp(ln(a) * b);
}

inline int peng_pow(int a, int b){
    if (b < 0)
	    throw std::string("Negative degree in operation '**' with integer arguments is not allowed");
    int result = 1;
    while (b) {
        if (b&1)
	        --b,
            result *= a;
        else
	        b >>= 1,
	        a *= a;
    }
    return result;
}

#endif //MATH_H
