all: 
	g++ -std=c++11 example.cpp -o example
clean:
	rm -f example
install:
	mkdir -p /usr/local/include/argparse4cpp
	cp -f argparse.hpp /usr/local/include/argparse4cpp/
