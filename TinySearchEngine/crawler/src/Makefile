CC=gcc 
CFLAGS= -g -Wall -pedantic -std=c99 
EXEC=crawler 
INCLUDE=-I/Users/tomtom/Documents/C/cs50/TinySearchEngine/include 
SRCS= crawler.c hash.c gumbo.c 
DEPS= ../include/crawler.h ../include/hash.h ../include/my_gumbo.h 
OBJ= $(SRCS:.c=.o) 

%.o:%c $(DEPS) 
	$(CC) -c -o $@ $< $(CFLAGS) 

all: $(EXEC)
	@echo compiled..

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(EXEC) -lgumbo $(OBJ) 

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXEC) logger_crawler.txt 
