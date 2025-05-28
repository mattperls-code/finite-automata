#include <iostream>

#include "../src/finite_automata.hpp"

int main()
{
    // re -> lnfa

    auto input1 = RegularExpression::fromExpressionString("ab*(a+b(a+λ)) + (a + λ)");
    auto output1 = FiniteAutomata::re2lnfa(input1);
    
    input1.exportExpression("./results/re2lnfa", "input");
    output1.exportGraph("./results/re2lnfa", "output");

    // lnfa -> nfa

    auto input2 = FiniteAutomata::create(
        { "A", "B", "C", "D", "E", "F" },
        "A",
        { "A", "F" },
        {
            Edge("A", "C", {}),
            Edge("A", "C", 'a'),
            Edge("A", "D", 'a'),
            Edge("B", "D", 'b'),
            Edge("B", "F", 'b'),
            Edge("C", "A", 'b'),
            Edge("C", "E", 'b'),
            Edge("D", "F", 'a'),
            Edge("E", "A", {}),
            Edge("E", "B", 'a'),
            Edge("E", "C", 'b'),
            Edge("E", "D", 'a'),
            Edge("F", "F", 'a'),
        }
    );
    auto output2 = input2.lnfa2nfa();

    input2.exportGraph("./results/lnfa2nfa", "input");
    output2.exportGraph("./results/lnfa2nfa", "output");

    // nfa -> dfa

    auto input3 = FiniteAutomata::create(
        { "A", "B", "C", "D", "E" },
        "A",
        { "B", "D" },
        {
            Edge("A", "B", 'a'),
            Edge("A", "E", 'a'),
            Edge("A", "E", 'b'),
            Edge("B", "C", 'a'),
            Edge("B", "C", 'b'),
            Edge("B", "E", 'b'),
            Edge("C", "B", 'b'),
            Edge("C", "D", 'b'),
            Edge("E", "C", 'a'),
            Edge("E", "D", 'b'),
        }
    );
    auto output3 = input3.nfa2dfa();

    input3.exportGraph("./results/nfa2dfa", "input");
    output3.exportGraph("./results/nfa2dfa", "output");

    // dfa -> min dfa

    auto input4 = FiniteAutomata::create(
        { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11" },
        "0",
        { "0" },
        {
            Edge("0",  "0", '0'),
            Edge("0",  "1", '1'),
            Edge("1",  "2", '0'),
            Edge("1",  "3", '1'),
            Edge("2",  "4", '0'),
            Edge("2",  "5", '1'),
            Edge("3",  "6", '0'),
            Edge("3",  "7", '1'),
            Edge("4",  "8", '0'),
            Edge("4",  "9", '1'),
            Edge("5", "10", '0'),
            Edge("5", "11", '1'),
            Edge("6",  "0", '0'),
            Edge("6",  "1", '1'),
            Edge("7",  "2", '0'),
            Edge("7",  "3", '1'),
            Edge("8",  "4", '0'),
            Edge("8",  "5", '1'),
            Edge("9",  "6", '0'),
            Edge("9",  "7", '1'),
            Edge("10", "8", '0'),
            Edge("10", "9", '1'),
            Edge("11", "10", '0'),
            Edge("11", "11", '1')
        }
    );
    auto output4 = input4.dfa2minDfa();

    input4.exportGraph("./results/dfa2minDfa", "input");
    output4.exportGraph("./results/dfa2minDfa", "output");

    // dfa -> re

    auto input5 = FiniteAutomata::create(
        { "A", "B", "C", "D", "E", "F" },
        "A",
        { "F" },
        {
            Edge("A", "B", '0'),
            Edge("B", "C", '2'),
            Edge("B", "E", '1'),
            Edge("B", "F", '0'),
            Edge("C", "D", '0'),
            Edge("D", "B", {}),
            Edge("E", "B", {}),
        }
    );
    auto output5 = input5.lnfa2re();

    input5.exportGraph("./results/dfa2re", "input");
    output5.exportExpression("./results/dfa2re", "output");
    
    return 0;
};