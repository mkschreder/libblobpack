all: libblob.a libblob.so simple-example

SOURCE:=$(wildcard *.c)
HEADERS:=$(wildcard *.h)
OBJECTS:=$(patsubst %.c,%.o,$(SOURCE))
LDFLAGS+=-ljson-c
CFLAGS+=-g -std=gnu99 -fPIC

libblob.a: $(OBJECTS)
	$(AR) rcs -o $@ $^ 

libblob.so: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $^

simple-example: examples/simple.c libblob.a
	$(CC) $(CFLAGS) -I. -o $@ $^ -L. -lblob -ljson-c

clean: 
	rm -f *.o *.a *.so
