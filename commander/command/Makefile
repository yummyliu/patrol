#######################
# Makefile
#######################

TARGET := command.a

# compile and lib parameter
CC      := g++
LIBS    :=
LDFLAGS :=
DEFINES :=
INCLUDE := -I /usr/local/pgsql/include/server
CFLAGS  :=
CXXFLAGS:= -std=c++11

# link parameter
AR  := ar
LIB := command.a

#link
$(LIB):command.o
	$(AR) -r $@ $^

#compile
command.o:command.cpp
	$(CC) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

# clean
clean:
	rm -fr *.o
