/* FingerPrinter.c - constructs an acoustic fingerprint from a wav file or
 * sequence of samples. */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <string.h>
#include <assert.h>
#include "FourierTransform.h"
#include "WAVReading.h"

/* Initial capacity of peak vectors. */
#define I_CAP 8

/* Length of fourier transforms - how many samples are fed into fft. */
#define FFT_LEN 4096

/* Neighborhood on each side of a point which it must exceed to be a peak. */
#define NEIGHBORHOOD 8

/* Square size for experimental peak-finding algorithm.
 * Larger keeps peak numbers manageable, but hurts frequency and time res */
#define SQUARESIZE 5

/* Threshold for peaks - peaks must have at least this magnitude. */
#define THRESHOLD 12000000.0

/* Delta threshold for peaks - peaks must be at least this much greater than
 * their neighboring bins. */
#define DELTA 10000.0

/* Fanout factor for constellating peaks. For each peak, take the next FANOUT
 * peaks and make a fingerprint out of each of those pairs. */
#define FANOUT 10 /* TODO: increase this and adjust everything else to keep
                    fingerprint numbers reasonable. */


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

/* Frequency-time peak vectors. */
typedef struct _PeakVector {
    int capacity;
    int elements;
    Peak * peaks;
} PeakVector;

/* Initialize an empty peak vector. Allocates memory for the vector itself
 * and its contents, which starts as I_CAP peaks stored in contiguous memory.
 */
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

/* Get the peak at a given index in a peak vector. Exits if the index is
 * invalid, for safety. */
Peak getPeak(PeakVector * vect, int index) {
    if (index < 0 || index >= vect->elements) {
        fprintf(stderr, "error! vector access at invalid index.\n");
        exit(1);
    }
    return vect->peaks[index];
}

