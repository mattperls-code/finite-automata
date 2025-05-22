main: main.cpp finite_automata.cpp lib/parser.cpp
	g++ -std=c++23 -o main lib/parser.cpp finite_automata.cpp main.cpp

.PHONY: clean
clean:
	rm -rf main