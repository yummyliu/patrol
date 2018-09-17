/* src/test/modules/worker_spi/worker_spi--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION commander" to load this file. \quit

CREATE SCHEMA patrol;
create table patrol.dbinfo(
	id text ,
	name text,
	pip text,
	pport int default 9999,
    primary key(id)
);

CREATE OR REPLACE FUNCTION patrol.getdbinfo(cstring,cstring)
RETURNS TABLE (maxage bigint, diskusage float)
AS 'commander.so',
'getdbinfo'
LANGUAGE C;

CREATE OR REPLACE FUNCTION patrol.getdbsinfo(cstring,cstring)
RETURNS TABLE (maxage bigint, diskusage float)
AS 'commander.so',
'getdbsinfo'
LANGUAGE C;
