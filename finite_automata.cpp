#include <stdexcept>
#include <vector>
#include <queue>
#include <algorithm>

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

// transitionClass hash

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

// regexp

RegularExpression RegularExpression::empty()
{
    return RegularExpression(RegularExpressionType::EMPTY, {});
};

RegularExpression RegularExpression::character(char c)
{
    return RegularExpression(RegularExpressionType::CHARACTER, c);
};

RegularExpression RegularExpression::concat(RegularExpression re1, RegularExpression re2)
{
    if (re1.type == EMPTY) return re2;
    if (re2.type == EMPTY) return re1;

    return RegularExpression(RegularExpressionType::CONCAT, std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> {
        std::make_shared<RegularExpression>(re1),
        std::make_shared<RegularExpression>(re2)
    });
};

RegularExpression RegularExpression::plus(RegularExpression re1, RegularExpression re2)
{
    return RegularExpression(RegularExpressionType::PLUS, std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> {
        std::make_shared<RegularExpression>(re1),
        std::make_shared<RegularExpression>(re2)
    });
};

RegularExpression RegularExpression::star(RegularExpression re)
{
    // TODO: handle lambda star

    return RegularExpression(RegularExpressionType::STAR, std::make_shared<RegularExpression>(re));
};

RegularExpression RegularExpression::fromToken(Token token)
{
    if (token.id == "CHAR") return RegularExpression::character(token.getStringLiteralContent()[0]);

    std::vector<RegularExpression> nestedExpressions;
    for (auto nestedToken : token.getNestingContent()) nestedExpressions.push_back(RegularExpression::fromToken(nestedToken));

    if (token.id == "CONCAT") return RegularExpression::concat(nestedExpressions[0], nestedExpressions[1]);

    if (token.id == "PLUS") return RegularExpression::plus(nestedExpressions[0], nestedExpressions[1]);
    
    if (token.id == "STAR") return RegularExpression::star(nestedExpressions[0]);

    return RegularExpression::empty();
};

RegularExpression RegularExpression::fromExpressionString(std::string expressionStr)
{
    ParserCombinator expression;
    
    auto whitespace = repetition(satisfy(is(' ')));

    auto characterExpression = satisfy("CHAR", isalnum);

    // λ
    auto lambdaExpression = sequence("EMPTY", {
        satisfy(is((char) 206)),
        satisfy(is((char) 187))
    });

    auto groupExpression = sequence({
        satisfy(is('(')),
        proxyParserCombinator(&expression),
        satisfy(is(')'))
    });

    auto atom = choice({
        characterExpression,
        lambdaExpression,
        groupExpression
    });

    auto starExpression = atom.followedBy("STAR", satisfy(is('*')));

    auto atomOrStarExpression = choice({
        starExpression,
        atom
    });

    ParserCombinator concatExpression = sequence("CONCAT", {
        atomOrStarExpression,
        whitespace,
        choice({
            proxyParserCombinator(&concatExpression),
            atomOrStarExpression
        })
    });

    ParserCombinator plusExpression = sequence("PLUS", {
        choice({
            concatExpression,
            atomOrStarExpression
        }),
        satisfy(is('+')).surroundedBy(whitespace),
        choice({
            proxyParserCombinator(&plusExpression),
            concatExpression,
            atomOrStarExpression
        })
    });

    expression = choice({
        plusExpression,
        concatExpression,
        starExpression,
        atom
    }).surroundedBy(whitespace);

    auto grammar = strictlySequence({
        expression,
        satisfy(is('\0'))
    });
    
    auto parseResult = parse(expressionStr + '\0', grammar);

    if (getResultType(parseResult) == PARSER_FAILURE) throw std::runtime_error("RegularExpression fromExpressionString: " + getParserFailureFromResult(parseResult).toString());

    return RegularExpression::fromToken(getTokenFromResult(parseResult).getNestingContent()[0]);
};

RegularExpressionType RegularExpression::getType()
{
    return this->type;
};

char RegularExpression::getCharacterExpression()
{
    return std::get<char>(this->value);
};

std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> RegularExpression::getConcatExpression()
{
    return std::get<std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>>>(this->value);
};

std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> RegularExpression::getPlusExpression()
{
    return std::get<std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>>>(this->value);
};

std::shared_ptr<RegularExpression> RegularExpression::getStarExpression()
{
    return std::get<std::shared_ptr<RegularExpression>>(this->value);
};

std::string RegularExpression::toString()
{
    if (this->type == EMPTY) return "λ";

    if (this->type == CHARACTER) return std::string(1, std::get<char>(this->value));

    if (this->type == STAR) {
        auto operandString = std::get<std::shared_ptr<RegularExpression>>(this->value)->toString();

        if (operandString.size() == 1 || (!operandString.empty() && operandString[0] == '(')) return operandString + "*";

        return "(" + operandString + ")*";
    }

    auto operands = std::get<std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>>>(this->value);

    auto leftOperandString = operands.first->toString();
    auto rightOperandString = operands.second->toString();

    if (this->type == CONCAT) return leftOperandString + rightOperandString;

    return "(" + leftOperandString + "+" + rightOperandString + ")";
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
    FiniteAutomata lnfa = FiniteAutomata({ "ROOT" }, "ROOT", {}, {});

    std::string lnfaAcceptingState = lnfa.addRe("ROOT", re);
    
    lnfa.acceptingStates.insert(lnfaAcceptingState);

    return lnfa.compressNames();
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

    // some caching for transitive closure can be done here

    std::unordered_set<std::string> nfaAcceptingStates;

    for (auto acceptingState : this->acceptingStates) nfaAcceptingStates.merge(this->getStatesTransitivelyEndingAt(acceptingState, {}));

    std::unordered_set<Edge> nfaEdges;

    for (auto edge : this->edges) {
        if (!edge.letter.has_value()) continue; // only construct non lambda edges
        
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