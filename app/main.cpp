#include <iostream>

#include "../src/finite_automata.hpp"

void test(std::string testExpressionStr)
{
    RegularExpression inputRe = RegularExpression::fromExpressionString(testExpressionStr);

    std::cout << "input re" << std::endl;
    std::cout << inputRe.toString() << std::endl << std::endl;

    FiniteAutomata lnfa = FiniteAutomata::re2lnfa(inputRe);

    std::cout << "lnfa" << std::endl;
    std::cout << lnfa.toString() << std::endl << std::endl;

    FiniteAutomata nfa = lnfa.lnfa2nfa();

    std::cout << "nfa" << std::endl;
    std::cout << nfa.toString() << std::endl << std::endl;

    FiniteAutomata dfa = nfa.nfa2dfa();

    std::cout << "dfa" << std::endl;
    std::cout << dfa.toString() << std::endl << std::endl;

    FiniteAutomata minDfa = dfa.dfa2minDfa();

    std::cout << "minDfa" << std::endl;
    std::cout << minDfa.toString() << std::endl << std::endl;

    FiniteAutomata compressedMinDfa = minDfa.compressNames();

    std::cout << "compressed minDfa" << std::endl;
    std::cout << compressedMinDfa.toString() << std::endl << std::endl;

    RegularExpression outputRe = compressedMinDfa.lnfa2re();

    std::cout << "output re" << std::endl;
    std::cout << outputRe.toString() << std::endl << std::endl;

    FiniteAutomata recycledMinDfa = FiniteAutomata::re2lnfa(outputRe).lnfa2nfa().nfa2dfa().dfa2minDfa().compressNames();

    std::cout << "recycled minDfa" << std::endl;
    std::cout << recycledMinDfa.toString() << std::endl << std::endl;

    std::cout << "dfa iso minDfa: " << FiniteAutomata::isIsomorphism(dfa, minDfa) << std::endl;
    std::cout << "minDfa iso recycled minDfa: " << FiniteAutomata::isIsomorphism(minDfa, recycledMinDfa) << std::endl;

    std::cout << "lnfa lang eq minDfa: " << FiniteAutomata::isLanguageEquivalence(lnfa, minDfa) << std::endl;
    std::cout << "lnfa lang eq recycled minDfa: " << FiniteAutomata::isLanguageEquivalence(lnfa, recycledMinDfa) << std::endl;

    recycledMinDfa.exportGraph("./results", "img1");
};

int main()
{
    test("((b + a)( a + b ))*(ab + bc)* + (λ + e(e + g)) + e*g");
    
    return 0;
};