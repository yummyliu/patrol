/* =============================================================================
#     FileName: commander.c
#         Desc: commander bgw for patrol
#       Author: LiuYangming
#        Email: sdwhlym@126.com
#     HomePage: http://yummyliu.github.io
#      Version: 0.0.1
#   LastChange: 2018-07-27 14:22:58
#      History:
============================================================================= */
#include "postgres.h"
#include "command/command.h"
#define TABLE_COUNT 1

void		_PG_init(void);
void		commander_main(Datum) pg_attribute_noreturn();

/* flags set by signal handlers */
static volatile sig_atomic_t got_sighup = false;
static volatile sig_atomic_t got_sigterm = false;

/* GUC variables */
static int	commander_naptime = 10;
static int commander_total_workers = 1;

/* Set a flag to let the main loop to terminate, and set our latch to wake it up.
 */
static void
commander_sigterm(SIGNAL_ARGS)
{
	int			save_errno = errno;

	got_sigterm = true;
	SetLatch(MyLatch);

	errno = save_errno;
}

/* Set a flag to tell the main loop to reread the config file, and set our latch to wake it up.
 */
static void
commander_sighup(SIGNAL_ARGS)
{
	int			save_errno = errno;

	got_sighup = true;
	SetLatch(MyLatch);

	errno = save_errno;
}



void
commander_main(Datum main_arg)
{
	pqsignal(SIGHUP, commander_sighup);
	pqsignal(SIGTERM, commander_sigterm);
	/* ready to receive signals */
	BackgroundWorkerUnblockSignals();
	/* Connect to our database */
	BackgroundWorkerInitializeConnection("postgres", NULL);

	// initialize_commander();
	elog(LOG, "%s initialized", MyBgworkerEntry->bgw_name);

	/* Main loop: do this until the SIGTERM handler tells us to terminate
	 */
	while (!got_sigterm)
	{
		int			rc;

		rc = WaitLatch(MyLatch,
					   WL_LATCH_SET | WL_TIMEOUT | WL_POSTMASTER_DEATH,
					   commander_naptime * 1000L,
					   PG_WAIT_EXTENSION);
		ResetLatch(MyLatch);

		/* emergency bailout if postmaster has died */
		if (rc & WL_POSTMASTER_DEATH)
			proc_exit(1);

		CHECK_FOR_INTERRUPTS();

		/*
		 * In case of a SIGHUP, just reload the configuration.
		 */
		if (got_sighup)
		{
			got_sighup = false;
			ProcessConfigFile(PGC_SIGHUP);
		}
	}

	proc_exit(1);
}

void
_PG_init(void)
{
	BackgroundWorker worker;

	/* get the configuration */
	DefineCustomIntVariable("commander.naptime",
							"Duration between each check (in seconds).",
							NULL,
							&commander_naptime,
							10,
							1,
							INT_MAX,
							PGC_SIGHUP,
							0,
							NULL,
							NULL,
							NULL);

	if (!process_shared_preload_libraries_in_progress)
		return;
	DefineCustomIntVariable("commander.total_workers",
							"Number of workers.",
							NULL,
							&commander_total_workers,
							1,
							1,
							100,
							PGC_POSTMASTER,
							0,
							NULL,
							NULL,
							NULL);

	/* set up common data for all our workers */
	memset(&worker, 0, sizeof(worker));
	worker.bgw_flags = BGWORKER_SHMEM_ACCESS |
		BGWORKER_BACKEND_DATABASE_CONNECTION;
	worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
	worker.bgw_restart_time = BGW_NEVER_RESTART;
	sprintf(worker.bgw_library_name, "commander");
	sprintf(worker.bgw_function_name, "commander_main");
	worker.bgw_notify_pid = 0;

	/* Now fill in worker-specific data, and do the actual registrations.
	 */
	for (int i = 1; i <= commander_total_workers; i++)
	{
		snprintf(worker.bgw_name, BGW_MAXLEN, "commander worker-%d", i);
		worker.bgw_main_arg = Int32GetDatum(i);

		RegisterBackgroundWorker(&worker);
	}
}
