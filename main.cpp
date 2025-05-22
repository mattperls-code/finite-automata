#include <iostream>

#include "./finite_automata.hpp"

void testRegularExpressions()
{
    RegularExpression re = RegularExpression::fromExpressionString("(cbd (ab + b*(ca)))*abc + ab(c  + d*)*");

    std::cout << "regular expression" << std::endl;
    std::cout << re.toString() << std::endl << std::endl;
};

void testFiniteAutomataKleeneCycle()
{
    FiniteAutomata lnfa = FiniteAutomata::create(
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
            Edge("5", "4", 'b'),
        }
    );

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
};

int main()
{
    testRegularExpressions();
    // testFiniteAutomataKleeneCycle();
    
    return 0;
};