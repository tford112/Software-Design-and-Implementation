
CC=gcc 
CFLAGS= -Wall -pedantic -std=c99
EXEC=prs 
INCLUDE=-I/Users/tomtom/Documents/C/cs50/TinySearchEngine/include
OBJ=crawler.o hash.o 

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(EXEC) $(OBJ)

crawler.o: src/crawler.c include/crawler.h include/html.h include/hash.h include/header.h 
	$(CC) $(CFLAGS) $(INCLUDE) -c $? 

hash.o: src/hash.c include/hash.h 
	$(CC) $(CFLAGS) $(INCLUDE) -c $? 

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXEC) 