/* Append a peak to a peak vector, potentially resizing it. */
void vectorAppend(PeakVector * vect, Peak pk) {

    /* If the vector is full, increase its capacity by double. */
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

/* Free all memory associated with a vector. Also frees the pointer passed. */
void freeVector(PeakVector * vect) {
    free(vect->peaks);
    free(vect);
}

/* Compute the time-frequency spectrogram of a given WAV file. These can be
 * pretty big, around 2Gb for a 5-minute song. */
double complex ** computeSpectrogram(
        FILE * infile, int m, int channels, int windows) {

    /* Allocate memory for the spectrogram - an array of arrays, one for each
     * time window, each containing the fourier transform frequency profile
     * of that time window. */
    double complex ** spectrogram = malloc(sizeof(double complex *) * windows);
    if (spectrogram == NULL) {
        fprintf(stderr, "error! Out of memory.\n");
        exit(1);
    }

    /* This pointer lets us chase the spectrogram and do our ffts but still
     * return the start of the spectrogram. Replace with pointer arithmetic
     * later when windows has been vetted. */
    double complex ** fft = spectrogram;

    /* Read in the first m values from the file. */
    double complex * inputs = malloc(sizeof(double complex) * m);
    if (inputs == NULL) {
        fprintf(stderr, "error! Out of memory.\n");
        exit(1);
    }

    int fileEnd = getNextMValues(infile, inputs, m, channels) != m;
    
    if (fileEnd) {
        fprintf(stderr, "error: Could not get %d samples from wav file.\n", m);
        exit(1);
    }

    /* Compute the fourier transform of the first window. */

    /* While there's new data, repeatedly shift the array of inputs, read the
     * next m/2 values in, and take a new fourier transform. */

    do {
        *fft = fastFourierTransform(inputs, m);
        fft++;
        windows--;

        /* Shift the array and read the next m/2 values. */
        for (int i = 0; i < m/2; i++)
            inputs[i] = inputs[i + m/2];

        fileEnd = getNextMValues(infile, inputs, m/2, channels) != m/2;
    } while (!fileEnd);

    /* fileEnd should be replaced by feof function. */
    assert(feof(infile));

    /* Make sure the windows calculation was accurate. */
    assert(windows == 0);

    return spectrogram;
}

/* Second version of computePeaks, which holds the spectrogram in memory
 * to find peaks. 
 *
 * This works by breaking up the spectrogram into squares of a given
 * side length, finding the max in each of those squares, and cutting off
 * based on a threshold. */
PeakVector * computePeaksNew(FILE * infile, int m, int channels, int windows) {

    /* First, compute the spectrogram. */
    double complex ** spectrogram
        = computeSpectrogram(infile, m, channels, windows);

    /* Now, iterate over the spectrogram's square regions, collecting peaks.
     * For now, very simplistic brute-force algorithm. */
    PeakVector * peaks = newVector();

    for (int i = 0; i < windows - SQUARESIZE; i += SQUARESIZE) {
        for (int j = 0; j < m - SQUARESIZE; j += SQUARESIZE) {
            double maxAmplitude = THRESHOLD;
            int frequency = -1;
            int timeWindow = -1;
            for (int x = 0; x < SQUARESIZE; x++) {
                for (int y = 0; y < SQUARESIZE; y++) {
                    double amp = cabs(spectrogram[i+x][j+y]);
                    if (amp > maxAmplitude) {
                        maxAmplitude = amp;
                        frequency = j + y;
                        timeWindow = i + x;
                    }
                }
            }

            if (frequency != -1) {
                Peak p = { .frequency = frequency, .timeWindow = timeWindow };
                vectorAppend(peaks, p);
            }
        }
    }

    return peaks;
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

    int fileEnd = (getNextMValues(infile, inputs + m/2, m/2, channels) != m/2);
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
        for (int i = NEIGHBORHOOD; i < m - NEIGHBORHOOD; i++) {
            double mag = cabs(nextFFTValues[i]);
            int isPeak = 1;
            for (int j = 1; j <= NEIGHBORHOOD; j++) {
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

        fileEnd = (getNextMValues(infile, inputs + m/2, m/2, channels) != m/2);
    }

    return result;
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

/* Initialize an empty fingerprint vector */
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
 *
 * Currently, the hash function naively assumes that each of these three
 * values will be less than 16 bits long to get a no-collision hash by just
 * concatenating the bits together. Later we can make a better space/collisions
 * tradeoff with a real hash function.
 */
unsigned int basicHash(Fingerprint fp) {
    unsigned int hash = fp.frequency1;
    hash = (hash << 16) + fp.frequency2;
    hash = (hash << 16) + fp.timeDifference;
    return hash;
}

/* Take a vector of fingerprint structures and print hashes to stdout in a
 * format that sql can read as csv.
 */
void printFingerprints(FingerprintVector * fps, int songId) {
    
    for (int i = 0; i < fps->elements; i++) {
        Fingerprint fp = getFingerprint(fps, i);
        unsigned int hash = basicHash(fp);
        if (songId)
            printf("%d,%u,%u\n", songId, hash, fp.timeWindow);
        else
            printf("%u,%u\n", hash, fp.timeWindow);
    }
}

/********
 * Usage:
 * ./Fingerprinter <wavFile>
 *
 * fingerprints a wav file, dumping the fingerprints to stdout
 * (usually piped to a csv file).
 * file has lines which look like:
 * <songId (if it exists)>\t<hash>\t<timewindow>
 *
 * options:
 * -s <songId> : an optional songId to be attached to the fingerprints, for
 *               importing to sqlite.
 * -v : verbose, a debug mode where fingerprints are not printed to stdout
 *      but some information about the fingerprinting process is given.
 */
int main(int argc, char *argv[]) {

    int songId = 0;
    int verbose = 0;
    char * filename = NULL;
    /* Parse command line arguments. */
    argc--;
    argv++;
    while (argc > 0) {
        if (strcmp(*argv, "-v") == 0)
            verbose = 1;
        else if (strcmp(*argv, "-s") == 0) {
            argc--;
            argv++;
            songId = atoi(*argv);
        }
        else
            filename = *argv;

        argc--;
        argv++;
    }

    FILE * wav = fopen(filename, "r");
    int channels = readWAVChannels(wav);
    int length = readWAVLength(wav, channels);
    int windows = (length / (FFT_LEN / 2)) - 1;

    if (verbose) {
        printf("detected %d channels.\n", channels);
        printf("with a total length of %d.\n", length);
        printf("and %d windows.\n", windows);
    }

    PeakVector * peaks = computePeaksNew(wav, FFT_LEN, channels, windows);

    FingerprintVector * prints = fingerprintPeaks(peaks);
    
    if (verbose) {
        printf("detected %d peaks.\n", peaks->elements);
        printf("and created %d fingerprints.\n", prints->elements);
    }
    else {
        printFingerprints(prints, songId);
    }

    return 0;
}
