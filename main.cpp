#include <iostream>

#include "./finite_automata.hpp"

void testFiniteAutomataMatch()
{
    std::cout << "DFA to match binary strings which have remainder two mod three" << std::endl << std::endl;

    FiniteAutomata dfa = FiniteAutomata::create(
        { "A", "B", "C" },
        "A",
        { "C" },
        {
            Edge("A", "A", '0'),
            Edge("A", "B", '1'),

            Edge("B", "C", '0'),
            Edge("B", "A", '1'),

            Edge("C", "B", '0'),
            Edge("C", "C", '1'),
        }
    );

    std::cout << dfa.toString() << std::endl << std::endl;

    std::vector<std::string> testStrs = {
        "00101",
        "011",
        "100110",
        "0101",
        "11",
    };

    for (auto testStr : testStrs) std::cout << "String " << testStr << " in DFA: " << (dfa.matches(testStr) ? "true" : "false") << std::endl;
};

void testFiniteAutomataDfa2MinDfa()
{
    FiniteAutomata dfa = FiniteAutomata::create(
        { "Q0", "Q1", "Q2", "Q3", "Q4", "Q5", "Q6", "Q7" },
        "Q0",
        { "Q1", "Q2", "Q4" },
        {
            Edge("Q0", "Q3", '0'),
            Edge("Q0", "Q1", '1'),

            Edge("Q1", "Q2", '0'),
            Edge("Q1", "Q5", '1'),

            Edge("Q2", "Q2", '0'),
            Edge("Q2", "Q5", '1'),

            Edge("Q3", "Q0", '0'),
            Edge("Q3", "Q4", '1'),

            Edge("Q4", "Q2", '0'),
            Edge("Q4", "Q5", '1'),

            Edge("Q5", "Q5", '0'),
            Edge("Q5", "Q5", '1'),

            Edge("Q6", "Q7", '0'),
            Edge("Q6", "Q3", '1'),

            Edge("Q7", "Q0", '0'),
            Edge("Q7", "Q6", '1'),
        }
    );

    // FiniteAutomata dfa = FiniteAutomata(
    //     { "i", "p", "q", "r", "s", "t", "f", "rt", "psf", "0" },
    //     "i",
    //     { "i", "f" },
    //     {
    //         Edge("i", "q", 'a'),
    //         Edge("i", "rt", 'b'),
    //         Edge("p", "q", 'a'),
    //         Edge("p", "r", 'b'),
    //         Edge("q", "0", 'a'),
    //         Edge("q", "psf", 'b'),
    //         Edge("r", "psf", 'a'),
    //         Edge("r", "0", 'b'),
    //         Edge("s", "q", 'a'),
    //         Edge("s", "r", 'b'),
    //         Edge("t", "0", 'a'),
    //         Edge("t", "f", 'b'),
    //         Edge("f", "0", 'a'),
    //         Edge("f", "0", 'b'),
    //         Edge("rt", "psf", 'a'),
    //         Edge("rt", "f", 'b'),
    //         Edge("psf", "q", 'a'),
    //         Edge("psf", "r", 'b')
    //     }
    // );

    std::cout << "Initial DFA:" << std::endl;
    std::cout << dfa.toString() << std::endl << std::endl;

    FiniteAutomata minDFA = dfa.dfa2minDfa();

    std::cout << "Minimized DFA:" << std::endl;
    std::cout << minDFA.toString() << std::endl;
};

void testFiniteAutomataNfa2Dfa()
{
    FiniteAutomata nfa = FiniteAutomata::create(
        { "A", "B", "C", "D" },
        "A",
        { "A", "B" },
        {
            Edge("A", "A", '0'),
            Edge("A", "B", '0'),
            Edge("A", "C", '1'),

            Edge("B", "A", '1'),
            Edge("B", "B", '1'),
            Edge("B", "D", '0'),

            Edge("C", "B", '0'),
            Edge("C", "C", '1'),
            Edge("C", "D", '1'),

            Edge("D", "B", '0'),
            Edge("D", "B", '1'),
            Edge("D", "C", '1'),
        }
    );

    std::cout << "Initial NFA:" << std::endl;
    std::cout << nfa.toString() << std::endl << std::endl;

    FiniteAutomata dfa = nfa.nfa2dfa();

    std::cout << "DFA:" << std::endl;
    std::cout << dfa.toString() << std::endl;
};

void testFiniteAutomataKleeneCycle()
{
    FiniteAutomata nfa = FiniteAutomata::create(
        { "A", "B", "C", "D" },
        "A",
        { "A", "B" },
        {
            Edge("A", "A", '0'),
            Edge("A", "B", '0'),
            Edge("A", "C", '1'),

            Edge("B", "A", '1'),
            Edge("B", "B", '1'),
            Edge("B", "D", '0'),

            Edge("C", "B", '0'),
            Edge("C", "C", '1'),
            Edge("C", "D", '1'),

            Edge("D", "B", '0'),
            Edge("D", "B", '1'),
            Edge("D", "C", '1'),
        }
    );

    std::cout << "Initial NFA:" << std::endl;
    std::cout << nfa.toString() << std::endl << std::endl;

    FiniteAutomata dfa = nfa.nfa2dfa();

    std::cout << "DFA:" << std::endl;
    std::cout << dfa.toString() << std::endl << std::endl;

    FiniteAutomata minDfa = dfa.dfa2minDfa();

    std::cout << "Min DFA:" << std::endl;
    std::cout << minDfa.toString() << std::endl;
};

int main()
{
    // testFiniteAutomataMatch();
    // testFiniteAutomataDfa2MinDfa();
    // testFiniteAutomataNfa2Dfa();
    testFiniteAutomataKleeneCycle();
    
    return 0;
};