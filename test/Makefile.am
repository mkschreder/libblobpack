@CODE_COVERAGE_RULES@
check_PROGRAMS=random read-write json parse
random_SOURCES=random.c
random_CFLAGS=$(CODE_COVERAGE_CFLAGS) -I../src/ -std=c99 
random_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs/ -lblobpack -lm
read_write_SOURCES=read-write.c
read_write_CFLAGS=$(CODE_COVERAGE_CFLAGS) -I../src/ -D_GNU_SOURCE -std=c99 
read_write_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs/ -lblobpack -lm
json_SOURCES=json.c
json_CFLAGS=$(CODE_COVERAGE_CFLAGS) -I../src/ -D_GNU_SOURCE -std=c99 
json_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs/ -lblobpack -lm
parse_SOURCES=parse.c
parse_CFLAGS=$(CODE_COVERAGE_CFLAGS) -I../src/ -D_GNU_SOURCE -std=c99 
parse_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs/ -lblobpack -lm
TESTS=$(check_PROGRAMS)
