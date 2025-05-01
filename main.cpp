#include <iostream>

#include "./finite_automata.hpp"

void testStringMatching()
{
    std::cout << "DFA to match binary strings which have remainder two mod three" << std::endl << std::endl;

    FiniteAutomata dfa = FiniteAutomata(
        { '0', '1' },
        { "A", "B", "C" },
        "A",
        { "C" },
        {
            NamedEdge("A", "A", TransitionToken::character('0')),
            NamedEdge("A", "B", TransitionToken::character('1')),

            NamedEdge("B", "C", TransitionToken::character('0')),
            NamedEdge("B", "A", TransitionToken::character('1')),

            NamedEdge("C", "B", TransitionToken::character('0')),
            NamedEdge("C", "C", TransitionToken::character('1')),
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

void testMinimizeDFA()
{
    FiniteAutomata dfa = FiniteAutomata(
        { '0', '1' },
        { "Q0", "Q1", "Q2", "Q3", "Q4", "Q5" },
        "Q0",
        { "Q1", "Q2", "Q4" },
        {
            NamedEdge("Q0", "Q3", TransitionToken::character('0')),
            NamedEdge("Q0", "Q1", TransitionToken::character('1')),

            NamedEdge("Q1", "Q2", TransitionToken::character('0')),
            NamedEdge("Q1", "Q5", TransitionToken::character('1')),

            NamedEdge("Q2", "Q2", TransitionToken::character('0')),
            NamedEdge("Q2", "Q5", TransitionToken::character('1')),

            NamedEdge("Q3", "Q0", TransitionToken::character('0')),
            NamedEdge("Q3", "Q4", TransitionToken::character('1')),

            NamedEdge("Q4", "Q2", TransitionToken::character('0')),
            NamedEdge("Q4", "Q5", TransitionToken::character('1')),

            NamedEdge("Q5", "Q5", TransitionToken::character('0')),
            NamedEdge("Q5", "Q5", TransitionToken::character('1'))
        }
    );

    FiniteAutomata minDFA = dfa.minimizeDFA();

    std::cout << minDFA.toString();
};

int main()
{
    // testStringMatching();
    testMinimizeDFA();
    
    return 0;
};