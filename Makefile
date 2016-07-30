SOURCES = FourierTransform.c TestFourierTransform.c FingerPrinter.c

OBJECTS = $(SOURCES:.c=.o)

CC = gcc
CFLAGS = -g -Wall -Werror

all: TestFourierTransform

TestFourierTransform: TestFourierTransform.o FourierTransform.o
	$(CC) $(CFLAGS) -o TestFourierTransform $^ $(LDFLAGS)

FingerPrinter: FingerPrinter.o
	$(CC) $(CFLAGS) -o FingerPrinter $^ $(LDFAGS)

clean:
	rm *.o TestFourierTransform

