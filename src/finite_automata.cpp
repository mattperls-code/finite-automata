#include <stdexcept>
#include <vector>
#include <queue>
#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <filesystem>

#include "finite_automata.hpp"

// utils

std::string concatStrSet(std::unordered_set<std::string> strSet, std::string delimiter)
{
    std::string concat;

    std::vector<std::string> strVec(strSet.begin(), strSet.end());
    std::sort(strVec.begin(), strVec.end());

    for (auto str : strVec) concat += str + delimiter;
    
    if (!strVec.empty()) concat = concat.substr(0, concat.size() - delimiter.size());

    return concat;
};

// transition class hash (see FiniteAutomata::getMinDfaEquivalenceClassIndexes)

size_t std::hash<std::unordered_map<Letter, int>>::operator()(const std::unordered_map<Letter, int>& transitionClass) const
{
    size_t hash = 0;

    for (auto [letter, endState] : transitionClass) hash ^= letter.value_or(256) * (endState + 1);

    return hash;
};

// edge

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

// finite automata

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

FiniteAutomata FiniteAutomata::compressNames()
{
    std::vector<std::string> originalStates(this->states.begin(), this->states.end());
    std::sort(originalStates.begin(), originalStates.end());

    std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    std::unordered_map<std::string, std::string> compressionMap;
    for (int i = 0;i<originalStates.size();i++) compressionMap[originalStates[i]] = originalStates.size() > alphabet.size() ? std::to_string(i) : std::string(1, alphabet[i]);

    std::unordered_set<std::string> compressedStates;
    for (auto originalState : this->states) compressedStates.insert(compressionMap[originalState]);

    std::string compressedStartState = compressionMap[this->startState];

    std::unordered_set<std::string> compressedAcceptingStates;
    for (auto originalAcceptingState : this->acceptingStates) compressedAcceptingStates.insert(compressionMap[originalAcceptingState]);

    std::unordered_set<Edge> compressedEdges;
    for (auto edge : this->edges) compressedEdges.insert(Edge(compressionMap[edge.start], compressionMap[edge.end], edge.letter));

    return FiniteAutomata(compressedStates, compressedStartState, compressedAcceptingStates, compressedEdges);
};

bool FiniteAutomata::hasLambdaMoves()
{
    for (auto edge : this->edges) if (!edge.letter.has_value()) return true;

    return false;
};

bool FiniteAutomata::isDeterministic()
{
    for (auto [_, transitions] : this->transitionTable) {
        if (transitions.contains({})) return false;
        
        for (auto [_, endStates] : transitions) if (endStates.size() > 1) return false;
    }

    return true;
};

std::string FiniteAutomata::addRe(std::string startState, RegularExpression re)
{
    auto type = re.getType();

    if (type == EMPTY) return this->addEmptyRe(startState);
    
    else if (type == CHARACTER) return this->addCharacterRe(startState, re.getCharacterExpression());
    
    else if (type == CONCAT) {
        auto [re1, re2] = re.getConcatExpression();

        return this->addConcatRe(startState, *re1, *re2);
    }

    else if (type == PLUS) {
        auto [re1, re2] = re.getPlusExpression();

        return this->addPlusRe(startState, *re1, *re2);
    }

    else return this->addStarRe(startState, *re.getStarExpression());
};

std::string FiniteAutomata::addEmptyRe(std::string startState)
{
    std::string nextState = startState + "-c";

    this->states.insert(nextState);
    
    this->edges.insert(Edge(startState, nextState, {}));

    return nextState;
};

std::string FiniteAutomata::addCharacterRe(std::string startState, char characterExpression)
{
    std::string nextState = startState + "-c";

    this->states.insert(nextState);
    
    this->edges.insert(Edge(startState, nextState, characterExpression));

    return nextState;
};

std::string FiniteAutomata::addConcatRe(std::string startState, RegularExpression re1, RegularExpression re2)
{
    std::string nextState = this->addRe(startState, re1);
    std::string nextNextState = this->addRe(nextState, re2);

    return nextNextState;
};

