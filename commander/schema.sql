create table patrol_info (
    id integer,
    hostname text,
    port integer
);

create table db_stats (
    id bigint,
    dbname text,
	maxage bigint,
	diskusage float,
	walsize_gb integer
);

create table table_info (
	--- XXX
    id integer,
    tablename text,
    schemaname text
);

create table func_info (
	--- XXX
    id integer,
    funcname text,
    schemaname text
);

create table table_stats (
	--- XXX
    id integer,
    seq_scan bigint,
    index_scan bigint
);

create table func_stats (
	--- XXX
    id integer,
    total_time bigint,
    total_call bigint
);
