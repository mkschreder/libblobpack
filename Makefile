all: libblobpack.a libblobpack.so simple-example random-test

SOURCE:=$(wildcard *.c)
HEADERS:=$(wildcard *.h)
OBJECTS:=$(patsubst %.c,%.o,$(SOURCE))
LDFLAGS+=-ljson-c
CFLAGS+=-std=gnu99 -fPIC

libblobpack.a: $(OBJECTS)
	$(AR) rcs -o $@ $^ 

libblobpack.so: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $^

simple-example: examples/simple.c libblobpack.a
	$(CC) $(CFLAGS) -I. -o $@ $^ -L. -lblobpack -ljson-c

random-test: tests/random.c libblobpack.a
	$(CC) $(CFLAGS) -I. -o $@ $^ -L. -lblobpack -ljson-c

clean: 
	rm -f *.o *.a *.so *-example *-test
