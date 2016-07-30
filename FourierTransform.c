/* FourierTransform.c - fourier transform function. */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <assert.h>
#include "FourierTransform.h"

/* Performs a naive computation of the discrete fourier transform of the input
 * vector. Stores output in the pointer referenced by output argument. */
double complex * slowFourierTransform(double complex * input, int n) {

    double complex * output = malloc(sizeof(double complex) * n);
    if (output == NULL) {
        fprintf(stderr, "err out of memory!\n");
        exit(1);
    }

    for (int k = 0; k < n; k++) {
        double complex nextResult = 0.0;
        for (int j = 0; j < n; j++) {
            nextResult += input[j] * \
                          cexp((-2.0 * M_PI * I * k * j) / (double) n);
        }
        output[k] = nextResult;
    }

    return output;
}

/* helper predicate that returns nonzero if and only if the given integer is a
 * power of two. */
int isPowerofTwo(int n) {
    int x = 1;
    while (x <= n) {
        if (x == n)
            return 1;
        x *= 2;
    }
    return 0;
}

/* Helper function for the Cooley-Tukey fast fourier transform:
 * takes a step argument which refers to the length of the interleaving. */
void fftHelper(double complex * input,
        double complex * output, int n, int step) {

    if (n == 1) {
        /* base case, trivially return the only element of the input. */
        output[0] = input[0];
        return;
    }

    fftHelper(input, output, n / 2, 2 * step);
    fftHelper(input + step, output + (n / 2), n / 2, 2 * step);
    
    for (int i = 0; i < n / 2; i++) {
        double complex temp = output[i];
        output[i] = temp + cexp(-2.0 * M_PI * I * i / n) * output[i + n / 2];
        output[i + n / 2] = temp - \
                            cexp(-2.0 * M_PI * I * i / n) * output[i + n / 2];
    }
}


/* Runs a fast fourier transform (Cooley-Tukey algorithm) on the input array.
 * Allocates an output array of size n which holds the results.
 * Assumes the array's length is a power of two. */
double complex * fastFourierTransform(double complex * input, int n) {
    assert(isPowerofTwo(n));

    double complex * result = malloc(sizeof(double complex) * n);
    if (result == NULL) {
        fprintf(stderr, "err out of memory!\n");
        exit(1);
    }

    fftHelper(input, result, n, 1);

    return result;
}


/* Slides a fourier transform to the next window of time samples.
 * This is a destructive process and will overwrite the fourier coefficients
 * calculated from the last window of time samples, passed in as fourierResults.
 */
void fourierSlide(double complex * fourierResults, double complex * output,
        double complex earlyInput, double complex nextInput, int n) {

    for (int i = 0; i < n; i++) {
        double complex newResult = fourierResults[i];
        newResult -= earlyInput;
        newResult += nextInput;
        newResult *= cexp(2.0 * M_PI * I * i / n);
        output[i] = newResult;
    }

}
