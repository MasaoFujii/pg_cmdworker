#include "postgres.h"

#include <signal.h>

#include "fmgr.h"
#include "miscadmin.h"
#include "postmaster/bgworker.h"
#include "storage/ipc.h"
#include "utils/guc.h"

PG_MODULE_MAGIC;

void		_PG_init(void);
void		cmdworker_main(Datum) __attribute__((noreturn));

static char *cmdworker_command = NULL;

void
cmdworker_main(Datum main_arg)
{
	int			rc;

	rc = system(cmdworker_command);
	if (rc != 0)
	{
		/*
		 * If either the shell itself, or a called command, died on a signal,
		 * abort the archiver.  We do this because system() ignores SIGINT and
		 * SIGQUIT while waiting; so a signal is very likely something that
		 * should have interrupted us too.  If we overreact it's no big deal,
		 * the postmaster will just start the archiver again.
		 *
		 * Per the Single Unix Spec, shells report exit status > 128 when a
		 * called command died on a signal.
		 */
		int			lev = (WIFSIGNALED(rc) || WEXITSTATUS(rc) > 128) ? FATAL : LOG;

		if (WIFEXITED(rc))
		{
			ereport(lev,
					(errmsg("pg_cmdworker.command failed with exit code %d",
							WEXITSTATUS(rc)),
					 errdetail("The failed command was: %s",
							   cmdworker_command)));
		}
		else if (WIFSIGNALED(rc))
		{
#if defined(WIN32)
			ereport(lev,
				  (errmsg("pg_cmdworker.command was terminated by exception 0x%X",
						  WTERMSIG(rc)),
				   errhint("See C include file \"ntstatus.h\" for a description of the hexadecimal value."),
				   errdetail("The failed command was: %s",
							 cmdworker_command)));
#elif defined(HAVE_DECL_SYS_SIGLIST) && HAVE_DECL_SYS_SIGLIST
			ereport(lev,
					(errmsg("pg_cmdworker.command was terminated by signal %d: %s",
							WTERMSIG(rc),
			  WTERMSIG(rc) < NSIG ? sys_siglist[WTERMSIG(rc)] : "(unknown)"),
					 errdetail("The failed command was: %s",
							   cmdworker_command)));
#else
			ereport(lev,
					(errmsg("pg_cmdworker.command was terminated by signal %d",
							WTERMSIG(rc)),
					 errdetail("The failed command was: %s",
							   cmdworker_command)));
#endif
		}
		else
		{
			ereport(lev,
				(errmsg("pg_cmdworker.command exited with unrecognized status %d",
						rc),
				 errdetail("The failed command was: %s",
						   cmdworker_command)));
		}
	}

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
