/* FingerPrinter.c - constructs an acoustic fingerprint from a wav file or
 * sequence of samples. */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>

/* frequency-time peaks and the vectors to hold a variable number of them. */

/* Initial capacity of peak vectors. */
#define I_CAP 8


/* Structure for frequency-time peaks. */
typedef struct _Peak {
    double magnitude;
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
void initVector(PeakVector * new) {
    new->capacity = I_CAP;
    new->elements = 0;
    new->peaks = malloc(sizeof(Peak) * I_CAP);
    if (new->peaks == NULL) {
        fprintf(stderr, "error! Out of memory.\n");
        exit(1);
    }
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

/* Reads the header of a WAV file and returns the number of channels in it.
 * Leaves the file pointer at the beginning of the sample values in the file.
 */
int readWAVHeader(FILE * infile) {
    unsigned char byte1;
    unsigned char byte2;

    if (fseek(infile, 22, SEEK_SET)) {
        fprintf(stderr, "error reading file.\n");
        exit(1);
    }

    byte1 = fgetc(infile);
    byte2 = fgetc(infile);

    if (fseek(infile, 44, SEEK_SET)) {
        fprintf(stderr, "error reading file.\n");
        exit(1);
    }

    /* The order here is weird because of endian-ness. */
    return (byte2 << 8) | byte1;
}

/* Read the first m samples from a WAV file into the array passed in.
 * Assumes the file pointer begins at the samples, header has been skipped. */
void getFirstValues(double complex * output, FILE * infile,
        int m, int channels) {

}

/* Compute the time-frequency peaks from the samples in a given WAV file. */
void computePeaks(PeakVector * results, FILE * infile, int m, int channels) {
    
    double complex * inputs = malloc(sizeof(double complex) * m);
    if (inputs == NULL) {
        fprintf(stderr, "error, out of memory.\n");
        exit(1);
    }

}

int main(int argc, char *argv[]) {

    FILE * wav = fopen(argv[1], "r");
    int channels = readWAVHeader(wav);

    printf("detected %d channels.\n", channels);

    return 0;
}
