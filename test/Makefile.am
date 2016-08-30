@CODE_COVERAGE_RULES@
check_PROGRAMS=random read-write
random_SOURCES=random.c
random_CFLAGS=$(CODE_COVERAGE_CFLAGS) -I../src/ -std=c99 
random_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs/ -lblobpack -lm
read_write_SOURCES=read-write.c
read_write_CFLAGS=$(CODE_COVERAGE_CFLAGS) -I../src/ -D_GNU_SOURCE -std=c99 
read_write_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs/ -lblobpack -lm
TESTS=$(check_PROGRAMS)
