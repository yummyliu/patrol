MODULE_big = commander

EXTENSION = commander
DATA = commander--1.0.sql

CXX = c++ -std=c++11 -I/usr/local/opt/openssl/include -Wno-deprecated-register

CFLAGS_SL += -lcurl -stdlib=libc++ -lcpprest -lssl -lcrypto -lboost_system -lboost_thread-mt -lboost_chrono-mt
SHLIB_LINK += -lstdc++
#PG_CPPFLAGS += -std=c++11
OBJS +=  command/command.o command/restapi.o commander.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
