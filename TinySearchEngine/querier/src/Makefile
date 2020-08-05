
CC=gcc 
CFLAGS= -g -Wall -pedantic -std=c99 
LOADCFLAGS= -g -Wall -pedantic 
EXEC=querier 
#INCLUDE= -I../include   ## needs to be reworked but currently the makefile doesn't recognize the included header files
SRCS= querier.c recreate.c hash.c saveClean.c allocate.c 
DEPS= querier.h recreate.h hash.h saveClean.h recreate.h 
OBJ= $(SRCS:.c=.o) 

%.o:%.c $(DEPS) 
	$(CC) -c -o $@ $< $(CFLAGS) 

all: $(EXEC) 
	@echo compiled..

$(EXEC): $(OBJ) 
	$(CC) $(CFLAGS) $(INCLUDE) -o $(EXEC) $(OBJ) 

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXEC) logger_querier.txt no_emplines_index.dat 

