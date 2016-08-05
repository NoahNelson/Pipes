SOURCES = FourierTransform.c TestFourierTransform.c FingerPrinter.c WAVReading.c

OBJECTS = $(SOURCES:.c=.o)

CC = gcc
CFLAGS = -g -Wall -Werror -std=c99

all: TestFourierTransform FingerPrinter

TestFourierTransform: TestFourierTransform.o FourierTransform.o
	$(CC) $(CFLAGS) -o TestFourierTransform $^ $(LDFLAGS) -lm

FingerPrinter: FingerPrinter.o FourierTransform.o WAVReading.o
	$(CC) $(CFLAGS) -o FingerPrinter $^ $(LDFAGS) -lm

clean:
	rm -f *.o TestFourierTransform FingerPrinter

