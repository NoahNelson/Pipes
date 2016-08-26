# PrintMatcher.py
# code for matching song fingerprints.
# Takes a recorded fingerprint through a file and compares it to a file or
# database, trying to find a likely match.
import argparse
import sqlite3
import sys
from os.path import splitext

BINSIZE = 5
MATCHTHRESHOLD = 100

############################################################################
### DeltaBin class
###

class DeltaBin:
    """
    A space-efficient way to create a time-delta histogram, only keeping the
    information the matching algorithm needs.
    """

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

############################################################################

def hashMatchesFromDB(dbCursor, hashVal):
    """Get matches of a hash from a database connection."""
    #TODO: error handling and transition to sqlite
    dbCursor.execute(
            "SELECT songId, offset FROM fingerprints \
             WHERE hash = %d" % hashVal)
    return dbCursor.fetchall()

def hashMatchesFromFile(stream, hashVal, songId=0):
    """Get matches of a hash from a given file stream."""
    results = []
    for line in stream:
        fp = line.split(',')
        hashVal2 = int(fp[0])
        timeOffset = int(fp[1])
        if hashVal2 == hashVal:
            results.append(timeOffset)
    stream.seek(0)
    return results

def matchesBetweenFiles(snippetStream, masterStream):
    """Gets the number of time delta matches in the largest delta bin made
    of matches between the fingerprints in two files."""
    # matching stream 1 against stream 2
    matchBins = DeltaBin(BINSIZE)
    for line in snippetStream:
        fp = line.split(',')
        hashVal = int(fp[0])
        offset = int(fp[1])
        matches = hashMatchesFromFile(masterStream, hashVal)
        for offset2 in matches:
            delta = offset2 - offset
            matchBins.add(delta)
    return matchBins.largestBin()


def matchesInDB(snippetStream, dbCursor):
    """Try to find a song match for the fingerprint in the given sqlite
    database. Returns a dictionary of songId: mostMatches pairs."""

    # For each fingerprint in the file, get matching fingerprints.
    # Record the time delta between them, and the song id of the match.
    matches = {}
    for line in snippetStream:
        fp = line.split(',')
        hashVal = int(fp[0])
        offset = int(fp[1])
        results = hashMatchesFromDB(dbCursor, hashVal)
        for row in results:
            songId = row[0]
            offset2 = row[1]
            delta = offset2 - offset
            if songId in matches:
                matches[songId].add(delta)
            else:
                matches[songId] = DeltaBin(BINSIZE)
                matches[songId].add(delta)
        
    return {songId: bins.largestBin() for songId, bins in matches.items()}


############################################################################

usageString = """Usage: python3 {fn} <snippetFile> <masterFile | sqliteFile>

Match a file of snippet fingerprints against a master file or sqlite database
of fingerprinted full songs. SnippetFile is a csv file of hash, timeWindow
pairs, and masterFile is either a csv file of hash, timeWindow pairs or
a sqlite database file.""".format(fn=sys.argv[0])

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(usageString)
        exit(1)

    snippetFilename = sys.argv[1]
    masterFilename = sys.argv[2]

    snippetStream = open(snippetFilename, 'r')

    _, ext = splitext(masterFilename)
    if ext == '.sqlite':
        conn = sqlite3.connect(masterFilename)
        curs = conn.cursor()
        
        dbMatches = matchesInDB(snippetStream, curs)
        print("{snip} against {db}".format(snip=snippetFilename,
                db=masterFilename))
        print("SongId:\tMatches:")
        for songId, matches in dbMatches.items():
            print("{id}\t{num}".format(id=songId, num=matches))
        snippetStream.close()

    elif ext == '.csv':
        masterStream = open(masterFilename, 'r')
        matches = matchesBetweenFiles(snippetStream, masterStream)
        print("{mf}\t{sf}\t{matches}".format(mf=masterFilename,
                        sf=snippetFilename, matches=matches))
        masterStream.close()
        snippetStream.close()

    else:
        print(usageString)

