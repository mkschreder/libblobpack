@CODE_COVERAGE_RULES@
includedir=$(prefix)/include/blobpack/
lib_LTLIBRARIES=libblobpack.la
include_HEADERS=blobpack.h blob.h blob_field.h blob_json.h utils.h
libblobpack_la_SOURCES=blob.c blob_field.c blob_json.c blob_ujson.c ujsondec.c ujsonenc.c utils.c
libblobpack_la_CFLAGS=$(CODE_COVERAGE_CFLAGS) -std=gnu99 -Wall -Werror
