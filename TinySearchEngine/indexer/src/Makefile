
CC=gcc 
CFLAGS= -g -Wall -pedantic -std=c99 
LOADCFLAGS= -g -Wall -pedantic 
EXEC=indexer
#INCLUDE= -I../include   ## needs to be reworked but currently the makefile doesn't recognize the included header files
SRCS= indexer.c hash.c loadDoc.c saveClean.c allocate.c 
DEPS= indexer.h hash.h loadDoc.h saveClean.h recreate.h 
OBJ= $(SRCS:.c=.o) 

RECREATE=recreate 
RECREATE_SRCS= recreate.c saveClean.c hash.c allocate.c 
RECREATE_OBJ= $(RECREATE_SRCS:.c=.o) 

%.o:%.c $(DEPS) 
	$(CC) -c -o $@ $< $(CFLAGS) 

all: $(EXEC) $(RECREATE) 
	@echo compiled..

$(EXEC): $(OBJ) 
	$(CC) $(CFLAGS) $(INCLUDE) -o $(EXEC) $(OBJ) 

$(RECREATE): $(RECREATE_OBJ) 
	$(CC) $(CFLAGS) -o $(RECREATE) $(RECREATE_OBJ) 

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXEC) $(RECREATE) $(TEST_OBJ) $(RECREATE_OBJ) logger_index.txt index.dat test_index.dat sorted_index.dat sorted_test_index.dat recreate_index_logger.txt no_emplines_index.dat 

