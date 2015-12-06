all: libblob.a libblob.so

SOURCE:=$(wildcard *.c)
HEADERS:=$(wildcard *.h)
OBJECTS:=$(patsubst %.c,%.o,$(SOURCE))
LDFLAGS+=-ljson-c
CFLAGS+=-fPIC

libblob.a: $(OBJECTS)
	$(AR) rcs -o $@ $^ 

libblob.so: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $^

clean: 
	rm -f *.o
