#include <catch2/catch_all.hpp>
#include <bitset>

#include "../src/finite_automata.hpp"

TEST_CASE("CONSTRUCTIONS") {
    // str -> re

    auto input1 = "a (b (b* + a + λ) + λ(a + (ab + b + λ)* bb)) b(ab)*";
    auto expectedOutput1 = "a(b(b*+a+λ)+a+(ab+b+λ)*bb)b(ab)*";
    auto observedOutput1 = RegularExpression::fromExpressionString(input1);

    REQUIRE(expectedOutput1 == observedOutput1.toString());

    // re -> lnfa

    auto input2 = RegularExpression::fromExpressionString("ab*(a+b(a+λ)) + (a + λ)");
    auto expectedOutput2 = FiniteAutomata::create(
        { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21" },
        "1",
        { "21" },
        {
            Edge("1", "2", {}),
            Edge("2", "3", 'a'),
            Edge("3", "4", 'b'),
            Edge("3", "4", {}),
            Edge("4", "3", {}),
            Edge("4", "5", {}),
            Edge("5", "6", 'a'),
            Edge("4", "7", {}),
            Edge("7", "8", 'b'),
            Edge("8", "9", {}),
            Edge("9", "10", 'a'),
            Edge("8", "11", {}),
            Edge("11", "12", {}),
            Edge("10", "13", {}),
            Edge("12", "13", {}),
            Edge("6", "14", {}),
            Edge("13", "14", {}),
            Edge("1", "15", {}),
            Edge("15", "16", {}),
            Edge("16", "17", 'a'),
            Edge("15", "18", {}),
            Edge("18", "19", {}),
            Edge("17", "20", {}),
            Edge("19", "20", {}),
            Edge("14", "21", {}),
            Edge("20", "21", {}),
        }
    );
    auto observedOutput2 = FiniteAutomata::re2lnfa(input2);

    REQUIRE(FiniteAutomata::isLanguageEquivalence(expectedOutput2, observedOutput2));

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
    auto expectedOutput3 = FiniteAutomata::create(
        { "A", "B", "C", "D", "E", "F" },
        "A",
        { "A", "E", "F" },
        {
            Edge("A", "A", 'b'),
            Edge("A", "C", 'a'),
            Edge("A", "C", 'b'),
            Edge("A", "D", 'a'),
            Edge("B", "D", 'b'),
            Edge("A", "E", 'b'),
            Edge("B", "F", 'b'),
            Edge("C", "A", 'b'),
            Edge("C", "C", 'b'),
            Edge("C", "E", 'b'),
            Edge("D", "F", 'a'),
            Edge("E", "A", 'b'),
            Edge("E", "B", 'a'),
            Edge("E", "C", 'a'),
            Edge("E", "C", 'b'),
            Edge("E", "D", 'a'),
            Edge("E", "E", 'b'),
            Edge("F", "F", 'a'),
        }
    );
    auto observedOutput3 = input3.lnfa2nfa();

    REQUIRE(FiniteAutomata::isLanguageEquivalence(expectedOutput3, observedOutput3));

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
    auto expectedOutput4 = FiniteAutomata::create(
        { "A", "B", "C", "D", "E", "BE", "CE", "BD", "CDE" },
        "A",
        { "B", "D", "BE", "BD", "CDE" },
        {
            Edge("A", "BE", 'a'),
            Edge("A", "E", 'b'),
            Edge("B", "C", 'a'),
            Edge("B", "CE", 'b'),
            Edge("C", "BD", 'b'),
            Edge("E", "C", 'a'),
            Edge("E", "D", 'b'),
            Edge("BE", "C", 'a'),
            Edge("BE", "CDE", 'b'),
            Edge("CE", "C", 'a'),
            Edge("CE", "BD", 'b'),
            Edge("BD", "C", 'a'),
            Edge("BD", "CE", 'b'),
            Edge("CDE", "C", 'a'),
            Edge("CDE", "BD", 'b')
        }
    );
    auto observedOutput4 = input4.nfa2dfa();

    REQUIRE(FiniteAutomata::isIsomorphism(expectedOutput4, observedOutput4));

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
    auto expectedOutput5 = FiniteAutomata::create(
        { "A", "B", "C", "D", "E" },
        "A",
        { "A" },
        {
            Edge("A", "A", '0'),
            Edge("A", "E", '1'),
            Edge("B", "E", '0'),
            Edge("B", "B", '1'),
            Edge("C", "A", '0'),
            Edge("C", "E", '1'),
            Edge("D", "C", '0'),
            Edge("D", "E", '1'),
            Edge("E", "B", '0'),
            Edge("E", "D", '1'),
        }
    );
    auto observedOutput5 = input5.dfa2minDfa();

    REQUIRE(FiniteAutomata::isIsomorphism(expectedOutput5, observedOutput5));

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
    auto expectedOutput6 = RegularExpression::fromExpressionString("0(1 + 20)*0");
    auto observedOutput6 = input6.lnfa2re();
    
    REQUIRE(FiniteAutomata::isLanguageEquivalence(
        FiniteAutomata::re2lnfa(expectedOutput6),
        FiniteAutomata::re2lnfa(observedOutput6)
    ));

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
            Edge("D", "A", 'b')
        }
    );
    auto expectedOutput7 = FiniteAutomata::create(
        { "A", "B", "C", "D", "EMPTY" },
        "A",
        { "B", "D", "EMPTY" },
        {
            Edge("A", "B", 'a'),
            Edge("A", "EMPTY", 'b'),
            Edge("B", "EMPTY", 'a'),
            Edge("B", "C", 'b'),
            Edge("C", "B", 'a'),
            Edge("C", "D", 'b'),
            Edge("D", "EMPTY", 'a'),
            Edge("D", "A", 'b'),
            Edge("EMPTY", "EMPTY", 'a'),
            Edge("EMPTY", "EMPTY", 'b'),
        }
    );
    auto observedOutput7 = input7.dfa2complement();

    REQUIRE(FiniteAutomata::isIsomorphism(expectedOutput7, observedOutput7));

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

    for (int i = 0;i<10*m;i++) {
        auto expectedOutput8 = n.contains(i % m);
        auto observedOutput8 = input8.matches(std::bitset<8>(i).to_string());
        
        REQUIRE(expectedOutput8 == observedOutput8);
    }
}

