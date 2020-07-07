
CC=gcc 
CFLAGS= -g -Wall -pedantic -std=c99 
LOADCFLAGS= -g -Wall -pedantic 
EXEC=indexer
#INCLUDE= -I../include   ## needs to be reworked but currently the makefile doesn't recognize the included header files
SRCS= indexer.c hash.c loadDoc.c 
DEPS= indexer.h hash.h loadDoc.h 
#LFLAGS= -L/Users/tomtom/.pyenv/versions/3.8.3/lib/python3.8/config-3.8-darwin -lintl -ldl -framework CoreFoundation
OBJ= $(SRCS:.c=.o) 

%.o:%.c $(DEPS) 
	$(CC) -c -o $@ $< $(CFLAGS) 

all: $(EXEC)
	@echo compiled..
	
$(EXEC): $(OBJ) 
	$(CC) $(CFLAGS) $(INCLUDE) $(LFLAGS) -o $(EXEC) $(OBJ) 

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXEC) logger_index.txt 
