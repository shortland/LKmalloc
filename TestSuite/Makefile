EXEC := testsuite

CC := gcc

SRCD := src
BLDD := build
TSTD := tests

.PHONY: clean depend all tests

depend: setup $(BLDD)/LKmalloc.a

$(BLDD)/LKmalloc.a: $(BLDD)/LKmalloc.o $(BLDD)/Tables.o
	ar rcs $@ $^

$(BLDD)/LKmalloc.o: $(BLDD)/LKmalloc.c $(BLDD)/Tables.c $(BLDD)/Tables.h $(BLDD)/Flags.h
	gcc -c -o $@ $<

$(BLDD)/Tables.o: $(BLDD)/Tables.c $(BLDD)/Tables.h $(BLDD)/Flags.h
	gcc -c -o $@ $<

all: setup $(EXEC)

setup: $(BLDD) readylib

readylk:
	mv ../LKmalloc/* $(BLDD)/

$(BLDD):
	mkdir -p $(BLDD)

