@CODE_COVERAGE_RULES@
includedir=$(prefix)/include/blobpack/
lib_LTLIBRARIES=libblobpack.la
include_HEADERS=blobpack.h blob.h blob_field.h blob_json.h 
libblobpack_la_SOURCES=blob.c blob_field.c blob_json.c blob_ujson.c ujsondec.c ujsonenc.c ieee754.c
libblobpack_la_CFLAGS=$(CODE_COVERAGE_CFLAGS) -std=gnu99 -Wall -Werror -Wno-format-y2k -W -Wstrict-prototypes -Wmissing-prototypes \
-Wpointer-arith -Wreturn-type -Wcast-qual -Wwrite-strings -Wswitch \
-Wshadow -Wcast-align -Wchar-subscripts -Winline \
-Wnested-externs -Wredundant-decls -Wmissing-field-initializers -Wextra \
-Wformat=2 -Wno-format-nonliteral -Wpointer-arith -Wno-missing-braces \
-Wno-unused-parameter -Wno-unused-variable -Wno-inline

