# src/test/modules/worker_spi/Makefile

MODULES = commander

EXTENSION = commander
DATA = commander--1.0.sql
PGFILEDESC = "commander - patrol commander worker"

ifdef NOT_USE_PGXS
subdir = src/test/modules/commander
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
else
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
endif
