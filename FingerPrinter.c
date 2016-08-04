/* FingerPrinter.c - constructs an acoustic fingerprint from a wav file or
 * sequence of samples. */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include "FourierTransform.h"
#include "WAVReading.h"

/* Initial capacity of peak vectors. */
#define I_CAP 8
/* Length of fourier transforms - how many samples are fed into fft. */
#define FFT_LEN 512
/* Sample-step between time windows - how many samples we slide forward to the
 * next fft. */
/*#define SLIDE_LEN FFT_LEN / 2*/

/**********************
 * Peak Data Structures
 *
 * frequency-time peaks and dynamic vectors to hold a variable number of them.
 */

/* Structure for frequency-time peaks. */
typedef struct _Peak {
    int frequency;
    int timeWindow;
} Peak;

/* frequency-time peak vectors. */
typedef struct _PeakVector {
    int capacity;
    int elements;
    Peak * peaks;
} PeakVector;

/* Initialize an empty vector */
PeakVector * newVector() {
    PeakVector * new = malloc(sizeof(PeakVector));
    if (new == NULL) {
        fprintf(stderr, "error! Out of memory.\n");
        exit(1);
    }

    new->capacity = I_CAP;
    new->elements = 0;
    new->peaks = malloc(sizeof(Peak) * I_CAP);
    if (new->peaks == NULL) {
        fprintf(stderr, "error! Out of memory.\n");
        exit(1);
    }

    return new;
}

/* Get the peak at a given index in a peak vector. */
Peak getPeak(PeakVector * vect, int index) {
    if (index < 0 || index >= vect->elements) {
        fprintf(stderr, "error! vector access at invalid index.\n");
        exit(1);
    }
    return vect->peaks[index];
}

/* Append a peak to a peak vector, potentially resizing it. */
void vectorAppend(PeakVector * vect, Peak pk) {
    if (vect->elements == vect->capacity) {
        vect->capacity *= 2;
        vect->peaks = realloc(vect->peaks, sizeof(Peak) * vect->capacity);
        if (vect->peaks == NULL) {
            fprintf(stderr, "error! Out of memory.\n");
            exit(1);
        }
    }

    vect->peaks[vect->elements] = pk;
    vect->elements++;
}

/* Free the memory associated with a vector. Also frees the pointer passed. */
void freeVector(PeakVector * vect) {
    free(vect->peaks);
    free(vect);
}

/* Compute the time-frequency peaks from the samples in a given WAV file. */
PeakVector * computePeaks(FILE * infile, int m, int channels) {
    PeakVector * result = newVector();
    
    /* Read the first m values into the array of the inputs. */
    double complex * inputs = malloc(sizeof(double complex) * m);
    if (inputs == NULL) {
        fprintf(stderr, "ERR out of memory\n");
        exit(1);
    }
    
    getNextMValues(infile, inputs, m, channels);

    double complex * oldFFTValues = fastFourierTransform(inputs, m);

    PeakVector * potentials = newVector();
    int t = 0;

    double complex * nextFFTValues;

    for (int i = 0; i < m/2; i++)
        inputs[i] = inputs[i + m/2];

    int fileEnd = getNextMValues(infile, inputs + m/2, m/2, channels) != m/2;
    /* Read till the end of the file, collecting peaks. */
    while (!fileEnd) {
        
        

        nextFFTValues = fastFourierTransform(inputs, m);

        /*for (int j = 0; j < 10; j++) {
            fourierSlide(oldFFTValues, nextFFTValues, inputs[0], nextInput, m);

             update the vector of inputs we're currently using 
            for (int i = 0; i < m - 1; i++)
                inputs[i] = inputs[i+1];
            inputs[m-1] = nextInput;
        }*/

        /* Check if we confirmed any potential peaks. */
        for (int i = 0; i < potentials->elements; i++) {
            Peak poss = getPeak(potentials, i);
            if (cabs(oldFFTValues[poss.frequency]) >
                    cabs(nextFFTValues[poss.frequency])) {
                /* peak confirmed. */
                vectorAppend(result, poss);
            }
        }

        /* Find the next potential peaks. */
        freeVector(potentials);
        potentials = newVector();
        for (int i = 1; i < m - 1; i++) {
            double mag = cabs(nextFFTValues[i]);
            if (mag > cabs(nextFFTValues[i-1]) && mag > cabs(nextFFTValues[i+1])
                    && mag > cabs(oldFFTValues[i])) {
                /* found a potential peak! */
                Peak poss = { .frequency = i, .timeWindow = t };
                vectorAppend(potentials, poss);
            }
        }

        /* Move the new fourier transform values into the old array. */
        for (int i = 0; i < m; i++) {
            oldFFTValues[i] = nextFFTValues[i];
        }

        free(nextFFTValues);

        t++;

        fileEnd = getNextMValues(infile, inputs + m/2, m/2, channels) != m/2;
    }

    return result;
}

int main(int argc, char *argv[]) {

    FILE * wav = fopen(argv[1], "r");
    int channels = readWAVHeader(wav);

    printf("detected %d channels.\n", channels);

    PeakVector * peaks = computePeaks(wav, FFT_LEN, channels);

    printf("detected %d peaks.\n", peaks->elements);

    return 0;
}
