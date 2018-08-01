#######################
# Makefile
#######################

# target
TARGET := libcommander.so

# compile and lib parameter
CC      := gcc
LDFLAGS := -L/usr/local/pgsql/lib -Wl,-dead_strip_dylibs   -bundle -bundle_loader /usr/local/pgsql/bin/postgres
LIBS    := ./command/command.a
DEFINES :=
INCLUDE:= -I. -I./ -I/usr/local/pgsql/include/server -I/usr/local/pgsql/include/internal
CFLAGS  :=
CXXFLAGS:=

# link
$(TARGET): commander.o $(LIBS)
	$(CC) -fpic $(INCLUDE) $(LDFLAGS) -o $@ $^
#compile
commander.o: commander.c
	$(CC) $(INCLUDE) -fno-strict-aliasing -fwrapv -c $^ -o $@

# clean
clean:
	rm -fr *.o
	rm -fr $(TARGET)
