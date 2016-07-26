/* FourierTransform.c - fourier transform function. */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <assert.h>
#include "FourierTransform.h"

/* Performs a naive computation of the discrete fourier transform of the input
 * vector. Dynamically allocates memory for the output array. */
double complex * slowFourierTransform(double complex * input, int n) {

    double complex * result = malloc(sizeof(double complex) * n);
    if (result == NULL) {
        fprintf(stderr, "Error, out of memory\n");
        exit(1);
    }

    for (int k = 0; k < n; k++) {
        double complex nextResult = 0.0;
        for (int j = 0; j < n; j++) {
            double complex forSum = -2.0 * M_PI * I * k * j;
            forSum /= (double complex) n;
            forSum = cexp(forSum);
            forSum *= input[j];
            nextResult += forSum;
            /*nextResult += input[j] * cexp((-2.0 * M_PI * I * k * j) / (double) n);*/
        }
        result[k] = nextResult;
    }
    return result;
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
 * Returns the fourier transform of the array given.
 * Assumes the array's length is a power of two. */
double complex * fastFourierTransform(double complex * input, int n) {
    assert(isPowerofTwo(n));

    double complex * evensin, * oddsin;
    double complex * evensout, * oddsout;
    double complex * result = malloc(sizeof(double complex) * n);
    if (result == NULL) {
        fprintf(stderr, "Error, out of memory\n");
        exit(1);
    }

    if (n == 1) {
        /* base case, trivially return the only element of the input. */
        result[0] = input[0];
        return result;
    }
    
    /* prepare the even and odd slices for recursive call. */
    evensin = malloc(sizeof(double complex) * (n/2));
    oddsin = malloc(sizeof(double complex) * (n/2));
    if (evensin == NULL || oddsin == NULL) {
        fprintf(stderr, "Error, out of memory\n");
        exit(1);
    }

    /* copy in the odd and even components of the input. */
    for (int i = 0; i < n; i += 2) {
        evensin[i/2] = input[i];
        oddsin[i/2] = input[i+1];
    }

    evensout = fastFourierTransform(evensin, n / 2);
    oddsout = fastFourierTransform(oddsin, n / 2);

    free(evensin);
    free(oddsin);

    /* now, assemble final output with the even and odd outputs. */
    for (int i = 0; i < n / 2; i++) {
        double complex xi = evensout[i] + cexp((-2.0 * M_PI * I * i) / ((double) n)) * oddsout[i];
        result[i] = xi;
    }
    for (int i = n / 2; i < n; i++) {
        double complex xi = evensout[i - n/2] + cexp((-2.0 * M_PI * I * i) / ((double) n)) * oddsout[i - n/2];
        result[i] = xi;
    }

    free(oddsout);
    free(evensout);

    return result;
}
