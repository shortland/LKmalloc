EXEC := testsuite

CC := gcc

SRCD := src
INCD := include
BLDD := build
TSTD := tests
MAIN := main
LIB := LKmalloc

CFLAGS := -Wall -Werror -Wno-unused-variable -Wno-unused-function -MMD
STD := -std=gnu11
CFLAGS += $(STD)
INC := -I $(INCD)

.PHONY: clean depend all tests

depend: setup
	cd build && $(CC) $(CFLAGS) -c *.c && ar -cvq $(LIB).a *.o && cd ..

$(BLDD):
	mkdir -p $(BLDD)

$(EXEC):
	$(CC) $(CFLAGS) -o $@ src/main.c $(BLDD)/$(LIB).a -I $(BLDD)/ 

all: depend $(EXEC)

setup: $(BLDD) readylk

readylk:
	cp -r ../$(LIB)/* $(BLDD)/

clean:
	rm -rf $(BLDD) $(EXEC)

.PRECIOUS: $(BLDD)/*.d
-include $(BLDD)/*.d