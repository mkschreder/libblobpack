PACKAGE_NAME:=blobpack
SOURCE:=$(wildcard *.c)
HEADERS:=$(wildcard *.h)
OBJECTS:=$(patsubst %.c,%.o,$(SOURCE))
LDFLAGS+=
CFLAGS+=-g -Werror -Wall -Wno-unused-function -std=gnu99 -fPIC
prefix:=$(DESTDIR)/usr/

STATIC_LIB:=lib$(PACKAGE_NAME).a
SHARED_LIB:=lib$(PACKAGE_NAME).so

PUBLIC_HEADERS:=blobpack.h blob.h blob_field.h blob_json.h utils.h
HEADERS:=$(PUBLIC_HEADERS); 

all: $(STATIC_LIB) $(SHARED_LIB) simple-example random-test

$(STATIC_LIB): $(OBJECTS)
	$(AR) rcs -o $@ $^ 
	ranlib $@

$(SHARED_LIB): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $^

simple-example: examples/simple.c libblobpack.a
	$(CC) $(CFLAGS) -I. -o $@ $^ -L. -lblobpack -ljson-c -lm

random-test: tests/random.c libblobpack.a
	$(CC) $(CFLAGS) -I. -o $@ $^ -L. -lblobpack -ljson-c -lm

install: 
	mkdir -p $(prefix)/lib/ 
	mkdir -p $(prefix)/include/$(PACKAGE_NAME)
	cp -Rp $(STATIC_LIB) $(prefix)/lib/
	cp -Rp $(SHARED_LIB) $(prefix)/lib/
	cp -Rp $(PUBLIC_HEADERS) $(prefix)/include/$(PACKAGE_NAME)
	
clean: 
	rm -f *.o *.a *.so *-example *-test
