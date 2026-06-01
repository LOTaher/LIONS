all: lightctl

lightctl:
	gcc -Wall -Wextra -pedantic -std=gnu99 -g lightctl.c ../../lib/c/liblmp.c ../../lib/c/lmp.c ../../lib/c/lt_string.c ../../lib/c/lt_arena.c -o lightctl

clean:
	rm lightctl
