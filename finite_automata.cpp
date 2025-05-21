#include <stdexcept>
#include <vector>
#include <queue>
#include <algorithm>

#include "finite_automata.hpp"

std::string concatStrSet(std::unordered_set<std::string> strSet, std::string delimiter)
{
    std::string concat;

    std::vector<std::string> strVec(strSet.begin(), strSet.end());
    std::sort(strVec.begin(), strVec.end());

    for (auto str : strVec) concat += str + delimiter;
    
    if (!strVec.empty()) concat = concat.substr(0, concat.size() - delimiter.size());

    return concat;
};

size_t std::hash<std::unordered_map<Letter, int>>::operator()(const std::unordered_map<Letter, int>& transitionClass) const
{
    size_t hash = 0;

    for (auto [letter, endState] : transitionClass) hash ^= letter.value_or(256) * (endState + 1);

    return hash;
};

std::string Edge::toString()
{
    return "From " + this->start + " via " + (this->letter.has_value() ? std::string(1, this->letter.value()) : "λ") + " to " + this->end;
};

size_t std::hash<Edge>::operator()(const Edge& edge) const
{
    size_t startHash = std::hash<std::string>()(edge.start);
    size_t endHash = std::hash<std::string>()(edge.end);

    return startHash ^ endHash ^ edge.letter.value_or(256);
};

FiniteAutomata::FiniteAutomata(std::unordered_set<std::string> states, std::string startState, std::unordered_set<std::string> acceptingStates, std::unordered_set<Edge> edges)
{
    this->states = states;

    this->startState = startState;
    if (!this->states.contains(this->startState)) throw std::runtime_error("FiniteAutomata constructor: start refers to unknown state");

    this->acceptingStates = acceptingStates;
    for (auto acceptingState : this->acceptingStates) {
        if (!this->states.contains(acceptingState)) throw std::runtime_error("FiniteAutomata constuctor: accepting state refers to unknown state");
    }

    this->edges = edges;

    for (auto edge : this->edges) {
        if (!this->states.contains(edge.start) || !this->states.contains(edge.end)) throw std::runtime_error("FiniteAutomata constructor: edge refers to unknown state");

        this->transitionTable[edge.start][edge.letter].insert(edge.end);
        this->invertedTransitionTable[edge.end][edge.letter].insert(edge.start);
    }
};

FiniteAutomata FiniteAutomata::create(std::unordered_set<std::string> states, std::string startState, std::unordered_set<std::string> acceptingStates, std::unordered_set<Edge> edges)
{
    for (auto state : states) {
        for (auto c : state) if (!isalnum(c) && c != '_') throw std::runtime_error("FiniteAutomata create: state names must be alphanumeric or underscored");
    }

    return FiniteAutomata(states, startState, acceptingStates, edges);
};

std::unordered_set<std::string> FiniteAutomata::getStatesDirectlyStartingAt(std::string state)
{
    std::unordered_set<std::string> allEndStates;

    for (auto [_, endStates] : this->transitionTable[state]) allEndStates.merge(endStates);

    return allEndStates;
};

std::unordered_set<std::string> FiniteAutomata::getStatesDirectlyStartingAt(std::string state, Letter letter)
{
    std::unordered_set<std::string> allEndStates;

    if (this->transitionTable[state].contains(letter)) allEndStates = this->transitionTable[state][letter];

    return allEndStates;
};

std::unordered_set<std::string> FiniteAutomata::getStatesTransitivelyStartingAt(std::string state)
{
    std::unordered_set<std::string> allEndStates;

    std::queue<std::string> queue;
    queue.push(state);

    while (!queue.empty()) {
        auto currentState = queue.front();

        queue.pop();

        if (allEndStates.contains(currentState)) continue;

        allEndStates.insert(currentState);

        for (auto neighbor : this->getStatesDirectlyStartingAt(currentState)) queue.push(neighbor);
    }

    return allEndStates;
};

