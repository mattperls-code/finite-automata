#ifndef FINITE_AUTOMATA_HPP
#define FINITE_AUTOMATA_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

class TransitionToken
{
    private:
        bool isChar;
        char c;
        
        TransitionToken(): isChar(false), c() {};
        TransitionToken(char c): isChar(true), c(c) {};
    
    public:
        bool isLambda();
        char getChar();

        static TransitionToken lambda();
        static TransitionToken character(char c);
};

class NamedEdge
{
    public:
        std::string startState;
        std::string endState;
        TransitionToken transitionToken;

        NamedEdge(std::string startState, std::string endState, TransitionToken transitionToken): startState(startState), endState(endState), transitionToken(transitionToken) {};
};

class IndexedEdge
{
    public:
        int startState;
        int endState;
        TransitionToken transitionToken;

        IndexedEdge(int startState, int endState, TransitionToken transitionToken): startState(startState), endState(endState), transitionToken(transitionToken) {};
};

class FiniteAutomata
{
    private:
        std::unordered_set<char> alphabet;
        
        std::vector<std::string> stateIndexToNameMap;
        std::unordered_map<std::string, int> stateNameToIndexMap;

        std::string startStateName;
        int startStateIndex;

        std::unordered_set<std::string> finalStateNames;
        std::unordered_set<int> finalStateIndices;

        std::vector<NamedEdge> namedEdges;
        std::vector<IndexedEdge> indexedEdges;
        
        // [startStateIndex][edgeIndex] = { startState, endState, transitionToken }
        std::vector<std::vector<IndexedEdge>> edgeTable;

        bool hasState(std::string stateName);

        int nextState(int currentStateIndex, char c);
    
    public:
        FiniteAutomata(std::unordered_set<char> alphabet, std::unordered_set<std::string> stateNames, std::string startStateName, std::unordered_set<std::string> finalStateNames, std::vector<NamedEdge> namedEdges);

        bool isValidDFA();

        bool matches(std::string str);

        FiniteAutomata nfa2dfa();

        FiniteAutomata minimizeDFA();

        std::string toString();
};

#endif