std::string FiniteAutomata::addPlusRe(std::string startState, RegularExpression re1, RegularExpression re2)
{
    std::string branchStartState1 = startState + "-b0";
    std::string branchStartState2 = startState + "-b1";
    
    this->states.insert(branchStartState1);
    this->states.insert(branchStartState2);

    this->edges.insert(Edge(startState, branchStartState1, {}));
    this->edges.insert(Edge(startState, branchStartState2, {}));

    std::string branchEndState1 = this->addRe(branchStartState1, re1);
    std::string branchEndState2 = this->addRe(branchStartState2, re2);

    std::string branchCombineState = startState + "-c";

    this->states.insert(branchCombineState);

    this->edges.insert(Edge(branchEndState1, branchCombineState, {}));
    this->edges.insert(Edge(branchEndState2, branchCombineState, {}));

    return branchCombineState;
};

std::string FiniteAutomata::addStarRe(std::string startState, RegularExpression re)
{
    std::string nextState = this->addRe(startState, re);

    this->edges.insert(Edge(startState, nextState, {}));
    this->edges.insert(Edge(nextState, startState, {}));

    return nextState;
};

FiniteAutomata FiniteAutomata::re2lnfa(RegularExpression re)
{
    FiniteAutomata lnfa = FiniteAutomata({ "START" }, "START", {}, {});

    std::string lnfaAcceptingState = lnfa.addRe("START", re);
    
    lnfa.acceptingStates.insert(lnfaAcceptingState);

    return lnfa.compressNames();
};

FiniteAutomata FiniteAutomata::lnfa2renfa()
{
    // assume $START and $ACCEPT are not taken (should be guaranteed by FiniteAutomata::create, no internal methods add "$")
    std::string renfaStartState = "$START";
    std::string renfaAcceptState = "$ACCEPT";

    auto renfaStates = this->states;
    renfaStates.insert({ renfaStartState, renfaAcceptState });

    std::unordered_set<std::string> renfaAcceptingStates = { renfaAcceptState };
    auto renfaEdges = this->edges;

    // ensure start and accept state are "pulled out" so renfa characteristics are met

    renfaEdges.insert(Edge(renfaStartState, this->startState, {}));

    for (auto originalAcceptingState : this->acceptingStates) renfaEdges.insert(Edge(originalAcceptingState, renfaAcceptState, {}));

    return FiniteAutomata(renfaStates, renfaStartState, renfaAcceptingStates, renfaEdges);
};

