##Gramatica:

P -> SL
SL -> S SL'
SL' -> ; Stmt StmtList' |  ε
S -> id = E | print( E )
E -> T E'
E' -> + T E'  | - Term Exp' | ε
T -> F T'
T' -> * F T'  | ε
F -> id | Num | ( Exp )

