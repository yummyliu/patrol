MODULE_big = commander

EXTENSION = commander
DATA = commander--1.0.sql

CXX = c++ -std=c++11 -I/usr/local/include -I/usr/local/opt/openssl/include

#CFLAGS_SL += -stdlib=libc++ -lcpprest -lssl -lcrypto -lboost_system -lboost_thread-mt -lboost_chrono-mt
CFLAGS_SL +=  -fPIC
PG_CPPFLAGS +=  -fPIC
SHLIB_LINK += -lstdc++ -lcurl -ljsoncpp
#PG_CPPFLAGS += -std=c++11
OBJS +=  command/command.o command/httpclient.o commander.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
