/* WAVReading.h */

uint16_t readWAVHeader(FILE * infile);

int getNextMValues(FILE * infile,
        double complex * output, int m, int channels);