// https://people.cs.umass.edu/~barring/cs250f24/exams/finsol.pdf

TEST_CASE("F24") {
    // 4.a)

    auto input4a = FiniteAutomata::create(
        { "i", "p", "q", "f" },
        "i",
        { "f" },
        {
            Edge("i", "i", 'a'),
            Edge("i", "i", 'b'),
            Edge("i", "p", 'a'),
            Edge("i", "q", 'b'),
            Edge("p", "f", 'a'),
            Edge("q", "f", 'b'),
            Edge("f", "f", 'a'),
            Edge("f", "f", 'b'),
        }
    );
    auto expectedOutput4a = FiniteAutomata::create(
        { "i", "ip", "iq", "ipf", "iqf" },
        "i",
        { "ipf", "iqf" },
        {
            Edge("i", "ip", 'a'),
            Edge("i", "iq", 'b'),
            Edge("ip", "iq", 'b'),
            Edge("iq", "ip", 'a'),
            Edge("ip", "ipf", 'a'),
            Edge("iq", "iqf", 'b'),
            Edge("ipf", "ipf", 'a'),
            Edge("iqf", "iqf", 'b'),
            Edge("ipf", "iqf", 'b'),
            Edge("iqf", "ipf", 'a')
        }
    );
    auto observedOutput4a = input4a.nfa2dfa();

    REQUIRE(FiniteAutomata::isIsomorphism(expectedOutput4a, observedOutput4a));

    // 4.b)

    auto input4b = observedOutput4a;
    auto expectedOutput4b = FiniteAutomata::create(
        { "i", "ip", "iq", "f" },
        "i",
        { "f" },
        {
            Edge("i", "ip", 'a'),
            Edge("i", "iq", 'b'),
            Edge("ip", "iq", 'b'),
            Edge("iq", "ip", 'a'),
            Edge("ip", "f", 'a'),
            Edge("iq", "f", 'b'),
            Edge("f", "f", 'a'),
            Edge("f", "f", 'b'),
        }
    );
    auto observedOutput4b = input4b.dfa2minDfa();

    REQUIRE(FiniteAutomata::isIsomorphism(expectedOutput4b, observedOutput4b));

    // 4.c)

    auto input4c = observedOutput4b;
    auto expectedOutput4c = RegularExpression::fromExpressionString("aa(a + b)* + (b + ab)(ab)*(b(a + b)* + aa(a + b)*)");
    auto observedOutput4c = input4c.lnfa2re();

    REQUIRE(FiniteAutomata::isLanguageEquivalence(
        FiniteAutomata::re2lnfa(expectedOutput4c),
        FiniteAutomata::re2lnfa(observedOutput4c)
    ));

    // 4.d)

    auto input4d = FiniteAutomata::create(
        { "1", "2", "3", "4", "5" },
        "1",
        { "5" },
        {
            Edge("1", "2", {}),
            Edge("1", "4", 'b'),
            Edge("2", "1", 'a'),
            Edge("2", "3", 'b'),
            Edge("2", "5", {}),
            Edge("3", "5", {}),
            Edge("4", "2", 'a'),
            Edge("5", "4", 'b')
        }
    );
    auto expectedOutput4d = FiniteAutomata::create(
        { "1", "2", "3", "4", "5" },
        "1",
        { "1", "2", "3", "5" },
        {
            Edge("1", "1", 'a'),
            Edge("1", "2", 'a'),
            Edge("1", "5", 'a'),
            Edge("1", "3", 'b'),
            Edge("1", "4", 'b'),
            Edge("1", "5", 'b'),
            Edge("2", "1", 'a'),
            Edge("2", "2", 'a'),
            Edge("2", "5", 'a'),
            Edge("2", "3", 'b'),
            Edge("2", "4", 'b'),
            Edge("2", "5", 'b'),
            Edge("3", "4", 'b'),
            Edge("4", "2", 'a'),
            Edge("4", "5", 'a'),
            Edge("5", "4", 'b'),
        }
    );
    auto observedOutput4d = input4d.lnfa2nfa();

    REQUIRE(FiniteAutomata::isIsomorphism(
        observedOutput4d.nfa2dfa(),
        expectedOutput4d.nfa2dfa()
    ));
}

