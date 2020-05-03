CC=gcc
CXX=g++
CFLAGS=-g
DL_FLAGS=-shared -fPIC -ldl

SANDBOX=sandbox sandbox.so

all: $(SANDBOX)

sandbox.so: inject.c
	$(CC) -o $@ $< $(DL_FLAGS)

%: %.cpp
	$(CXX) -o $@ $<

.PHONY: clean
clean:
	rm -f $(SANDBOX)
