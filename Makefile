SOURCES = FourierTransform.c TestFourierTransform.c FingerPrinter.c WAVReading.c
SCRIPTS = PrintAll.sh TestMatcher.sh PrintMatcher.py
SQLITE  = TestSet/test.sqlite
DBINIT  = InitDatabase.sql

OBJECTS = $(SOURCES:.c=.o)

CC = gcc
CFLAGS = -g -Wall -Werror -std=c99

all: TestFourierTransform FingerPrinter

# Dependencies of this aren't exactly right. Should detect if we need new
# fingerprints.
test: FingerPrinter $(SCRIPTS)
	rm -f $(SQLITE)
	sqlite3 $(SQLITE) < $(DBINIT)
	./PrintAll.sh
	./FillSQL.sh $(SQLITE)
	./TestMatcher.sh $(SQLITE)

snippet: FingerPrinter
	./FillSQL.sh $(SQLITE)
	./FingerPrinter TestSet/Angelssnippet.wav > TestSet/Angelssnippet.csv
	python3 PrintMatcher.py TestSet/Angelssnippet.csv $(SQLITE)

TestFourierTransform: TestFourierTransform.o FourierTransform.o
	$(CC) $(CFLAGS) -o TestFourierTransform $^ $(LDFLAGS) -lm

FingerPrinter: FingerPrinter.o FourierTransform.o WAVReading.o
	$(CC) $(CFLAGS) -o FingerPrinter $^ $(LDFAGS) -lm

clean:
	rm -f *.o TestFourierTransform FingerPrinter