// https://people.cs.umass.edu/~barring/cs250s24/exams/finsol.pdf

TEST_CASE("S24") {
    // 6.a)

    auto input6a = FiniteAutomata::create(
        { "1", "2", "3" },
        "1",
        { "2" },
        {
            Edge("1", "2", 'a'),
            Edge("1", "3", 'a'),
            Edge("2", "1", 'b'),
            Edge("2", "3", 'b')
        }
    );
    auto expectedOutput6a = FiniteAutomata::create(
        { "1", "13", "23" },
        "1",
        { "23" },
        {
            Edge("1", "23", 'a'),
            Edge("13", "23", 'a'),
            Edge("23", "13", 'b'),
        }
    );
    auto observedOutput6a = input6a.nfa2dfa();

    REQUIRE(FiniteAutomata::isIsomorphism(expectedOutput6a, observedOutput6a));

    // 6.b)

    auto input6b = observedOutput6a;
    auto expectedOutput6b = FiniteAutomata::create(
        { "X", "23" },
        "X",
        { "23" },
        {
            Edge("X", "23", 'a'),
            Edge("23", "X", 'b')
        }
    );
    auto observedOutput6b = input6b.dfa2minDfa();

    REQUIRE(FiniteAutomata::isIsomorphism(expectedOutput6b, observedOutput6b));

    // 6.c)

    auto input6c = observedOutput6b;
    auto expectedOutput6c = RegularExpression::fromExpressionString("a(ba)*");
    auto observedOutput6c = input6c.lnfa2re();

    REQUIRE(FiniteAutomata::isLanguageEquivalence(
        FiniteAutomata::re2lnfa(expectedOutput6c),
        FiniteAutomata::re2lnfa(observedOutput6c)
    ));

    // 6.d)

    auto input6d = RegularExpression::fromExpressionString("a(ba)*");
    auto expectedOutput6d = FiniteAutomata::create(
        { "1", "2", "3", "4", "5", "6" },
        "1",
        { "6" },
        {
            Edge("1", "2", 'a'),
            Edge("2", "3", {}),
            Edge("3", "4", 'b'),
            Edge("3", "5", {}),
            Edge("4", "5", 'a'),
            Edge("5", "3", {}),
            Edge("5", "6", {}),
        }
    );
    auto observedOutput6d = FiniteAutomata::re2lnfa(input6d);

    REQUIRE(FiniteAutomata::isLanguageEquivalence(
        expectedOutput6d,
        observedOutput6d
    ));

    // 6.e)

    auto input6e = FiniteAutomata::create(
        { "p", "q", "r", "s" },
        "p",
        { "q" },
        {
            Edge("p", "q", 'a'),
            Edge("p", "r", {}),
            Edge("q", "s", 'a'),
            Edge("r", "q", {}),
            Edge("s", "q", {}),
            Edge("s", "r", 'b'),
        }
    );
    auto expectedOutput6e = FiniteAutomata::create(
        { "p", "q", "r", "s" },
        "p",
        { "p", "q" },
        {
            Edge("p", "q", 'a'),
            Edge("p", "s", 'a'),
            Edge("q", "q", 'a'),
            Edge("q", "s", 'a'),
            Edge("r", "q", 'a'),
            Edge("r", "s", 'a'),
            Edge("s", "q", 'a'),
            Edge("s", "q", 'b'),
            Edge("s", "r", 'b'),
            Edge("s", "s", 'a'),
        }
    );
    auto observedOutput6e = input6e.lnfa2nfa();

    REQUIRE(FiniteAutomata::isLanguageEquivalence(expectedOutput6e, observedOutput6e));
}

int main() {
    return Catch::Session().run();
}