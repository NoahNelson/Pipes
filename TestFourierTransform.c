/* TestFourierTransform.c - tests speed of discrete fourier transform
 * functions on large random complex vectors for speed. */

#include "FourierTransform.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <complex.h>
#include <math.h>

#define RDOUBLE() (((double) rand()) / ((double) RAND_MAX))
#define ERR_TOL 1.0E-5


/* Helper function that determines if two complex numbers are within a given
 * error tolerance of each other. */
int ceq(double complex a, double complex b) {
    double complex diff = a - b;
    return cabs(diff) < ERR_TOL;
}

/* Helper function to compare two complex arrays for equality. */
int carrEquals(double complex * a, double complex * b, int n) {
    int result = 1;
    for (int i = 0; i < n; i++) {
        result = result && ceq(a[i], b[i]);
    }
    return result;
}

/* Helper function to print out complex column vectors in two columns. */
void printCArrays(double complex * a, double complex * b, int n) {
    for (int i = 0; i < n; i++) {
        printf("%.2f + %.2fi\t%.2f + %.2fi\n", creal(a[i]), cimag(a[i]), creal(b[i]), cimag(b[i]));
    }
}

/* Brief correctness test for fast fourier transform functions.
 * tests a couple of hard-coded examples, not exhaustive.
 * Returns 0 if everything was correct, 1 if any calls give incorrect results.
 */
int correctnessTest() {
    int result = 0;
    double complex z[] = {1 + 3.0 * I, 1 - 4.0 * I};
    double complex r[] = {2 - 1.0 * I, 0 + 7.0 * I};
    double complex n = 2 + 8 * I;
    double complex r2[] = {3 + 4 * I, -1 - 12 * I};
    double complex * x1, * x2;
    double complex * x3 = malloc(sizeof(double complex) * 2);
    x1 = slowFourierTransform(z, 2);
    x2 = fastFourierTransform(z, 2);
    fourierSlide(x2, x3, 1 + 3 * I, n, 2);

    printf("Expected:\tGot: (slow)\n");
    printCArrays(r, x1, 2);

    printf("Expected:\tGot: (fast)\n");
    printCArrays(r, x2, 2);

    printf("Expected:\tGot: (slide)\n");
    printCArrays(r2, x3, 2);

    result = result || (!carrEquals(r, x1, 2));
    result = result || (!carrEquals(r, x2, 2));

    free(x1);
    free(x2);

    return result;
}

/* Speed test for fast fourier transform functions.
 * Creates an array of n complex numbers, then runs naive and fast fourier
 * transforms on them, measuring the time it takes. */
void speedTest(int n) {
    double complex * input;
    double complex * output;
    clock_t start, end;
    double secs;

    input = malloc(sizeof(double complex) * n);
    if (input == NULL) {
        fprintf(stderr, "error, out of memory\n");
        exit(1);
    }
    
    for (int i = 0; i < n; i++) {
        double complex z = RDOUBLE() + RDOUBLE() * I;
        input[i] = z;
    }

/*
    start = clock();
    output = slowFourierTransform(input, n);
    end = clock();
    secs = (double)(end - start) / CLOCKS_PER_SEC;
    printf("slow fourier transform took %f seconds.\n", secs);
    free(output);
*/

    start = clock();
    output = fastFourierTransform(input, n);
    end = clock();
    secs = (double)(end - start) / CLOCKS_PER_SEC;
    printf("fast fourier transform took %f seconds.\n", secs);

    free(input);
    free(output);
}

#define PURESIZE 65536

/* Generates an input representing a pure tone, and runs it through the fft.
 * Test of what the output means in terms of frequencies. */
void testPureTone() {

    double complex * input = malloc(sizeof(double complex) * PURESIZE);
    if (input == NULL) {
        fprintf(stderr, "out of memory.\n");
        exit(1);
    }

    /* populate the input array with samples from a pure 440 Hz tone. */
    for (int i = 0; i < PURESIZE; i++) {
        input[i] = sin(2 * M_PI * 440 * i / 44100);
    }

    double complex * output = fastFourierTransform(input, PURESIZE);

    printf("frequency:\t magnitude:\n");
    for (int i = 0; i < PURESIZE; i++) {
        printf("%d:\t %.2f\n", i * 44100 / PURESIZE, cabs(output[i]));
    }

    free(input);
    free(output);
}

/* Program usage:
 * TestFourierTransform N <optional random seed>
 * for speed tests
 * or just
 * TestFourierTransform
 * for the brief correctness test.
 *
 * tests the slow and fast fourier transforms on an array of N random complex
 * numbers, printing out how long it took with each function.
 * If no seed is provided, uses the current time. */
int main(int argc, char *argv[]) {
    int n;

    if (argc > 3) {
        printf("usage: %s N <optional random seed> (speed)\n", argv[0]);
        printf("or   : %s (correctness)\n", argv[0]);
        exit(1);
    }
    if (argc == 1) {
        int result = correctnessTest();
        if (result)
            printf("test failed!\n");
        else
            printf("test passed!\n");
        return 0;
    }
    if (argc == 3) {
        srand((unsigned) atoi(argv[2]));
    }
    else {
        srand((unsigned) time(NULL));
    }
    
    n = atoi(argv[1]);

    speedTest(n);

    /* testPureTone();*/

    return 0;

}
