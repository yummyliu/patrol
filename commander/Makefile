# src/test/modules/worker_spi/Makefile

MODULES = commander

EXTENSION = commander
DATA = commander--1.0.sql
PGFILEDESC = "commander - background worker for patrol"

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = src/test/modules/commander
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
