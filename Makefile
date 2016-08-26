SOURCES = FourierTransform.c TestFourierTransform.c FingerPrinter.c WAVReading.c
SCRIPTS = PrintAll.sh TestMatcher.sh PrintMatcher.py

OBJECTS = $(SOURCES:.c=.o)

CC = gcc
CFLAGS = -g -Wall -Werror -std=c99

all: TestFourierTransform FingerPrinter

test: FingerPrinter $(SCRIPTS)
	./PrintAll.sh
	./TestMatcher.sh

TestFourierTransform: TestFourierTransform.o FourierTransform.o
	$(CC) $(CFLAGS) -o TestFourierTransform $^ $(LDFLAGS) -lm

FingerPrinter: FingerPrinter.o FourierTransform.o WAVReading.o
	$(CC) $(CFLAGS) -o FingerPrinter $^ $(LDFAGS) -lm

clean:
	rm -f *.o TestFourierTransform FingerPrinter

