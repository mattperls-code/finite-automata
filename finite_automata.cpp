#include <stdexcept>
#include <functional>
#include <iostream>

#include "./finite_automata.hpp"

bool TransitionToken::isLambda()
{
    return !this->isChar;
};

char TransitionToken::getChar()
{
    if (this->isLambda()) throw std::runtime_error("getChar called on lambda transition token");

    return this->c;
};

TransitionToken TransitionToken::lambda()
{
    return TransitionToken();
};

TransitionToken TransitionToken::character(char c)
{
    return TransitionToken(c);
};

bool FiniteAutomata::hasState(std::string stateName)
{
    return this->stateNameToIndexMap.contains(stateName);
};

FiniteAutomata::FiniteAutomata(std::unordered_set<char> alphabet, std::unordered_set<std::string> stateNames, std::string startStateName, std::unordered_set<std::string> finalStateNames, std::vector<NamedEdge> namedEdges)
{
    this->alphabet = alphabet;

    if (stateNames.empty()) throw std::runtime_error("no states provided in finite automata constructor call");
    
    for (auto name : stateNames) {
        this->stateNameToIndexMap[name] = this->stateIndexToNameMap.size();
        this->stateIndexToNameMap.push_back(name);
    }

    if (!this->hasState(startStateName)) throw std::runtime_error("unknown start state in finite automata constructor call");

    this->startStateName = startStateName;
    this->startStateIndex = this->stateNameToIndexMap[this->startStateName];

    for (auto finalStateName : finalStateNames) {
        if (!this->hasState(finalStateName)) throw std::runtime_error("unknown final state in finite automata constructor call");

        this->finalStateNames.insert(finalStateName);
        this->finalStateIndices.insert(this->stateNameToIndexMap[finalStateName]);
    }

    for (auto namedEdge : namedEdges) {
        if (!this->hasState(namedEdge.startState)) throw std::runtime_error("unknown start state in named edge in finite automata constructor call");
        if (!this->hasState(namedEdge.endState)) throw std::runtime_error("unknown end state in named edge in finite automata constructor call");

        if (!namedEdge.transitionToken.isLambda() && !this->alphabet.contains(namedEdge.transitionToken.getChar())) throw std::runtime_error("unknown transition token character in named edge in finite automata constructor call");

        this->namedEdges.push_back(namedEdge);
        this->indexedEdges.push_back(IndexedEdge(
            this->stateNameToIndexMap[namedEdge.startState],
            this->stateNameToIndexMap[namedEdge.endState],
            namedEdge.transitionToken
        ));
    }

    this->edgeTable.resize(this->stateIndexToNameMap.size());

    for (auto indexedEdge : this->indexedEdges) this->edgeTable[indexedEdge.startState].push_back(indexedEdge);
};

bool FiniteAutomata::isValidDFA()
{
    for (auto outgoingEdgesSet : this->edgeTable) {
        std::unordered_set<char> transitionTokenCharacters;

        for (auto indexedEdge : outgoingEdgesSet) {
            if (indexedEdge.transitionToken.isLambda()) return false;

            char transitionTokenCharacter = indexedEdge.transitionToken.getChar();
            
            // nfa if two distinct outgoing edges share a transition token character
            if (transitionTokenCharacters.contains(transitionTokenCharacter)) return false;
        }
    }

    return true;
};

int FiniteAutomata::nextState(int currentStateIndex, char c)
{
    auto currentOutgoingEdges = this->edgeTable[currentStateIndex];

    for (auto outgoingEdge : currentOutgoingEdges) {
        // assume first found is valid since isValidDFA checked before
        if (outgoingEdge.transitionToken.getChar() == c) return outgoingEdge.endState;
    }

    return -1;
};

bool FiniteAutomata::matches(std::string str)
{
    if (!this->isValidDFA()) throw std::runtime_error("string matching is only supported for DFAs");

    int currentStateIndex = this->startStateIndex;

    for (char c : str) {
        currentStateIndex = this->nextState(currentStateIndex, c);

        if (currentStateIndex == -1) return false; // no matching outgoing edge
    }

    return this->finalStateIndices.contains(currentStateIndex);
};

FiniteAutomata FiniteAutomata::nfa2dfa()
{
    // TODO
};

class EqClass
{
    public:
        bool isFinal;

