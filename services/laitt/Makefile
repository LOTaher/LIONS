all: laitt

laitt:
	gcc -Wall -Wextra -pedantic -std=gnu99 -g laitt.c -I/opt/homebrew/include -L/opt/homebrew/lib ../../lib/c/liblmp.c ../../lib/c/lmp.c -lmosquitto -o laitt

clean:
	rm ./laitt
