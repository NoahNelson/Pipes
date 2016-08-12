# PrintMatcher.py
# code for matching song fingerprints.
# Takes a recorded fingerprint through a file or stdio and compares it to a
# database, trying to find a likely match.
import argparse
import MySQLdb
import sys

BINSIZE = 10
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

def matchFingerPrintFromFile(filename, dbCursor):
    """Try to find a song match for the fingerprint in the given file."""
    bestMatch = 0
    with open(filename, 'r') as f:
        # For each fingerprint in the file, get matching fingerprints.
        # Record the time delta between them, and the song id of the match.
        matches = {}
        for line in f:
            fp = line.split('\t')
            hashVal = int(fp[0])
            offset = int(fp[1])
            # TODO: error handling here
            dbCursor.execute(
                    "SELECT songId, offset FROM fingerprints \
                     WHERE hash = %d" % hashVal)
            results = dbCursor.fetchall()
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
            
    return bestMatch


if __name__ == '__main__':
    db = MySQLdb.connect("localhost", "noah", sys.argv[1], "pipes")
    curs = db.cursor()
    filename = sys.argv[2]
    match = matchFingerPrintFromFile(filename, curs)
    print match
