main: main.cpp finite_automata.cpp
	g++ -std=c++23 -o main finite_automata.cpp main.cpp

.PHONY: clean
clean:
	rm -rf main