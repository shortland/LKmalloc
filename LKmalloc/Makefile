TARGET = prog

$(TARGET): main.o LKmalloc.a
	gcc $^ -o $@

main.o: main.c
	gcc -c $< -o $@

LKmalloc.a: LKmalloc.o Tables.o
	ar rcs $@ $^

LKmalloc.o: LKmalloc.c Tables.c Tables.h Flags.h
	gcc -c -o $@ $<

Tables.o: Tables.c Tables.h Flags.h
	gcc -c -o $@ $<

clean:
	rm -f *.o *.a $(TARGET)