        // letter -> eqClassIndex
        std::unordered_map<char, int> transitions; // states in this class have a directed edge to eqClassIndex via letter

        EqClass(bool isFinal, std::unordered_map<char, int> transitions): isFinal(isFinal), transitions(transitions) {};

        bool equals(const EqClass& other)
        {
            if (this->isFinal != other.isFinal) return false;

            if (this->transitions.size() != other.transitions.size()) return false;

            for (auto [letter, eqClassIndex] : this->transitions) {
                if (!other.transitions.contains(letter) || other.transitions.at(letter) != eqClassIndex) return false;
            }

            return true;
        };

        int indexIn(const std::vector<EqClass>& objs)
        {
            for (int i = 0;i<objs.size();i++) if (this->equals(objs[i])) return i;

            return -1;
        };
};

FiniteAutomata FiniteAutomata::minimizeDFA()
{
    if (!this->isValidDFA()) throw std::runtime_error("minimize should only be called on valid DFAs");

    // TODO: bfs to get reachable states beforehand
    
    std::vector<int> stateToEqClassIndexMap;

    // initial partition, non final state class index = 0, final state class index = 1
    for (int i = 0;i<this->stateIndexToNameMap.size();i++) stateToEqClassIndexMap.push_back(this->finalStateIndices.contains(i));

    int numEqClasses = 2;

    // repartition until partitioning does not create new equivalence classes
    while (true) {
        std::vector<EqClass> eqClasses;
        std::vector<int> newStateToEqClassIndexMap;

        for (int i = 0;i<this->stateIndexToNameMap.size();i++) {
            bool isFinal = this->finalStateIndices.contains(i);

            std::unordered_map<char, int> transitions;

            // for every outgoing edge, add (letter of that edge -> equivalence class index of the outgoing state of that edge)
            for (auto indexedEdge : this->edgeTable[i]) transitions[indexedEdge.transitionToken.getChar()] = stateToEqClassIndexMap[indexedEdge.endState];

            EqClass eqClass(isFinal, transitions);

            // this could be done via table lookup, but the resulting multimaps are hard to follow
            int indexOfEqClass = eqClass.indexIn(eqClasses);

            if (indexOfEqClass == -1) {
                newStateToEqClassIndexMap.push_back(eqClasses.size());
                eqClasses.push_back(eqClass);
            }
            else newStateToEqClassIndexMap.push_back(indexOfEqClass);
        }

        stateToEqClassIndexMap = newStateToEqClassIndexMap;

        // partitions changed, continue to next round
        if (eqClasses.size() > numEqClasses) {
            numEqClasses = eqClasses.size();

            continue;
        }
        
        std::unordered_set<std::string> minDfaStateNames;
        std::unordered_set<std::string> minDfaFinalStateNames;
        std::vector<NamedEdge> minDfaNamedEdges;

        std::string minDfaStartStateName = "S_" + std::to_string(stateToEqClassIndexMap[this->startStateIndex]);

        for (int i = 0;i<eqClasses.size();i++) {
            std::string stateName = "S_" + std::to_string(i);

            minDfaStateNames.insert(stateName);
            
            if (eqClasses[i].isFinal) minDfaFinalStateNames.insert(stateName);

            for (auto [letter, eqClassIndex] : eqClasses[i].transitions) minDfaNamedEdges.push_back(NamedEdge(stateName, "S_" + std::to_string(eqClassIndex), TransitionToken::character(letter)));
        }

        return FiniteAutomata(this->alphabet, minDfaStateNames, minDfaStartStateName, minDfaFinalStateNames, minDfaNamedEdges);
    }
};

std::string FiniteAutomata::toString()
{
    std::string output;

    output += "States: ";

    for (auto stateName : this->stateIndexToNameMap) output += stateName + ", ";

    output.pop_back();
    output.pop_back();

    output += "\n";

    output += "Start State: " + this->startStateName + "\n";

    output += "Final States: ";

    for (auto stateName : this->finalStateNames) output += stateName + ", ";

    output.pop_back();
    output.pop_back();

    output += "\n";

    output += "Edges:\n";

    for (auto namedEdge : this->namedEdges) output += "\tFrom " + namedEdge.startState + " to " + namedEdge.endState + " on " + (namedEdge.transitionToken.isLambda() ? "lambda" : std::string(1, namedEdge.transitionToken.getChar())) + "\n";

    output.pop_back();

    return output;
};