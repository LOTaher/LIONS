all: laitt

laitt:
	gcc -Wall -Wextra -pedantic -std=gnu99 -g laitt.c -I/opt/homebrew/include -L/opt/homebrew/lib ../../lib/c/liblmp.c ../../lib/c/lmp.c ../../lib/c/lt_arena.c ../../lib/c/lt_string.c -lmosquitto -o laitt

clean:
	rm ./laitt
