/* FourierTransform.c - fourier transform function. */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
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

/* Simple tests of Fourier Transform functions. */
int main() {
    double complex z[] = {1.0 + 3.0 * I, 1.0 - 4.0 * I};
    double complex * x = slowFourierTransform(z, 2);
    for (int i = 0; i < 2; i++) {
        printf("%d: \t %.2f %+.2fi\n", i, creal(x[i]), cimag(x[i]));
    }

    free(x);
}
