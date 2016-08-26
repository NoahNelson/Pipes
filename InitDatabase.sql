/* create the necessary tables - one for song titles and metadata and one for
 * fingerprint hashes. */


CREATE TABLE songs (
    songId MEDIUMINT PRIMARY KEY NOT NULL,
    title VARCHAR(200) NOT NULL,
    CONSTRAINT uniqueSongId UNIQUE(songId)
);

CREATE TABLE fingerprints (
    songId MEDIUMINT NOT NULL,
    hash BIGINT NOT NULL,
    offset INT NOT NULL,
    CONSTRAINT uniqueEntries UNIQUE(songId, offset, hash)
);

CREATE INDEX indexHash on fingerprints (hash);
