/* WAVReading.h */

uint16_t readWAVChannels(FILE * infile);

int readWAVLength(FILE * infile, int channels);

int getNextMValues(FILE * infile,
        double complex * output, int m, int channels);
