#include <iostream>

#include "../src/finite_automata.hpp"

int main()
{
    // str -> re

    auto input1 = "a (b (b* + a + λ) + λ(a + (ab + b + λ)* bb)) b(ab)*";
    auto output1 = RegularExpression::fromExpressionString(input1);

    output1.exportExpression("./examples/str2re", "output");

    // re -> lnfa

    auto input2 = RegularExpression::fromExpressionString("ab*(a+b(a+λ)) + (a + λ)");
    auto output2 = FiniteAutomata::re2lnfa(input2);
    
    input2.exportExpression("./examples/re2lnfa", "input");
    output2.exportGraph("./examples/re2lnfa", "output");

    // lnfa -> nfa

    auto input3 = FiniteAutomata::create(
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
    auto output3 = input3.lnfa2nfa();

    input3.exportGraph("./examples/lnfa2nfa", "input");
    output3.exportGraph("./examples/lnfa2nfa", "output");

    // nfa -> dfa

    auto input4 = FiniteAutomata::create(
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
    auto output4 = input4.nfa2dfa();

    input4.exportGraph("./examples/nfa2dfa", "input");
    output4.exportGraph("./examples/nfa2dfa", "output");

    // dfa -> min dfa

    auto input5 = FiniteAutomata::create(
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
    auto output5 = input5.dfa2minDfa();

    input5.exportGraph("./examples/dfa2minDfa", "input");
    output5.exportGraph("./examples/dfa2minDfa", "output");

    // dfa -> re

    auto input6 = FiniteAutomata::create(
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
    auto output6 = input6.lnfa2re();

    input6.exportGraph("./examples/dfa2re", "input");
    output6.exportExpression("./examples/dfa2re", "output");

    // dfa -> complement

    auto input7 = FiniteAutomata::create(
        { "A", "B", "C", "D" },
        "A",
        { "A", "C" },
        {
            Edge("A", "B", 'a'),
            Edge("B", "C", 'b'),
            Edge("C", "B", 'a'),
            Edge("C", "D", 'b'),
            Edge("D", "A", 'b'),
        }
    );
    auto output7 = input7.dfa2complement();

    input7.exportGraph("./examples/dfa2complement", "input");
    output7.exportGraph("./examples/dfa2complement", "output");

    // matches

    // f(x) = x congruent n mod m
    std::unordered_set<int> n = { 1, 5 };
    int m = 6;
    
    std::unordered_set<std::string> input8_states;
    std::string input8_startState = "0";
    std::unordered_set<std::string> input8_acceptingStates;
    std::unordered_set<Edge> input8_edges;

    for (int i = 0;i<m;i++) {
        auto iStr = std::to_string(i);

        input8_states.insert(iStr);

        if (n.contains(i)) input8_acceptingStates.insert(iStr);
        
        input8_edges.insert(Edge(iStr, std::to_string((2 * i) % m), '0'));
        input8_edges.insert(Edge(iStr, std::to_string((2 * i + 1) % m), '1'));
    }

    auto input8 = FiniteAutomata::create(input8_states, input8_startState, input8_acceptingStates, input8_edges);

    input8.exportGraph("./examples/matches", "input");
    
    return 0;
};