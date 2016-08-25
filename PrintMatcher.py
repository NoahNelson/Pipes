# PrintMatcher.py
# code for matching song fingerprints.
# Takes a recorded fingerprint through a file or stdio and compares it to a
# database, trying to find a likely match.
import argparse
#import MySQLdb
import sys

BINSIZE = 5
MATCHTHRESHOLD = 100

class DeltaBin:
    """Stores time deltas in bins so that it's easy to get the size of the
    largest bin."""

    def __init__(self, binSize):
        """Create an empty DeltaBin."""
        self.binSize = binSize
        self.maxBin = 0
        self.numbers = {}

    def add(self, delta):
        """Add the given delta to the correct bin"""
        index = delta / self.binSize
        if index in self.numbers:
            self.numbers[index] += 1
        else:
            self.numbers[index] = 1
        if self.numbers[index] > self.maxBin:
            self.maxBin = self.numbers[index]

    def largestBin(self):
        """Return the number of deltas in the largest bin."""
        return self.maxBin

def matchesFromDB(dbCursor, hashVal):
    """Get matches of a hash from a database connection."""
    #TODO: error handling and transition to sqlite
    dbCursor.execute(
            "SELECT songId, offset FROM fingerprints \
             WHERE hash = %d" % hashVal)
    return dbCursor.fetchall()

def matchesFromFile(stream, hashVal, songId=0):
    """Get matches of a hash from a given file stream."""
    results = []
    for line in stream:
        fp = line.split('\t')
        hashVal2 = int(fp[0])
        timeOffset = int(fp[1])
        if hashVal2 == hashVal:
            results.append((songId, timeOffset))
    stream.seek(0)
    return results

def matchesBetweenFiles(stream1, stream2):
    """Gets the number of time delta matches in the largest delta bin made
    of matches between the fingerprints in two files."""
    # matching stream 1 against stream 2
    matchBins = DeltaBin(BINSIZE)
    for line in stream1:
        fp = line.split('\t')
        hashVal = int(fp[0])
        offset = int(fp[1])
        matches = matchesFromFile(stream2, hashVal)
        for match in matches:
            offset2 = match[1]
            delta = offset2 - offset
            matchBins.add(delta)
    return matchBins.largestBin()


def matchFingerPrintFromFile(stream, matchFunction):
    """Try to find a song match for the fingerprint in the given file.

    What to match against is given by the matchFuntion, usually either
    matchesFromDB or matchesFromFile, it's just a function which takes a hash
    and returns a list of songId, offset pairs with the same hash."""
    bestMatch = 0
    # For each fingerprint in the file, get matching fingerprints.
    # Record the time delta between them, and the song id of the match.
    matches = {}
    for line in stream:
        fp = line.split('\t')
        hashVal = int(fp[0])
        offset = int(fp[1])
        results = matchFunction(hashVal)
        for row in results:
            songId = row[0]
            offset2 = row[1]
            delta = offset2 - offset
            if songId in matches:
                matches[songId].add(delta)
            else:
                matches[songId] = DeltaBin(BINSIZE)
                matches[songId].add(delta)
        
    maxDeltas = 0
    for songId in matches:
        maxBin = matches[songId].largestBin()
        if maxBin > maxDeltas and maxBin > MATCHTHRESHOLD:
            maxDeltas = maxBin
            bestMatch = songId
            
    #print "best match is %d with %d deltas in a bin" % (bestMatch, maxDeltas)
    return bestMatch


if __name__ == '__main__':
    #db = MySQLdb.connect("localhost", "noah", sys.argv[1], "pipes")
    #curs = db.cursor()
    filename1 = sys.argv[1]
    filename2 = sys.argv[2]
    stream1 = open(filename1, 'r')
    stream2 = open(filename2, 'r')
    print "%s\t%s\t%d" % (filename2, filename1,
            matchesBetweenFiles(stream1, stream2))
    #match = matchFingerPrintFromFile(stream, (lambda h: matchesFromFile(stream2, h)))
