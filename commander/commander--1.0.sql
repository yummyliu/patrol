/* src/test/modules/worker_spi/worker_spi--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION commander" to load this file. \quit

CREATE SCHEMA patrol;
create table patrol.dbinfo(id int, name text, pip text, pport int);
insert into patrol.dbinfo values (1,'testdb','localhost',3000);
CREATE OR REPLACE FUNCTION getdbinfo(int)
    RETURNS cstring
AS 'commander.so',
'getdbinfo'
LANGUAGE C;

