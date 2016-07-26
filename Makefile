CC = gcc
CFLAGS = -g -Wall

all: FourierTransform

FourierTransform: FourierTransform.c
	$(CC) $(CFLAGS) -o FourierTransform FourierTransform.c

clean:
	rm FourierTransform
