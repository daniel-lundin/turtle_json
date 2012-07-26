AR = ar
OBJS = jsonnode.o parser.o
CFLAGS = -Wall -pedantic -O2
CXX = clang++

test: $(OBJS) test.cpp
	$(CXX) $(OBJS) $(CFLAGS) test.cpp -o test.o

lib: $(OBJS)
	$(AR) rc libturtle_json.a $(OBJS)

singlefile:
	cat parser.h jsonnode.h parser.cpp  jsonnode.cpp | grep -v "#include \"" > turtle_json.cpp

tags:
	ctags -R --sort=1 --c++-kinds=+p --fields=+iaS --extra=+q --language-force=C++ -f cpp cpp_src

clean:
	rm -rf *.o
	rm -rf *.a
	rm -f testout.json
