all: cps

CC=mpicxx
CPPFLAGS=-Wall -Wextra -std=c++11

cps: cps.cpp
	$(CC) $(CPPFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f cps
