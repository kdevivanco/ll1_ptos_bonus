#include <iostream>
#include "classes.cpp"

using namespace std;

// Primero generamos la gramatica con sus terminales y no terminales / reglas de prd: 
vector<ProdRule> generateGrammarRules() {   
    Symbol id("ID", TERMINAL);
    Symbol num("NUM", TERMINAL);
    Symbol assign("=", TERMINAL);
    Symbol print("print", TERMINAL);
    Symbol lparen("(", TERMINAL);
    Symbol rparen(")", TERMINAL);
    Symbol plus("+", TERMINAL);
    Symbol minus("-", TERMINAL);
    Symbol mul("*", TERMINAL);
    Symbol semicolon(";", TERMINAL);
    Symbol epsilon("epsilon", EPSILON);

    Symbol program("P", NON_TERMINAL);
    Symbol stmList("SL", NON_TERMINAL);
    Symbol stmListPrime("SL'", NON_TERMINAL);
    Symbol stmt("S", NON_TERMINAL);
    Symbol exp("E", NON_TERMINAL);
    Symbol expPrime("E'", NON_TERMINAL);
    Symbol term("T", NON_TERMINAL);
    Symbol termPrime("T'", NON_TERMINAL);
    Symbol factor("F", NON_TERMINAL);

    // Guardamos ls reglas de produccion en el vector de producction rules: 
    vector<ProdRule> rules;

    // P -> SL
    rules.emplace_back( program.value,vector<Symbol>{stmList});
    
    // SL -> S SL'
    rules.emplace_back( stmList.value,vector<Symbol>{stmt, stmListPrime});

    // SL' -> SL | epsilon
    rules.emplace_back( stmListPrime.value, vector<Symbol>{semicolon, stmt, stmListPrime});
    rules.emplace_back( stmListPrime.value,vector<Symbol>{epsilon});

    // S -> id = E | print(E)
    rules.emplace_back( stmt.value,vector<Symbol>{id, assign, exp});
    rules.emplace_back( stmt.value,vector<Symbol>{print, lparen, exp, rparen});

    // E -> T E' 
    rules.emplace_back( exp.value,vector<Symbol>{term, expPrime});

    // E' -> + T E' - T E'  | epsilon
    rules.emplace_back( expPrime.value,vector<Symbol>{plus, term, expPrime});
    rules.emplace_back( expPrime.value,vector<Symbol>{minus, term, expPrime});
    rules.emplace_back( expPrime.value,vector<Symbol>{epsilon});

    // T -> F T' 
    rules.emplace_back( term.value,vector<Symbol>{factor, termPrime});

    // T' -> * F T' | epsilon
    rules.emplace_back( termPrime.value,vector<Symbol>{mul, factor, termPrime});
    rules.emplace_back( termPrime.value, vector<Symbol>{epsilon});

    // F -> id  | num | (E)
    rules.emplace_back(factor.value,vector<Symbol>{id});
    rules.emplace_back(factor.value, vector<Symbol>{num});
    rules.emplace_back( factor.value,vector<Symbol>{lparen, exp, rparen});

    return rules;
}

int main() {
    //Test correcto:
    auto *grammar = new Grammar(generateGrammarRules());
    auto *scanner = new Scanner("x=5; print(x)");
    auto *parser = new Parser(scanner, grammar);

    parser->parse();

    delete grammar;
    delete scanner;
    delete parser;

    //Test con manejo de errores: 
    auto *grammar2 = new Grammar(generateGrammarRules());
    auto *scanner2 = new Scanner("x=5; print(x;)");
    auto *parser2 = new Parser(scanner2, grammar2);
    parser2->parse();
    delete grammar2;
    delete scanner2;
    delete parser2;

    
    return 0;
}