std::unordered_set<std::string> FiniteAutomata::getStatesTransitivelyStartingAt(std::string state, Letter letter)
{
    std::unordered_set<std::string> allEndStates;

    std::queue<std::string> queue;
    queue.push(state);

    while (!queue.empty()) {
        auto currentState = queue.front();

        queue.pop();

        if (allEndStates.contains(currentState)) continue;

        allEndStates.insert(currentState);

        for (auto neighbor : this->getStatesDirectlyStartingAt(currentState, letter)) queue.push(neighbor);
    }

    return allEndStates;
};

std::unordered_set<std::string> FiniteAutomata::getStatesDirectlyEndingAt(std::string state)
{
    std::unordered_set<std::string> allStartStates;

    for (auto [_, startStates] : this->invertedTransitionTable[state]) allStartStates.merge(startStates);

    return allStartStates;
};

std::unordered_set<std::string> FiniteAutomata::getStatesDirectlyEndingAt(std::string state, Letter letter)
{
    std::unordered_set<std::string> allStartStates;

    if (this->invertedTransitionTable[state].contains(letter)) allStartStates = this->invertedTransitionTable[state][letter];

    return allStartStates;
};

std::unordered_set<std::string> FiniteAutomata::getStatesTransitivelyEndingAt(std::string state)
{
    std::unordered_set<std::string> allStartStates;

    std::queue<std::string> queue;
    queue.push(state);

    while (!queue.empty()) {
        auto currentState = queue.front();

        queue.pop();

        if (allStartStates.contains(currentState)) continue;

        allStartStates.insert(currentState);

        for (auto neighbor : this->getStatesDirectlyEndingAt(currentState)) queue.push(neighbor);
    }

    return allStartStates;
};

std::unordered_set<std::string> FiniteAutomata::getStatesTransitivelyEndingAt(std::string state, Letter letter)
{
    std::unordered_set<std::string> allStartStates;

    std::queue<std::string> queue;
    queue.push(state);

    while (!queue.empty()) {
        auto currentState = queue.front();

        queue.pop();

        if (allStartStates.contains(currentState)) continue;

        allStartStates.insert(currentState);

        for (auto neighbor : this->getStatesDirectlyEndingAt(currentState, letter)) queue.push(neighbor);
    }

    return allStartStates;
};

bool FiniteAutomata::hasLambdaMoves()
{
    for (auto edge : this->edges) if (!edge.letter.has_value()) return true;

    return false;
};

bool FiniteAutomata::isDeterministic()
{
    for (auto [_, transitions] : this->transitionTable) {
        if (transitions.contains({})) return false; // lambda edge
        
        for (auto [_, endStates] : transitions) {
            if (endStates.size() > 1) return false; // multiple edges for same letter
        }
    }

    return true;
};

bool FiniteAutomata::matches(std::string str)
{
    if (!this->isDeterministic()) throw std::runtime_error("FiniteAutomata matches: only callable for DFA");

    std::string state = this->startState;

    for (auto letter : str) {
        auto transitionsAtState = this->transitionTable[state];

        if (!transitionsAtState.contains(letter)) return false;

        state = *transitionsAtState[letter].begin();
    }

    return this->acceptingStates.contains(state);
};

FiniteAutomata FiniteAutomata::lnfa2nfa()
{
    if (!this->hasLambdaMoves()) return *this;

    std::unordered_set<Edge> nfaEdges;
    
    // some caching for transitive closure can be done here

    for (auto edge : this->edges) {
        if (!edge.letter.has_value()) continue; // only construct non lambda edges
        
        for (auto startState : this->getStatesTransitivelyEndingAt(edge.start, {})) {
            for (auto endState : this->getStatesTransitivelyStartingAt(edge.end, {})) {
                nfaEdges.insert(Edge(startState, endState, edge.letter));
            }
        }
    }

    return FiniteAutomata(this->states, this->startState, this->acceptingStates, nfaEdges);
};

