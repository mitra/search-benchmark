CFLAGS:=-Wall -Werror -g -I. -std=gnu99
CFLAGS+= -O2
LDFLAGS:=-g

all: test-memcmp

test-memcmp.c: ifunc-impl-list.h kmemcmp.h test-skeleton.c test-string.h
	touch $@

test-memcmp: test-memcmp.c

clean:
	$(RM) test-memcmp

test: test-memcmp
	./$<

.PHONY: all clean test
