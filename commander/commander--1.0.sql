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

insert into patrol.dbinfo(id,pip) values ('di:m','10.7.129.175');
insert into patrol.dbinfo(id,pip) values ('as:m','10.7.225.102');
insert into patrol.dbinfo(id,pip) values ('sh0:m','10.7.16.126');
insert into patrol.dbinfo(id,pip) values ('sh1:m','10.7.73.203');
insert into patrol.dbinfo(id,pip) values ('sh2:m','10.7.13.0');
insert into patrol.dbinfo(id,pip) values ('sh3:m','10.7.36.30');
insert into patrol.dbinfo(id,pip) values ('cr:m','10.7.70.220');
insert into patrol.dbinfo(id,pip) values ('ch:m','10.7.63.166');
insert into patrol.dbinfo(id,pip) values ('lc:m','10.7.145.167');
insert into patrol.dbinfo(id,pip) values ('mr:m','10.7.38.43');
insert into patrol.dbinfo(id,pip) values ('at:m','10.7.140.77');
insert into patrol.dbinfo(id,pip) values ('mm:m','10.7.30.131');
insert into patrol.dbinfo(id,pip) values ('mm:s','10.7.31.86');
insert into patrol.dbinfo(id,pip) values ('sh0:s','10.7.129.249');
insert into patrol.dbinfo(id,pip) values ('sh1:s','10.7.46.248');
insert into patrol.dbinfo(id,pip) values ('sh2:s','10.7.86.246');
insert into patrol.dbinfo(id,pip) values ('sh3:s','10.7.47.36');
insert into patrol.dbinfo(id,pip) values ('cr:s','10.7.112.225');
insert into patrol.dbinfo(id,pip) values ('ch:s','10.7.225.54');
insert into patrol.dbinfo(id,pip) values ('lc:s','10.7.92.181');
insert into patrol.dbinfo(id,pip) values ('mr:s','10.7.117.1');
insert into patrol.dbinfo(id,pip) values ('at:s','10.7.215.93');
insert into patrol.dbinfo(id,pip) values ('di:s','10.7.89.58');
insert into patrol.dbinfo(id,pip) values ('as:s','10.7.92.123');

CREATE OR REPLACE FUNCTION patrol.getdbinfo(cstring,cstring)
RETURNS TABLE (maxage bigint, diskusage float)
AS 'commander.so',
'getdbinfo'
LANGUAGE C;