FiniteAutomata FiniteAutomata::nfa2dfa()
{
    if (this->hasLambdaMoves()) throw std::runtime_error("FiniteAutomata nfa2dfa: only callable for ordinary NFA");

    if (this->isDeterministic()) return *this;

    std::unordered_set<std::string> dfaStates;
    std::string dfaStartState = "{" + this->startState + "}";
    std::unordered_set<std::string> dfaAcceptingStates;
    std::unordered_set<Edge> dfaEdges;

    std::queue<std::unordered_set<std::string>> queue;
    queue.push({ this->startState });

    while (!queue.empty()) {
        auto currentStates = queue.front();

        auto dfaState = "{" + concatStrSet(currentStates, ",") + "}";

        queue.pop();

        if (dfaStates.contains(dfaState)) continue;

        dfaStates.insert(dfaState);
        for (auto state : currentStates) if (this->acceptingStates.contains(state)) dfaAcceptingStates.insert(dfaState);

        std::unordered_map<Letter, std::unordered_set<std::string>> currentStatesTransitions;

        for (auto state : currentStates) {
            for (auto [letter, endStates] : this->transitionTable[state]) currentStatesTransitions[letter].merge(endStates);
        }

        for (auto [letter, endStates] : currentStatesTransitions) {
            std::string dfaEndState = "{" + concatStrSet(endStates, ",") + "}";

            dfaEdges.insert(Edge(dfaState, dfaEndState, letter));

            queue.push(endStates);
        }
    }

    return FiniteAutomata(dfaStates, dfaStartState, dfaAcceptingStates, dfaEdges);
};

std::unordered_map<std::string, int> FiniteAutomata::getMinDfaEquivalenceClassIndexes()
{
    auto reachableStates = this->getStatesTransitivelyStartingAt(this->startState);

    std::unordered_map<std::string, int> equivalenceClassIndexes;

    // initial partition
    int numEquivalenceClasses = 2;
    for (auto state : reachableStates) equivalenceClassIndexes[state] = this->acceptingStates.contains(state);

    while (true) {
        std::unordered_map<std::unordered_map<Letter, int>, std::unordered_set<std::string>> acceptingEquivalenceClasses;
        std::unordered_map<std::unordered_map<Letter, int>, std::unordered_set<std::string>> nonAcceptingEquivalenceClasses;

        for (auto state : reachableStates) {
            std::unordered_map<Letter, int> transitionClass;

            for (auto [letter, endStates] : this->transitionTable[state]) transitionClass[letter] = equivalenceClassIndexes[*endStates.begin()];

            auto& equivalenceClassesFamily = this->acceptingStates.contains(state) ? acceptingEquivalenceClasses : nonAcceptingEquivalenceClasses;

            equivalenceClassesFamily[transitionClass].insert(state);
        }

        int newNumEquivalenceClasses = acceptingEquivalenceClasses.size() + nonAcceptingEquivalenceClasses.size();

        if (newNumEquivalenceClasses == numEquivalenceClasses) return equivalenceClassIndexes; // minimal partition found

        numEquivalenceClasses = newNumEquivalenceClasses;

        int equivalenceClassIndex = 0;

        for (auto [_, equivalentStates] : acceptingEquivalenceClasses) {
            for (auto state : equivalentStates) equivalenceClassIndexes[state] = equivalenceClassIndex;

            equivalenceClassIndex++;
        }
        
        for (auto [_, equivalentStates] : nonAcceptingEquivalenceClasses) {
            for (auto state : equivalentStates) equivalenceClassIndexes[state] = equivalenceClassIndex;

            equivalenceClassIndex++;
        }
    }
};

