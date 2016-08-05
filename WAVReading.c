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
uint16_t readWAVChannels(FILE * infile) {

    uint16_t result;

    if (fseek(infile, 22, SEEK_SET)) {
        fprintf(stderr, "readWAVChannels: error seeking file.\n");
        exit(1);
    }

    if (fread(&result, 2, 1, infile) != 1) {
        fprintf(stderr, "readWAVChannels: error reading file.\n");
        exit(1);
    }

    if (fseek(infile, 44, SEEK_SET)) {
        fprintf(stderr, "readWAVChannels: error seeking file.\n");
        exit(1);
    }

    return result;
}

/* Read a WAV header and use the number of channels to compute how many samples
 * are in one channel from start to finish. */
int readWAVLength(FILE * infile, int channels) {

    int result;

    if (fseek(infile, 40, SEEK_SET)) {
        fprintf(stderr, "readWAVLength: error seeking file.\n");
        exit(1);
    }

    if (fread(&result, 4, 1) != 1) {
        fprintf(stderr, "readWAVLength: error reading file.\n");
        exit(1);
    }

    
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


