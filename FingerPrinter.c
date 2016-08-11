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
/* Neighborhood on each side of a point which it must exceed to be a peak. */
#define SIDES 2
#define THRESHOLD 3800000.0
#define DELTA 10000.0
#define FANOUT 5

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

        /* Check if we confirmed any potential peaks. */
        for (int i = 0; i < potentials->elements; i++) {
            Peak poss = getPeak(potentials, i);
            if (cabs(oldFFTValues[poss.frequency]) >
                    cabs(nextFFTValues[poss.frequency])) {
                /* peak confirmed. */
                vectorAppend(result, poss);
            }
        }

        freeVector(potentials);
        potentials = newVector();
        for (int i = SIDES; i < m - SIDES; i++) {
            double mag = cabs(nextFFTValues[i]);
            int isPeak = 1;
            for (int j = 1; j <= SIDES; j++) {
                isPeak = isPeak && mag > cabs(nextFFTValues[i+j]) + DELTA;
                isPeak = isPeak && mag > cabs(nextFFTValues[i-j]) + DELTA;
            }
            isPeak = isPeak && mag > cabs(oldFFTValues[i]) + DELTA;
            isPeak = isPeak && mag > THRESHOLD;
            if (isPeak) {
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

/* Second version of computePeaks, which holds the spectrogram in
 * memory to find peaks. */
PeakVector * computePeaks2(FILE * infile, int m, int channels) {

    /* First, compute the spectrogram. */
    return NULL;
}

/* Structure of a fingerprint. */
typedef struct _Fingerprint {
    /* The time window of the first peak that makes up this fingerprint. */
    int timeWindow;

    /* The values that actually make up the hash of these fingerprints. */
    /* The frequencies of the two peaks in the fingerprint. */
    int frequency1;
    int frequency2;
    /* The time difference between the two peaks. */
    int timeDifference;
} Fingerprint;

/* Package a pair of peaks into a fingerprint. */
Fingerprint fromPeaks(Peak p1, Peak p2) {
    Fingerprint result = { .timeWindow = p1.timeWindow,
        .frequency1 = p1.frequency,
        .frequency2 = p2.frequency,
        .timeDifference = p2.timeWindow - p1.timeWindow };
    return result;
}

/* fingerprint vectors. */
/* TODO: use generic void * vectors? */
typedef struct _FingerprintVector {
    int capacity;
    int elements;
    Fingerprint * fingerprints;
} FingerprintVector;

/* Initialize an empty vector */
FingerprintVector * newFPVector() {
    FingerprintVector * new = malloc(sizeof(FingerprintVector));
    if (new == NULL) {
        fprintf(stderr, "error! Out of memory.\n");
        exit(1);
    }

    new->capacity = I_CAP;
    new->elements = 0;
    new->fingerprints = malloc(sizeof(Fingerprint) * I_CAP);
    if (new->fingerprints == NULL) {
        fprintf(stderr, "error! Out of memory.\n");
        exit(1);
    }

    return new;
}

/* Get the fingerprint at a given index in a fingerprint vector. */
Fingerprint getFingerprint(FingerprintVector * vect, int index) {
    if (index < 0 || index >= vect->elements) {
        fprintf(stderr, "error! vector access at invalid index.\n");
        exit(1);
    }
    return vect->fingerprints[index];
}

/* Append a fingerprint to a fingerprint vector, potentially resizing it. */
void vectorFPAppend(FingerprintVector * vect, Fingerprint fp) {
    if (vect->elements == vect->capacity) {
        vect->capacity *= 2;
        vect->fingerprints =
            realloc(vect->fingerprints, sizeof(Fingerprint) * vect->capacity);
        if (vect->fingerprints == NULL) {
            fprintf(stderr, "error! Out of memory.\n");
            exit(1);
        }
    }

    vect->fingerprints[vect->elements] = fp;
    vect->elements++;
}

/* Free the memory associated with a vector. Also frees the pointer passed. */
void freeFPVector(FingerprintVector * vect) {
    free(vect->fingerprints);
    free(vect);
}

/* Fingerprint all of the peaks in a given peak vector. Returns a vector
 * of the fingerprints that were generated. */
FingerprintVector * fingerprintPeaks(PeakVector * pv) {

    FingerprintVector * result = newFPVector();
    
    for (int i = 0; i < pv->elements; i++) {
        for (int j = 0; j < FANOUT; j++) {
            if (i + j < pv->elements) {
                Fingerprint fp = fromPeaks(getPeak(pv, i), getPeak(pv, i+j));
                vectorFPAppend(result, fp);
            }
        }
    }

    return result;
}


/* Hash a given fingerprint's two frequencies as well as time delta together.
 */
unsigned int djbHash(Fingerprint fp) {
    unsigned int hash = 5381;
    hash = ((hash << 5) + hash) + fp.frequency1;
    hash = ((hash << 5) + hash) + fp.frequency2;
    hash = ((hash << 5) + hash) + fp.timeDifference;
    return hash;
}

/* Take a vector of fingerprint structures and print them out to stdout.
 * Also performs the hashing.
 * For now, there's no hashing actually, we just output the entire fingerprint.
 */
void printFingerprints(FingerprintVector * fps) {
    
    for (int i = 0; i < fps->elements; i++) {
        Fingerprint fp = getFingerprint(fps, i);
        unsigned int hash = djbHash(fp);
        printf("%d %u\n", fp.timeWindow, hash);
    }
}

int main(int argc, char *argv[]) {

    FILE * wav = fopen(argv[1], "r");
    int channels = readWAVChannels(wav);

    /* printf("detected %d channels.\n", channels);*/

    PeakVector * peaks = computePeaks(wav, FFT_LEN, channels);
    /* printf("detected %d peaks.\n", peaks->elements);*/

    FingerprintVector * prints = fingerprintPeaks(peaks);
    /* printf("and created %d fingerprints.\n", prints->elements);*/
    printFingerprints(prints);

    return 0;
}
