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


/* Runs a fast fourier transform (Cooley-Tukey algorithm) on the input array.
 * Places the fourier transform of the array beginning at input in the array
 * beginning at output.
 * Assumes the array's length is a power of two. */
void fastFourierTransform(double complex * input,
        double complex * output, int n) {
    assert(isPowerofTwo(n));

    double complex * evensin, * oddsin;
    double complex * evensout, * oddsout;

    if (n == 1) {
        /* base case, trivially return the only element of the input. */
        output[0] = input[0];
        return;
    }
    
    /* prepare the even and odd slices for recursive call. */
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

    /* copy in the odd and even components of the input. */
    for (int i = 0; i < n; i += 2) {
        evensin[i/2] = input[i];
        oddsin[i/2] = input[i+1];
    }

    fastFourierTransform(evensin, evensout, n / 2);
    fastFourierTransform(oddsin, oddsout, n / 2);

    free(evensin);
    free(oddsin);

    /* now, assemble final output with the even and odd outputs. */
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
}
