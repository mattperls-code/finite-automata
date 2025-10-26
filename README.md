# Finite Automata

## Bidirectional Conversions Between:
 - expression string    (regular expression in human readable form)
 - regular expression   (regular expression in tree form)
 - λNFA                 (finite automata with λ edges)
 - reNFA                (λNFA where start state in-degree is zero and single accept state out-degree is zero)
 - NFA                  (finite automata without λ edges)
 - DFA                  (finite automata with no ambiguous edges)
 - min DFA              (DFA that perserves it language using the minimal number states)
 - dfa complement       (the DFA representing the complement of the original language)

## Examples

### Expression String to Regular Expression

"a (b (b* + a + λ) + λ(a + (ab + b + λ)* bb)) b(ab)*"

<img src="./examples/str2re/output.png" width="800">

### Regular Expression to λNFA

<img src="./examples/re2lnfa/input.png" width="800"/>

<img src="./examples/re2lnfa/output.png" width="800"/>

### λNFA to NFA

<img src="./examples/lnfa2nfa/input.png" width="800"/>

<img src="./examples/lnfa2nfa/output.png" width="800"/>

### NFA to DFA

<img src="./examples/nfa2dfa/input.png" width="800"/>

<img src="./examples/nfa2dfa/output.png" width="800"/>

### DFA to Min DFA

<img src="./examples/dfa2minDfa/input.png" width="800"/>

<img src="./examples/dfa2minDfa/output.png" width="800"/>

### DFA to Regular Expression

<img src="./examples/dfa2re/input.png" width="800"/>

<img src="./examples/dfa2re/output.png" width="800"/>

### DFA to Complement

<img src="./examples/dfa2complement/input.png" width="800"/>

<img src="./examples/dfa2complement/output.png" width="800"/>

### Match N Mod M

N = { 1, 5 }, M = 6

<img src="./examples/matches/input.png" width="800"/>
