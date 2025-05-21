#ifndef FINITE_AUTOMATA_HPP
#define FINITE_AUTOMATA_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <variant>
#include <memory>

typedef std::optional<char> Letter; // nullopt for lambda

template <>
struct std::hash<std::unordered_map<Letter, int>> {
    size_t operator()(const std::unordered_map<Letter, int>& transitionClass) const;
};

class Edge
{
    public: 
        std::string start;
        std::string end;
        Letter letter;

        Edge(std::string start, std::string end, Letter letter): start(start), end(end), letter(letter) {};

        std::string toString();

        bool operator==(const Edge&) const = default;
};

template <>
struct std::hash<Edge> {
    size_t operator()(const Edge& edge) const;
};

class RegularExpression;

class FiniteAutomata
{
    private:
        std::unordered_set<std::string> states;
        std::string startState;
        std::unordered_set<std::string> acceptingStates;
        std::unordered_set<Edge> edges;

        FiniteAutomata(std::unordered_set<std::string> states, std::string startState, std::unordered_set<std::string> acceptingStates, std::unordered_set<Edge> edges);

        // [startState][letter] = set<endState>
        std::unordered_map<std::string, std::unordered_map<Letter, std::unordered_set<std::string>>> transitionTable;

        // [endState][letter] = set<startState>
        std::unordered_map<std::string, std::unordered_map<Letter, std::unordered_set<std::string>>> invertedTransitionTable;

        // states with an edge starting from the given state
        std::unordered_set<std::string> getStatesDirectlyStartingAt(std::string state);
        std::unordered_set<std::string> getStatesDirectlyStartingAt(std::string state, Letter letter);

        // states reachable starting from the given state
        std::unordered_set<std::string> getStatesTransitivelyStartingAt(std::string state);
        std::unordered_set<std::string> getStatesTransitivelyStartingAt(std::string state, Letter letter);

        // states with an edge ending at the given state
        std::unordered_set<std::string> getStatesDirectlyEndingAt(std::string state);
        std::unordered_set<std::string> getStatesDirectlyEndingAt(std::string state, Letter letter);

        // states reachable ending at the given state
        std::unordered_set<std::string> getStatesTransitivelyEndingAt(std::string state);
        std::unordered_set<std::string> getStatesTransitivelyEndingAt(std::string state, Letter letter);

        std::unordered_map<std::string, int> getMinDfaEquivalenceClassIndexes();

    public:
        static FiniteAutomata create(std::unordered_set<std::string> states, std::string startState, std::unordered_set<std::string> acceptingStates, std::unordered_set<Edge> edges);

        bool hasLambdaMoves();
        bool isDeterministic();

        bool matches(std::string str);

        FiniteAutomata lnfa2nfa();

        FiniteAutomata nfa2dfa();

        FiniteAutomata dfa2minDfa();

        RegularExpression dfa2re();

        std::string toString();
};

class RegularExpression
{
    private:
        enum Operation { NONE, CONCAT, PLUS, STAR };

        Operation operation;

        std::variant<std::monostate, char, std::unique_ptr<RegularExpression>> leftOperand;
        std::variant<std::monostate, char, std::unique_ptr<RegularExpression>> rightOperand;

    public:
        RegularExpression(): operation(NONE), leftOperand(), rightOperand() {};
        RegularExpression(char letter);
        
        static RegularExpression concat(RegularExpression re1, RegularExpression re2);
        static RegularExpression plus(RegularExpression re1, RegularExpression re2);
        static RegularExpression star(RegularExpression re);

        std::string toString();

        static RegularExpression fromString(std::string str);
};

#endif