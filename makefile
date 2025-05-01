main: main.cpp finite_automata.cpp
	clang++ -std=c++20 -o main finite_automata.cpp main.cpp

.PHONY: clean
clean:
	rm -rf main