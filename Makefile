SOURCES = FourierTransform.c TestFourierTransform.c

OBJECTS = $(SOURCES:.c=.o)

CC = gcc
CFLAGS = -g -Wall -Werror

all: TestFourierTransform

TestFourierTransform: TestFourierTransform.o FourierTransform.o
	$(CC) $(CFLAGS) -o TestFourierTransform $^ $(LDFLAGS)

clean:
	rm *.o TestFourierTransform

