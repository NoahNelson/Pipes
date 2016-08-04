/* WAVReading.c
 *
 * Utility functions for reading .wav files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>


/* Reads the header of a WAV file and returns the number of channels in it.
 * Leaves the file pointer at the beginning of the sample values in the file.
 */
uint16_t readWAVHeader(FILE * infile) {

    uint16_t result;

    if (fseek(infile, 22, SEEK_SET)) {
        fprintf(stderr, "readWAVHeader: error reading file.\n");
        exit(1);
    }

    if (fread(&result, 2, 1, infile) != 1) {
        fprintf(stderr, "readWAVHeader: error reading file.\n");
        exit(1);
    }

    if (fseek(infile, 44, SEEK_SET)) {
        fprintf(stderr, "readWAVHeader: error reading file.\n");
        exit(1);
    }

    return result;
}


/* Read the next m samples from a WAV file into the array passed in.
 * Assumes the file pointer begins somewhere in the samples, header has
 * been skipped.
 *
 * Returns the number of samples read. Will be different from m if EOF reached.
 */
int getNextMValues(FILE * infile,
        double complex * output, int m, int channels) {

    int samples = 0;
    for (int i = 0; i < m; i++) {
        uint16_t sample;
        if (fread(&sample, 2, 1, infile) == 1)
            samples++;
        else
            break;

        output[i] = sample;

        /* Seek ahead past the other channels' samples. */
        if (fseek(infile, 2 * channels, SEEK_CUR)) {
            fprintf(stderr, "getNextMValues: error seeking file.\n");
            exit(1);
        }
    }

    return samples;
}