RegularExpression FiniteAutomata::lnfa2re()
{
    auto renfa = this->lnfa2renfa();

    // [startState][endState] = re
    std::unordered_map<std::string, std::unordered_map<std::string, RegularExpression>> reTransitionTable;

    // [endState][startState] = re
    std::unordered_map<std::string, std::unordered_map<std::string, RegularExpression>> reInvertedTransitionTable;

    for (auto edge : renfa.edges) {
        auto edgeRe = edge.letter.has_value() ? RegularExpression::character(edge.letter.value()) : RegularExpression::empty();

        // combine parallel edges with plus
        auto updatedTransitionRe = reTransitionTable[edge.start].contains(edge.end) ? RegularExpression::plus(reTransitionTable[edge.start][edge.end], edgeRe) : edgeRe;
        
        reTransitionTable[edge.start][edge.end] = updatedTransitionRe;
        reInvertedTransitionTable[edge.end][edge.start] = updatedTransitionRe;
    }

    // pruning could be done here by taking the intersection of reachable states traversing both directions, effectively removing "dead ends"

    auto internalStates = renfa.states;
    internalStates.erase(renfa.startState);
    internalStates.erase(*renfa.acceptingStates.begin());

    // "splice out" each internal state and insert new edges for every combination of incoming and outgoing edges
    for (auto internalState : internalStates) {
        // if the state being spliced has a self edge, the regular expression for that edge is starred and placed between the left and right expressions being joined
        auto selfLoopRe = reTransitionTable[internalState].contains(internalState) ? RegularExpression::star(reTransitionTable[internalState][internalState]) : RegularExpression::empty();

        auto transitionsEndingAtInternalState = reInvertedTransitionTable[internalState];
        auto transitionsStartingAtInternalState = reTransitionTable[internalState];

        transitionsEndingAtInternalState.erase(internalState);
        transitionsStartingAtInternalState.erase(internalState);

        // new edge to join each in-edge to each out-edge
        for (auto [stateEndingAtInternalState, leftRe] : transitionsEndingAtInternalState) {
            auto intermediaryJoiningRe = RegularExpression::concat(leftRe, selfLoopRe);

            for (auto [stateStartingAtInternalState, rightRe] : transitionsStartingAtInternalState) {
                auto completeJoiningRe = RegularExpression::concat(intermediaryJoiningRe, rightRe);

                auto updatedTransitionRe = reTransitionTable[stateEndingAtInternalState].contains(stateStartingAtInternalState) ? RegularExpression::plus(reTransitionTable[stateEndingAtInternalState][stateStartingAtInternalState], completeJoiningRe) : completeJoiningRe;

                reTransitionTable[stateEndingAtInternalState][stateStartingAtInternalState] = updatedTransitionRe;
                reInvertedTransitionTable[stateStartingAtInternalState][stateEndingAtInternalState] = updatedTransitionRe;
            }
        }

        // cleanup old edges, (first two erase calls dont really change anything but makes the map a bit neater)
        reTransitionTable.erase(internalState);
        reInvertedTransitionTable.erase(internalState);
        for (auto [stateEndingAtInternalState, _] : transitionsEndingAtInternalState) reTransitionTable[stateEndingAtInternalState].erase(internalState);
        for (auto [stateStartingAtInternalState, _] : transitionsStartingAtInternalState) reInvertedTransitionTable[stateStartingAtInternalState].erase(internalState);
    }

    return reTransitionTable[renfa.startState][*renfa.acceptingStates.begin()];
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

FiniteAutomata FiniteAutomata::lnfa2nfa()
{
    if (!this->hasLambdaMoves()) return *this;

    // some caching for transitive closure should be done here (store map of lambda traversal in both directions for every state)

    std::unordered_set<std::string> nfaAcceptingStates;

    // anything that can reach an accepting state via lambda moves is transitively accepting
    for (auto acceptingState : this->acceptingStates) nfaAcceptingStates.merge(this->getStatesTransitivelyEndingAt(acceptingState, {}));

    std::unordered_set<Edge> nfaEdges;

    // for every non lambda edge from S via L to E, there should be an edge:
    // from anything that can reach S via lambda moves, via L, to anything that can be reached from E via lambda moves
    for (auto edge : this->edges) {
        if (!edge.letter.has_value()) continue;
        
        for (auto startState : this->getStatesTransitivelyEndingAt(edge.start, {})) {
            for (auto endState : this->getStatesTransitivelyStartingAt(edge.end, {})) {
                nfaEdges.insert(Edge(startState, endState, edge.letter));
            }
        }
    }

    return FiniteAutomata(this->states, this->startState, nfaAcceptingStates, nfaEdges);
};

FiniteAutomata FiniteAutomata::nfa2dfa()
{
    if (this->hasLambdaMoves()) throw std::runtime_error("FiniteAutomata nfa2dfa: only callable for ordinary NFA");

    if (this->isDeterministic()) return *this;

    std::unordered_set<std::string> dfaStates;
    std::string dfaStartState = "{" + this->startState + "}";
    std::unordered_set<std::string> dfaAcceptingStates;
    std::unordered_set<Edge> dfaEdges;

    // basically a normal bfs but "current" is a SET of states and traversals are the union of all moves within that set for a given letter
    
    // useful to note that by nature of taking bfs from start state, unreachable states are automatically pruned

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
    // ensures minimality since otherwise it might generate classes that arent actually reachable
    auto reachableStates = this->getStatesTransitivelyStartingAt(this->startState);

    // [state] = equivalenceClassIndex
    std::unordered_map<std::string, int> equivalenceClassIndexes;

    // initial partition
    int numEquivalenceClasses = 2;
    for (auto state : reachableStates) equivalenceClassIndexes[state] = this->acceptingStates.contains(state);

    // continue partitioning until minimal equivalence classes are found
    while (true) {
        // partition by:
        //      what equivalence class does a given letter result in        (transition class)
        //      are states in this equivalence class accepting or not       (accepts)
        std::unordered_map<std::unordered_map<Letter, int>, std::unordered_set<std::string>> acceptingEquivalenceClasses;
        std::unordered_map<std::unordered_map<Letter, int>, std::unordered_set<std::string>> nonAcceptingEquivalenceClasses;

        for (auto state : reachableStates) {
            std::unordered_map<Letter, int> transitionClass;

            for (auto [letter, endStates] : this->transitionTable[state]) transitionClass[letter] = equivalenceClassIndexes[*endStates.begin()];

            auto& equivalenceClassesFamily = this->acceptingStates.contains(state) ? acceptingEquivalenceClasses : nonAcceptingEquivalenceClasses;

            equivalenceClassesFamily[transitionClass].insert(state);
        }

        int newNumEquivalenceClasses = acceptingEquivalenceClasses.size() + nonAcceptingEquivalenceClasses.size();

        // if there exists a further partition, the number of equivalence classes must change
        // therefore this condition implies the minimal equivalence classes have been found
        if (newNumEquivalenceClasses == numEquivalenceClasses) return equivalenceClassIndexes;

        numEquivalenceClasses = newNumEquivalenceClasses;

        // reassign equivalence class indexes for next round of partitioning

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

    // use a representative from each equivalence class to reconstruct the transition behavior and whether it accepts
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

FiniteAutomata FiniteAutomata::dfa2complement()
{
    if (!this->isDeterministic()) throw std::runtime_error("FiniteAutomata complement: only callable for DFA");

    std::unordered_map<char, std::unordered_set<std::string>> presentTransitions;
    for (auto edge : this->edges) presentTransitions[edge.letter.value()].insert(edge.start);
    
    auto complementStates = this->states;
    auto complementEdges = this->edges;

    // if it is not fully connected, add implied edges to emptyset, that way complement can include emptyset
    if (this->edges.size() != this->states.size() * presentTransitions.size()) {
        complementStates.insert("$EMPTY"); // assume state namespace doesnt have $EMPTY
        
        for (auto [letter, transitioningStates] : presentTransitions) {
            for (auto complementState : complementStates) {
                if (!transitioningStates.contains(complementState)) complementEdges.insert(Edge(complementState, "$EMPTY", letter));
            }
        }
    }

    // invert accepting
    std::unordered_set<std::string> complementAcceptingStates;
    for (auto complementState : complementStates) if (!this->acceptingStates.contains(complementState)) complementAcceptingStates.insert(complementState);

    return FiniteAutomata(complementStates, this->startState, complementAcceptingStates, complementEdges);
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

bool FiniteAutomata::isIsomorphism(FiniteAutomata dfa1, FiniteAutomata dfa2)
{
    if (!dfa1.isDeterministic() || !dfa2.isDeterministic()) throw std::runtime_error("FiniteAutomata isIsomorphism: only callable on DFAs");

    // run bfs on both dfa graphs and look for structural differences

    std::unordered_set<std::string> visited1;
    std::unordered_set<std::string> visited2;

    std::queue<std::string> queue1;
    std::queue<std::string> queue2;

    queue1.push(dfa1.startState);
    queue2.push(dfa2.startState);

    while (!queue1.empty()) {
        auto currentState1 = queue1.front();
        auto currentState2 = queue2.front();

        queue1.pop();
        queue2.pop();

        if (dfa1.acceptingStates.contains(currentState1) != dfa2.acceptingStates.contains(currentState2)) return false;

        bool isVisited1 = visited1.contains(currentState1);
        bool isVisited2 = visited2.contains(currentState2);

        if (isVisited1 != isVisited2) return false;

        if (isVisited1) continue;

        visited1.insert(currentState1);
        visited2.insert(currentState2);

        auto transitions1 = dfa1.transitionTable[currentState1];
        auto transitions2 = dfa2.transitionTable[currentState2];

        std::vector<Letter> letters1;
        std::vector<Letter> letters2;

        for (auto [letter1, _] : transitions1) letters1.push_back(letter1);
        for (auto [letter2, _] : transitions2) letters2.push_back(letter2);

        // insertion into the queue must be ordered the same, so we sort the edges first

        std::sort(letters1.begin(), letters1.end());
        std::sort(letters2.begin(), letters2.end());

        if (letters1 != letters2) return false;

        for (auto letter1 : letters1) queue1.push(*transitions1[letter1].begin());
        for (auto letter2 : letters2) queue2.push(*transitions2[letter2].begin());
    }

    return true;
};

bool FiniteAutomata::isLanguageEquivalence(FiniteAutomata fa1, FiniteAutomata fa2)
{
    auto dfa1 = fa1.lnfa2nfa().nfa2dfa().dfa2minDfa();
    auto dfa2 = fa2.lnfa2nfa().nfa2dfa().dfa2minDfa();

    return isIsomorphism(dfa1, dfa2);
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

std::string FiniteAutomata::toDOT()
{
    std::string output;

    output += "digraph FiniteAutomata {";

    output += "\n";

    output += "\trankdir=LR;";

    output += "\n";

    output += "\tnodesep=1.0;";

    output += "\n";

    output += "\tranksep=1.0;";

    output += "\n";

    output += "\t\"$\" [shape=point, style=invis, width=0];";

    output += "\n";

    output += "\t\"$\" -> \"" + this->startState + "\";";

    output += "\n";

    std::unordered_set<std::string> acceptingStateStrSet;
    for (auto acceptingState : this->acceptingStates) acceptingStateStrSet.insert("\t\"" + acceptingState + "\" [penwidth=5];");

    output += concatStrSet(acceptingStateStrSet, "\n");

    output += "\n";

    std::unordered_set<std::string> edgeDotSet;

    // start -> transition class, letter end states

    for (auto [startState, transitions] : this->transitionTable) {
        std::unordered_map<std::string, std::unordered_set<Letter>> parallelEdges;

        for (auto [letter, endStates] : transitions) for (auto endState : endStates) parallelEdges[endState].insert(letter);

        for (auto [endState, letters] : parallelEdges) {
            std::unordered_set<std::string> lettersStrSet;
            for (auto letter : letters) lettersStrSet.insert(letter.has_value() ? std::string(1, letter.value()) : "λ");

            edgeDotSet.insert("\t\"" + startState + "\" -> \"" + endState + "\" [label=\"" + concatStrSet(lettersStrSet, ",") + "\"];");
        }
    }

    output += concatStrSet(edgeDotSet, "\n");

    output += "\n";

    output += "}";

    return output;
};

void FiniteAutomata::exportGraph(std::string outputDirPath, std::string outputFileName) {
    std::filesystem::create_directories(outputDirPath);

    std::string dotOutputFilePath = outputDirPath + "/" + outputFileName + ".dot";

    std::ofstream dotOutputFile(dotOutputFilePath);

    dotOutputFile << this->toDOT();

    dotOutputFile.close();

    std::string renderDotFileCommand = "dot -Tpng " + dotOutputFilePath + " -o " + outputDirPath + "/" + outputFileName + ".png";

    std::system(renderDotFileCommand.c_str());
};