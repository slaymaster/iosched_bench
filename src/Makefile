CC=gcc
CFLAGS=-Wall -g -pedantic -std=gnu99
LDFLAGS=
SOURCES=iosched.c
OBJECTS=$(SOURCES:.c=.o)
DEPS=
EXECUTABLE=iosched

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ -lpthread

.c.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -rf *.o
