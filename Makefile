MODULES = pg_cmdworker

EXTENSION = pg_cmdworker
PGFILEDESC = "pg_cmdworker - make bgworker run the command"

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_cmdworker
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
