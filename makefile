VPATH = include
$(shell mkdir -p build bin)

bin/lc3: build/lc3.o
	g++ -Wall build/lc3.o -o bin/lc3

build/lc3.o: src/lc3.cpp include/enums.hpp
	g++ -Wall -c src/lc3.cpp -o build/lc3.o -Iinclude

clean:
	rm -rf build/* bin/*