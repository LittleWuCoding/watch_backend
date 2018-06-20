/* -------------------------------------------------------------------------
 *
 * watch_backend.c
 *
 * Copyright (c) 2010-2017, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *		contrib/watch_backend/watch_backend.c
 *
 * -------------------------------------------------------------------------
 */

#include "postgres.h" 
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


PG_MODULE_MAGIC;

void		_PG_init(void);
void		watch_backend_main(Datum) pg_attribute_noreturn();

/* flags set by signal handlers */
static volatile sig_atomic_t got_sighup = false;
static volatile sig_atomic_t got_sigterm = false;


/* GUC Variables */





/*
 * Signal handler for SIGTERM
 *		Set a flag to let the main loop to terminate, and set our latch to wake
 *		it up.
 */
static void
worker_spi_sigterm(SIGNAL_ARGS)
{
	int			save_errno = errno;

	got_sigterm = true;
	SetLatch(MyLatch);

	errno = save_errno;
}

/*
 * Signal handler for SIGHUP
 *		Set a flag to tell the main loop to reread the config file, and set
 *		our latch to wake it up.
 */
static void
worker_spi_sighup(SIGNAL_ARGS)
{
	int			save_errno = errno;

	got_sighup = true;
	SetLatch(MyLatch);

	errno = save_errno;
}


/*
 * Entry point 
 */
void
watch_backend_main(Datum main_arg)
{
	
	/* Establish signal handlers before unblocking signals. */
	pqsignal(SIGHUP, worker_spi_sighup);
	pqsignal(SIGTERM, worker_spi_sigterm);
	
	/* We're now ready to receive signals */
	BackgroundWorkerUnblockSignals();
	
	elog(LOG,"HELLO,I am a new backend processxxx");
	
	while(!got_sigterm)
	{
		elog(LOG,"HELLO,I am a new backend process");
	}
	
	proc_exit(1);
}
 
/*
 * Module Load Callback
 */
void
_PG_init(void)
{
	BackgroundWorker worker;
	
		/* set up common data for all our workers */
	memset(&worker, 0, sizeof(worker));
	worker.bgw_flags = BGWORKER_SHMEM_ACCESS |
		BGWORKER_BACKEND_DATABASE_CONNECTION;
	worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
	worker.bgw_restart_time = BGW_NEVER_RESTART;
	sprintf(worker.bgw_library_name, "watch_backend");
	sprintf(worker.bgw_function_name, "watch_backend_main");
	worker.bgw_notify_pid = 0;
	
	RegisterBackgroundWorker(&worker);
}
