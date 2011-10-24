AR = ar
OBJS = jsonnode.o parser.o
CFLAGS = -Wall -pedantic -O2

test: $(OBJS) test.cpp
	g++ $(OBJS) $(CFLAGS) test.cpp -o test.o

lib: $(OBJS)
	$(AR) rc turtle_json.a $(OBJS)

tags:
	ctags -R --sort=1 --c++-kinds=+p --fields=+iaS --extra=+q --language-force=C++ -f cpp cpp_src

clean:
	rm -rf *.o
	rm -rf *.a
	rm -f testout.json
