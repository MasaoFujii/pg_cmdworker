#include "postgres.h"

PG_MODULE_MAGIC;

void		_PG_init(void);
void		cmdworker_main(Datum) __attribute__((noreturn));

static char *cmdworker_command = NULL;

void
cmdworker_main(Datum main_arg)
{
	proc_exit(0);
}

void
_PG_init(void)
{
	BackgroundWorker worker;

	if (!process_shared_preload_libraries_in_progress)
		return;

	DefineCustomStringVariable("pg_cmdworker.command",
							   "Sets the shell command to run.",
							   NULL,
							   &cmdworker_command,
							   NULL,
							   PGC_POSTMASTER,
							   0,
							   NULL,
							   NULL,
							   NULL);

	EmitWarningsOnPlaceholders("pg_cmdworker");

	worker.bgw_flags = 0;
	worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
	worker.bgw_restart_time = BGW_NEVER_RESTART;
	worker.bgw_main = cmdworker_main;
	worker.bgw_notify_pid = 0;
	snprintf(worker.bgw_name, BGW_MAXLEN, "pg_cmdworker");

	RegisterBackgroundWorker(&worker);
}
