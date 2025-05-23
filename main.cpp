#include <iostream>

#include "./finite_automata.hpp"

void test(std::string testExpressionStr)
{
    RegularExpression re = RegularExpression::fromExpressionString(testExpressionStr);

    std::cout << "re" << std::endl;
    std::cout << re.toString() << std::endl << std::endl;

    FiniteAutomata lnfa = FiniteAutomata::re2lnfa(re);

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
};

int main()
{
    test("λ + a(a + b)*b");
    
    return 0;
};