FiniteAutomata FiniteAutomata::dfa2minDfa()
{
    if (!this->isDeterministic()) throw std::runtime_error("FiniteAutomata dfa2minDfa: only callable for DFA");

    auto minDfaEquivalenceClassIndexes = this->getMinDfaEquivalenceClassIndexes();
    
    std::unordered_map<int, std::unordered_set<std::string>> minDfaEquivalenceClasses;

    for (auto [state, equivalenceClassIndex] : minDfaEquivalenceClassIndexes) minDfaEquivalenceClasses[equivalenceClassIndex].insert(state);

    std::unordered_set<std::string> minDfaStates;
    std::string minDfaStartState;
    std::unordered_set<std::string> minDfaAcceptingStates;
    std::unordered_set<Edge> minDfaEdges;

    for (auto [_, memberStates] : minDfaEquivalenceClasses) {
        auto memberState = *memberStates.begin();

        auto minDfaState = "{" + concatStrSet(memberStates, ",") + "}";
        
        minDfaStates.insert(minDfaState);
        if (memberStates.contains(this->startState)) minDfaStartState = minDfaState;
        if (this->acceptingStates.contains(memberState)) minDfaAcceptingStates.insert(minDfaState);

        for (auto [letter, endStates] : this->transitionTable[memberState]) {
            auto endState = *endStates.begin();
            
            auto endStateEquivalenceClassIndex = minDfaEquivalenceClassIndexes[endState];

            auto minDfaEndState = "{" + concatStrSet(minDfaEquivalenceClasses[endStateEquivalenceClassIndex], ",") + "}";

            minDfaEdges.insert(Edge(minDfaState, minDfaEndState, letter));
        }
    }

    return FiniteAutomata(minDfaStates, minDfaStartState, minDfaAcceptingStates, minDfaEdges);
};

std::string FiniteAutomata::toString()
{
    std::string output;

    output += "States: " + concatStrSet(this->states, ", ");

    output += "\n";

    output += "Start State: " + this->startState + "";

    output += "\n";

    output += "Accepting States: " + concatStrSet(this->acceptingStates, ", ");

    output += "\n";

    std::unordered_set<std::string> edgeStrSet;
    for (auto edge : this->edges) edgeStrSet.insert(edge.toString());

    output += "Edges: \n\t" + concatStrSet(edgeStrSet, "\n\t");

    if (this->edges.empty()) output += "NONE";

    return output;
};

// RegularExpression::RegularExpression(char letter)
// {
//     if (!isalnum(letter)) throw std::runtime_error("RegularExpression constructor: letter must be alphanumeric");

//     this->operation = NONE;

//     this->leftOperand = letter;
//     this->rightOperand = {};
// };

// RegularExpression RegularExpression::concat(RegularExpression re1, RegularExpression re2)
// {
//     RegularExpression concatExpression;

//     concatExpression.operation = CONCAT;

//     concatExpression.leftOperand = std::make_unique<RegularExpression>(re1);
//     concatExpression.rightOperand = std::make_unique<RegularExpression>(re2);

//     return concatExpression;
// };

// RegularExpression RegularExpression::plus(RegularExpression re1, RegularExpression re2)
// {
//     RegularExpression concatExpression;

//     concatExpression.operation = PLUS;

//     concatExpression.leftOperand = std::make_unique<RegularExpression>(re1);
//     concatExpression.rightOperand = std::make_unique<RegularExpression>(re2);

//     return concatExpression;
// };

// RegularExpression RegularExpression::star(RegularExpression re)
// {
//     RegularExpression concatExpression;

//     concatExpression.operation = STAR;

//     concatExpression.leftOperand = std::make_unique<RegularExpression>(re);

//     return concatExpression;
// };

// std::string RegularExpression::toString()
// {
//     switch (this->operation) {
//         case NONE:
//             return this->leftOperand.index() == 0 ? "λ" : std::string(1, std::get<char>(this->leftOperand));
//         case CONCAT:
//             return std::get<std::unique_ptr<RegularExpression>>(this->leftOperand)->toString() + std::get<std::unique_ptr<RegularExpression>>(this->rightOperand)->toString();
//         case PLUS:
//             return std::get<std::unique_ptr<RegularExpression>>(this->leftOperand)->toString() + " + " + std::get<std::unique_ptr<RegularExpression>>(this->rightOperand)->toString();
//         case STAR:
//             std::string operandString = std::get<std::unique_ptr<RegularExpression>>(this->leftOperand)->toString();

//             if (operandString == "λ" || operandString == "∅") return "∅";

//             else if (operandString.contains('+')) return "(" + operandString + ")*";

//             else return operandString + "*";
//     }
// };

// RegularExpression RegularExpression::fromString(std::string str)
// {
//     // TODO: simple paren depth parser and construct from tree
// };