/* create the necessary tables - one for song titles and metadata and one for
 * fingerprint hashes. */

create table songs (
    songId mediumint unsigned not null auto_increment,
    title varchar(200) not null,
    primary key (songId),
    unique key songId (songId)
);

create table fingerprints (
    hash bigint unsigned not null,
    songId mediumint unsigned not null,
    offset int unsigned not null,
    index(hash),
    unique(songId, offset, hash)
);
