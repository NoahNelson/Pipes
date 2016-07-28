#include <complex.h>

void slowFourierTransform(double complex * input,
        double complex * output, int n);

void fastFourierTransform(double complex * input,
        double complex * output, int n);

void fourierSlide(double complex * fourierResults,
        double complex earlyInput, double complex nextInput, int n);
