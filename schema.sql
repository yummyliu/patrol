create table patrol_info (
    id integer,
    hostname text,
    port integer
);

create table table_info (
    id integer,
    tablename text,
    schemaname text
);

create table func_info (
    id integer,
    funcname text,
    schemaname text
);

create table db_stats (
    id integer,
    dbname text,
);

create table table_stats (
    id integer,
    seq_scan bigint,
    index_scan bigint
);

create table func_stats (
    id integer,
    total_time bigint,
    total_call bigint
);
