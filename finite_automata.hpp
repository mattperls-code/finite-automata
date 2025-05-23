#ifndef FINITE_AUTOMATA_HPP
#define FINITE_AUTOMATA_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <variant>
#include <memory>

#include "lib/parser.hpp"

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

enum RegularExpressionType
{
    EMPTY,
    CHARACTER,
    CONCAT,
    PLUS,
    STAR
};

class RegularExpression
{
    private:
        RegularExpressionType type;

        typedef std::variant<
            std::monostate,
            char,
            std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>>,
            std::shared_ptr<RegularExpression>
        > Value;
        Value value;

        RegularExpression(RegularExpressionType type, Value value): type(type), value(value) {};

        static RegularExpression empty();
        static RegularExpression character(char c);
        static RegularExpression concat(RegularExpression re1, RegularExpression re2);
        static RegularExpression plus(RegularExpression re1, RegularExpression re2);
        static RegularExpression star(RegularExpression re);

        static RegularExpression fromToken(Token token);

    public:
        static RegularExpression fromExpressionString(std::string expressionStr);

        RegularExpressionType getType();

        char getCharacterExpression();
        std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> getConcatExpression();
        std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> getPlusExpression();
        std::shared_ptr<RegularExpression> getStarExpression();

        std::string toString();
};

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

        // these mutate the caller and return the end state for the branch added
        std::string addRe(std::string rootState, RegularExpression re);
        std::string addEmptyRe(std::string rootState);
        std::string addCharacterRe(std::string rootState, char characterExpression);
        std::string addConcatRe(std::string rootState, RegularExpression re1, RegularExpression re2);
        std::string addPlusRe(std::string rootState, RegularExpression re1, RegularExpression re2);
        std::string addStarRe(std::string rootState, RegularExpression re);

        std::unordered_set<std::string> getStatesDirectlyStartingAt(std::string state);
        std::unordered_set<std::string> getStatesDirectlyStartingAt(std::string state, Letter letter);
        std::unordered_set<std::string> getStatesTransitivelyStartingAt(std::string state);
        std::unordered_set<std::string> getStatesTransitivelyStartingAt(std::string state, Letter letter);

        std::unordered_set<std::string> getStatesDirectlyEndingAt(std::string state);
        std::unordered_set<std::string> getStatesDirectlyEndingAt(std::string state, Letter letter);
        std::unordered_set<std::string> getStatesTransitivelyEndingAt(std::string state);
        std::unordered_set<std::string> getStatesTransitivelyEndingAt(std::string state, Letter letter);

        std::unordered_map<std::string, int> getMinDfaEquivalenceClassIndexes();

    public:
        static FiniteAutomata create(std::unordered_set<std::string> states, std::string startState, std::unordered_set<std::string> acceptingStates, std::unordered_set<Edge> edges);

        FiniteAutomata compressNames();

        bool hasLambdaMoves();
        bool isDeterministic();

        static FiniteAutomata re2lnfa(RegularExpression re);

        FiniteAutomata lnfa2nfa();

        FiniteAutomata nfa2dfa();

        FiniteAutomata dfa2minDfa();

        RegularExpression dfa2re();

        bool matches(std::string str);

        std::string toString();
};

#endif