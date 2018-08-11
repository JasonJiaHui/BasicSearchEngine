all:a3search clean
a3search:a3search.o stmr.o tool.o
	g++ -O3 -o a3search a3search.o stmr.o tool.o
a3search.o:a3search.cpp
	g++ -c a3search.cpp stmr.c -std=c++11
tool.o:tool.cpp
	g++ -c tool.cpp -std=c++11
clean:
	rm *.o
	tar xvf words.tar