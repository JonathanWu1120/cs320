FLAGS = -g -Wl,--no-as-needed -ldl -Wall -pedantic -Wextra -std=c++14

all: clean test

test: main.o
	g++ $(FLAGS) main.o -o cache-sim

main.o: main.cpp
	g++ $(FLAGS) -c main.cpp -o main.o

miggy: miggy.o
	g++ $(FLAGS) main2.o -o miggy

miggy.o: main2.cpp
	g++ $(FLAGS) -c main2.cpp -o main2.o

clean:
	rm -f *.o cache-sim
