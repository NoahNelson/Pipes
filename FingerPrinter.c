/* FingerPrinter.c - constructs an acoustic fingerprint from a wav file or
 * sequence of samples. */

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
    if (index < 0 || index >= new->elements) {
        fprintf(stderr, "error! vector access at invalid index.\n");
        exit(1);
    }
    return new->peaks[index];
}

/* Append a peak to a peak vector, potentially resizing it. */
void vectorAppend(PeakVector * vect, Peak pk) {
    if (new->elements == new->capacity) {
        new->capacity *= 2;
        new->peaks = realloc(new->peaks, sizeof(Peak) * new->capacity);
        if (new->peaks == NULL) {
            fprintf(stderr, "error! Out of memory.\n");
            exit(1);
        }
    }

    new->peaks[new->elements] = pk;
    new->elements++;
}


