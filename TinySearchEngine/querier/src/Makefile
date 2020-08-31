
CC=gcc 
CFLAGS= -g -Wall -pedantic -std=c99 
LOADCFLAGS= -g -Wall -pedantic 
EXEC=querier 
SRCS= main.c query.c recreate.c hash.c clean.c allocate.c utils.c and.c 
DEPS= main.c query.h recreate.h hash.h clean.h recreate.h utils.c and.c 
OBJ= $(SRCS:.c=.o) 

%.o:%.c $(DEPS) 
	$(CC) -c -o $@ $< $(CFLAGS) 

all: $(EXEC) 
	@echo compiled..

$(EXEC): $(OBJ) 
	$(CC) $(CFLAGS) $(INCLUDE) -o $(EXEC) $(OBJ) 

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXEC) logger_querier.txt 

