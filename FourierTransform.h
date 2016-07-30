#include <complex.h>

double complex * slowFourierTransform(double complex * input, int n);

double complex * fastFourierTransform(double complex * input, int n);

void fourierSlide(double complex * fourierResults, double complex * output,
        double complex earlyInput, double complex nextInput, int n);
