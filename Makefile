MODULES = pg_commnad_worker

EXTENSION = pg_commnad_worker
PGFILEDESC = "pg_commnad_worker - make bgworker start the command"

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_commnad_worker
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
