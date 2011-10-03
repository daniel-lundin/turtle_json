OBJS = jsonnode.o parser.o

test: $(OBJS) test.cpp
	g++ $(OBJS) test.cpp -o test.o

tags:
	ctags -R --sort=1 --c++-kinds=+p --fields=+iaS --extra=+q --language-force=C++ -f cpp cpp_src

clean:
	rm -rf *.o
