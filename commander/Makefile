MODULE_big = commander

EXTENSION = commander
DATA = commander--1.0.sql

CXX = c++ -std=c++11

SHLIB_LINK += -lstdc++
#PG_CPPFLAGS += -std=c++11
OBJS += commander.o command/command.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
