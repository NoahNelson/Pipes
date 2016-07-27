/* FourierTransform.c - fourier transform function. */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <assert.h>
#include "FourierTransform.h"

/* Performs a naive computation of the discrete fourier transform of the input
 * vector. Stores output in the pointer referenced by output argument. */
void slowFourierTransform(double complex * input,
        double complex * output, int n) {

    for (int k = 0; k < n; k++) {
        double complex nextResult = 0.0;
        for (int j = 0; j < n; j++) {
            nextResult += input[j] * \
                          cexp((-2.0 * M_PI * I * k * j) / (double) n);
        }
        output[k] = nextResult;
    }
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
 * Places the fourier transform of the array beginning at input in the array
 * beginning at output.
 * Assumes the array's length is a power of two. */
void fastFourierTransform(double complex * input,
        double complex * output, int n) {
    assert(isPowerofTwo(n));

    fftHelper(input, output, n, 1);

    /* OLD IMPLEMENTATION - NOT MEMORY EFFICIENT
    double complex * evensin, * oddsin;
    double complex * evensout, * oddsout;

    if (n == 1) {
        output[0] = input[0];
        return;
    }
    
    evensin = malloc(sizeof(double complex) * (n/2));
    oddsin = malloc(sizeof(double complex) * (n/2));
    if (evensin == NULL || oddsin == NULL) {
        fprintf(stderr, "Error, out of memory\n");
        exit(1);
    }

    evensout = malloc(sizeof(double complex) * (n/2));
    oddsout = malloc(sizeof(double complex) * (n/2));
    if (evensout == NULL || oddsout == NULL) {
        fprintf(stderr, "Error, out of memory\n");
        exit(1);
    }

    for (int i = 0; i < n; i += 2) {
        evensin[i/2] = input[i];
        oddsin[i/2] = input[i+1];
    }

    fastFourierTransform(evensin, evensout, n / 2);
    fastFourierTransform(oddsin, oddsout, n / 2);

    free(evensin);
    free(oddsin);

    for (int i = 0; i < n / 2; i++) {
        double complex xi = evensout[i] + cexp((-2.0 * M_PI * I * i) / ((double) n)) * oddsout[i];
        output[i] = xi;
    }
    for (int i = n / 2; i < n; i++) {
        double complex xi = evensout[i - n/2] + cexp((-2.0 * M_PI * I * i) / ((double) n)) * oddsout[i - n/2];
        output[i] = xi;
    }

    free(oddsout);
    free(evensout);
    */
}
