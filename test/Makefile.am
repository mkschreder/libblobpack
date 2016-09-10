@CODE_COVERAGE_RULES@
check_PROGRAMS=random read-write json parse
AM_CFLAGS=$(CODE_COVERAGE_CFLAGS) -Wall -Werror -fPIC -Wno-format-y2k -W -Wstrict-prototypes -Wmissing-prototypes \
-Wpointer-arith -Wreturn-type -Wcast-qual -Wwrite-strings -Wswitch \
-Wshadow -Wcast-align -Wchar-subscripts -Winline \
-Wnested-externs -Wredundant-decls -Wmissing-field-initializers -Wextra \
-Wformat=2 -Wno-format-nonliteral -Wpointer-arith -Wno-missing-braces \
-Wno-unused-parameter -Wno-unused-variable -Wno-inline -I../src/ -std=c99 -D_GNU_SOURCE 
random_SOURCES=random.c
random_CFLAGS=$(AM_CFLAGS) 
random_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs/ -lblobpack -lm
read_write_SOURCES=read-write.c
read_write_CFLAGS=$(AM_CFLAGS) 
read_write_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs/ -lblobpack -lm
json_SOURCES=json.c
json_CFLAGS=$(AM_CFLAGS)
json_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs/ -lblobpack -lm
parse_SOURCES=parse.c
parse_CFLAGS=$(AM_CFLAGS) 
parse_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs/ -lblobpack -lm
TESTS=$(check_PROGRAMS)
