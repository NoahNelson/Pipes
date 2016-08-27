import sys
import sqlite3

usageString = """
usage: python3 {} <sqlfile> <printfile>
""".format(sys.argv[0])

if __name__ == '__main__':

    if len(sys.argv) < 3:
        print(usageString)

    sqlfile = sys.argv[1]
    conn = sqlite3.connect(sqlfile)
    curs = conn.cursor()

    printfile = sys.argv[2]
    printstream = open(printfile, 'r')
    prints = [tuple(line.split(',')) for line in printstream]

    curs.executemany(
            'insert or ignore into fingerprints values (?, ?, ?)', prints)

    conn.commit()
    conn.close()
    printstream.close()
