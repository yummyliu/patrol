#ifndef COMMAND_H
#define COMMAND_H

/* These are always necessary for a bgworker */
#include "miscadmin.h"
#include "postmaster/bgworker.h"
#include "storage/ipc.h"
#include "storage/latch.h"
#include "storage/lwlock.h"
#include "storage/proc.h"
#include "storage/shmem.h"

/* these headers are used by this particular worker's code */
#include "access/xact.h"
#include "executor/spi.h"
#include "fmgr.h"
#include "lib/stringinfo.h"
#include "pgstat.h"
#include "utils/builtins.h"
#include "utils/snapmgr.h"
#include "tcop/utility.h"

#define TABLE_COUNT 6

void		_PG_init(void);
void		commander_main(Datum) pg_attribute_noreturn();

/* flags set by signal handlers */
static volatile sig_atomic_t got_sighup = false;
static volatile sig_atomic_t got_sigterm = false;

/* GUC variables */
static int	commander_naptime = 10;
static int	commander_total_workers = 1;

typedef struct table
{
    const char *name;
    const char *create_sql;
}table;

typedef struct worktable
{
	const char *schema;
	const table ts[TABLE_COUNT];
} worktable;

/*
 * all meta table and statistics data table in commander db
 */
const worktable wt = {"commander",{
                            {"db_info","create table db_info(hostname text, dbname text)"},\
                            {"table_info","create table table_info(relid oid, tablename text)"},\
                            {"func_info","create table func_info(funcid oid, funcname text)"},\
                            {"db_report","create table db_report(dbname text, create_time timestamp)"},\
                            {"func_report","create table func_report(create_time timestamp)"},\
                            {"table_report","create table table_report(create_time timestamp)"},\
                            }};

#endif /* ifndef COMMAND_H */
