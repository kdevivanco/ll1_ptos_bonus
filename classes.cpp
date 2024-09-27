#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <stack>

using namespace std;

static unordered_set<string> TERMINALS = {"ID", "NUM", "=", ";", "(", ")", "+", "-", "*", "print", "$"};

enum Type {
    DOLAR,
    TERMINAL, 
    NON_TERMINAL, 
    EPSILON
   
};

struct Symbol {
    string value;
    Type type;

    Symbol(string v, Type t){
        value = v;
        type = t;
    }

    bool operator==(const Symbol &rhs) const {
        // return value == rhs.value && type == rhs.type;
        bool typematch = type == rhs.type;
        bool valuematch = value == rhs.value;
        return typematch && valuematch;
    }
};

struct HashSym {
    size_t operator()(const Symbol &symbol) const {
        size_t ret = hash<string>()(symbol.value) ^ std::hash<int>()(static_cast<int>(symbol.type));
        return ret; 
    }
};

struct EqSym {
    bool operator()(const Symbol &lhs, const Symbol &rhs) const {
        return lhs.value == rhs.value && lhs.type == rhs.type;
    }
};

using SetSym = unordered_set<Symbol, HashSym, EqSym>;

struct ProdRule {
    string lhs;
    vector<Symbol> rhs;

    ProdRule(string l, vector<Symbol> r){
        lhs = l;
        rhs = r;
    }

    bool operator==(const ProdRule &_rhs) const {
        bool eq = false;
        if (lhs == _rhs.lhs && rhs == _rhs.rhs){
            eq = true;
        }
        return eq;
    }
};

class Grammar {
    public:
        vector<ProdRule> rules;
        unordered_set<string> terminals;
        unordered_set<string> nonTerminals;
        unordered_map<string, SetSym> FIRST;
        unordered_map<string, SetSym> FOLLOW;

        Grammar() = default;
        Grammar(vector<ProdRule> r){
            rules = r;
            for (auto &rule: rules) {
                this->nonTerminals.insert(rule.lhs);
                for (const auto &symbol: rule.rhs) {
                    if (symbol.type == NON_TERMINAL){
                        this->nonTerminals.insert(symbol.value);
                        }
                    else if (symbol.type == TERMINAL) {
                        this->terminals.insert(symbol.value);
                        }
                }
            }
            terminals.insert("epsilon");

            calcFirst();
            calcFollow();
        }

        static bool isTerminal(const string &symbol){
                return TERMINALS.find(symbol) != TERMINALS.end();
            }

        const vector<ProdRule> &grammarRules(){
            return rules;
        }

        SetSym calcFirst(const vector<Symbol> &rhs){
            SetSym firstSet;
            bool canContainEpsilon = true;

            for (const auto &symbol: rhs) {
                if (symbol.type == TERMINAL) {
                    firstSet.insert(symbol);
                    canContainEpsilon = false;
                    break;
                }

                if (symbol.type == EPSILON) {
                    firstSet.insert(symbol);
                    break;
                }

                if (FIRST.find(symbol.value) == FIRST.end()) {
                    for (const ProdRule &rule: grammarRule(symbol.value)) {
                        SetSym firstY = calcFirst(rule.rhs);
                        FIRST[symbol.value].insert(firstY.begin(), firstY.end());
                    }
                }

                const SetSym &firstY = FIRST[symbol.value];
                bool hasEpsilon = false;
                for (const Symbol &sym: firstY) {
                    if (sym.type != EPSILON) firstSet.insert(sym);
                    else hasEpsilon = true;
                }

                if (!hasEpsilon) {
                    canContainEpsilon = false;
                    break;
                }
            }

            if (canContainEpsilon) firstSet.insert({"epsilon", EPSILON});
            FIRST[rhs[0].value].insert(firstSet.begin(), firstSet.end());
            return firstSet;
        }

        SetSym calcFollow(const string &lhs){
            SetSym follow;
            if (FOLLOW.find(lhs) != FOLLOW.end()){
                return FOLLOW[lhs];
            }
            if (lhs == rules[0].lhs){
                follow.insert({"$", DOLAR});
                }

            for (const auto &rule: rules) {
                if (rule.lhs == lhs) continue;

                for (int i = 0; i < rule.rhs.size(); i++) {
                    const vector<Symbol> &thisrhs = rule.rhs;

                    if (thisrhs[i].value == lhs) {
                        const string &currentLHS = rule.lhs;

                        if (i == thisrhs.size() - 1) {
                            SetSym followY = calcFollow(currentLHS);
                            follow.insert(followY.begin(), followY.end());
                        } else {
                            vector<Symbol> gamma(
                                    thisrhs.begin() + i + 1,
                                    rule.rhs.end());

                            SetSym firstGamma = calcFirst(gamma);
                            bool hasEpsilon = false;

                            for (const Symbol &symbol: firstGamma) {
                                if (symbol.type != EPSILON) follow.insert(symbol);
                                else hasEpsilon = true;
                            }

                            if (hasEpsilon) {
                                SetSym followY = calcFollow(currentLHS);
                                follow.insert(followY.begin(), followY.end());
                            }
                        }
                    }
                }
            }

            FOLLOW[lhs] = follow;
            return follow;
            
        }

        //calculamos el first recursivamente
        void calcFirst(){
            for (const auto &rule: rules) {
                SetSym firstTemp = calcFirst(rule.rhs);
                FIRST[rule.lhs].insert(firstTemp.begin(), firstTemp.end());
            }
        }

        //calculamos el follow recursivamente
        void calcFollow(){
            for (const auto &rule: rules){
                calcFollow(rule.lhs);
            }
        }
        vector<ProdRule> grammarRule(const string &lhs){
            vector<ProdRule> matchingRules;
            copy_if(rules.begin(), rules.end(),
                        back_inserter(matchingRules),
                        [&lhs](const ProdRule &rule) { 
                            return rule.lhs == lhs; 
                            }
            );
            return matchingRules;
        }

};


class Token {
public:
    enum Type {
        ID, NUM, ASSIGN, SEMI, LP, RP, PLUS, MIN, MUL, ERR, PRINT, END};

    Type type;
    string lexeme;

    // Constructor
    explicit Token(Type t){
        type = t;
    }

    Token(Type t, char l) 
    { 
        type = t;
        lexeme = l; }

    Token(Type t, string l) {
        type = t;
        lexeme = l; 
        }

    string toString() const {
        switch (type) {
            case ID: return "ID";
            case NUM: return "NUM";
            case ASSIGN: return "=";
            case SEMI: return ";";
            case LP: return "(";
            case RP: return ")";
            case PLUS: return "+";
            case MIN: return "-";
            case MUL: return "*";
            case ERR: return "ERR";
            case PRINT: return "print";
            case END: return "epsilon";
            default: return "UNKNOWN";
        }
    }
};

class Scanner {
    private:
        string input;
        int first, current;
    public:
        explicit Scanner(const char *s){
            input = s;
            first = 0;
            current = 0;
        }
        Token *nextToken(){
            Token *token;

    while (input[current] == ' ')
        current++;

    if (input[current] == '\0')
        return new Token(Token::END);

    char c = input[current];
    first = current;
    if (isdigit(c)) {
        current++;
        while (isdigit(input[current])) {
            current++;
            }
        token = new Token(Token::NUM, input.substr(first, current-first));
    } else if (isalpha(c)) {
        current++;
        while (isalnum(input[current])){
            current++;
        }
        if (input.substr(first, current - first) == "print"){
            token = new Token(Token::PRINT, input.substr(first, current - first));
            }
        else{
            token = new Token(Token::ID, input.substr(first, current - first));
            }
    } else if (strchr("+-*()=;", c)) {
        switch (c) {
            case '=':
                token = new Token(Token::ASSIGN, c);
                break;
            case ';':
                token = new Token(Token::SEMI, c);
                break;
            case '+':
                token = new Token(Token::PLUS, c);
                break;
            case '-':
                token = new Token(Token::MIN, c);
                break;
            case '*':
                token = new Token(Token::MUL, c);
                break;
            case '(':
                token = new Token(Token::LP, c);
                break;
            case ')':
                token = new Token(Token::RP, c);
                break;
            default:
                token = new Token(Token::ERR, c);
                cout << "Caracter invalido: " << c << endl;
        }
        current++;
    } else {
        token = new Token(Token::ERR, c);
    }
    return token;
        }

        ~Scanner(){
            // delete[] input;
        }

        [[nodiscard]] string getInput() const {
            return input;}
};


class Parser {
private:
    Grammar *grammar;
    Scanner *scanner;
    Token *currentToken;
    unordered_map<string, unordered_map<string, int>> parseTable;

public:

    // Constructor
    Parser(Scanner *s, Grammar *g) {
        this->scanner = s;
        this->grammar = g;
        this->currentToken = nullptr;
        // Llamamos a la funcion para construir la tabla de parseo
        buildParseTable();
    }

    // Funcion que construye la tabla de parseo
    void buildParseTable() {
        // Inicializamos la tabla de parseo con valores -1
        for (const string &nonTerm : grammar->nonTerminals) {
            for (const string &term : grammar->terminals) {
                parseTable[nonTerm][term] = -1;
            }
        }

        // Para cada regla de produccion, procesamos la RHS
        for (const ProdRule &rule : grammar->rules) {
            vector<Symbol> symbvec;
            vector<Symbol> thisrhs = rule.rhs;
            bool alert = false;
            

            // Recorremos los simbolos de la RHS
            for (auto &sym : thisrhs) {
                if (sym.type == TERMINAL) {
                    symbvec.push_back(sym);
                    alert = true;
                    break;
                }

                SetSym symbolFirstSet = grammar->FIRST[sym.value];
                bool containsEps = false;
                for (const Symbol &s : symbolFirstSet) {
                    if (s.type == EPSILON) {
                        containsEps = true;
                        break;
                    }
                }
                
                if (!containsEps) {
                    symbvec.insert(symbvec.end(), symbolFirstSet.begin(), symbolFirstSet.end());
                    alert = true;
                    break;
                }
            }

            // Si no hemos terminado, agregamos FOLLOW
            if (!alert) {
                SetSym &follow = grammar->FOLLOW[rule.lhs];
                symbvec.insert(symbvec.end(), follow.begin(), follow.end());
            }

            // Insertamos las reglas en la tabla de parseo
            for (const Symbol &sym : symbvec) {
                bool foundInTable = parseTable[rule.lhs].find(sym.value) != parseTable[rule.lhs].end();
                bool isValidEntry = (foundInTable && parseTable[rule.lhs][sym.value] != -1);

                if (isValidEntry) {
                    cout << "Gramatica no es LL1!" << endl;
                    exit(1);
                }

                string terminalStr;
                if (sym.value == "$") {
                    terminalStr = "epsilon";
                } else {
                    terminalStr = sym.value;
                }

                int ruleIndex = 0;
                for (auto it = grammar->grammarRules().begin(); it != grammar->grammarRules().end(); ++it, ++ruleIndex) {
                    if (*it == rule) {
                        break;
                    }
                }

                parseTable[rule.lhs][terminalStr] = ruleIndex;
            }
        }

        // Revisamos si hay sincronización
        for (auto &parseTableRow : parseTable) {
            const SetSym &currentFollow = grammar->FOLLOW[parseTableRow.first];
            
            for (auto &parseTableCol : parseTableRow.second) {
                if (parseTableCol.second != -1) continue;

                bool isInFollowSet = false;
                
                // Recorremos el conjunto FOLLOW
                for (const Symbol &s : currentFollow) {
                    string symValue = (s.value == "$") ? "epsilon" : s.value;
                    if (symValue == parseTableCol.first) {
                        isInFollowSet = true;
                        break;
                    }
                }

                parseTableCol.second = isInFollowSet ? -2 : -1;
            }
        }

    }

    // Modulo para realizar el match
    void match(const string &top, size_t &start) {
        cout << "accion: match " << top << " : " << currentToken->lexeme << endl;
        start += currentToken->lexeme.size();  // Avanzamos en el input
        currentToken = scanner->nextToken();  // Obtenemos el siguiente token
    }

    // Funcion principal de parseo
    void parse() {
        currentToken = scanner->nextToken();  // Obtenemos el primer token
        stack<string> parseStack;
        vector<string> parseStackVec;

        // Inicializamos el stack con el simbolo inicial y $
        parseStack.emplace("$");
        parseStackVec.emplace_back("$");
        parseStack.push(grammar->grammarRules()[0].lhs);
        parseStackVec.push_back(grammar->grammarRules()[0].lhs);

        unordered_set<string> syncSet = {"$", "epsilon", ";", ")"};
        string inputString = this->scanner->getInput() + "$";

        string newString;
        
        for (char c : inputString) {
            if (!isspace(c)) {
                newString += c;
            }
        }
        inputString = newString;


        size_t start = 0;  // Marca el comienzo del input

        // Ciclo principal del parseo
        while (!parseStack.empty()) {
            string top = parseStack.top();
            string tokenStr = currentToken->toString();

            cout << "----------------------------------------\n";
            cout << "stack: ";
            for (string &s : parseStackVec) {
                cout << s << " ";
            }
            cout << endl;

            cout << "input: " << inputString.substr(start) << endl;

            if (top == "$") {
                // Si llegamos al final del stack y aun hay tokens, hay un error
                if (currentToken->type != Token::END) {
                    cerr << "Input no vacio, stack terminado" << endl;
                    exit(1);
                }
                cout << "Exitoso!" << endl;
                return;
            }

            // Si el tope del stack es terminal
            if (Grammar::isTerminal(top)) {
                if (top == tokenStr) {
                    match(top, start);  // Llamamos a la funcion modularizada
                    parseStack.pop();
                    parseStackVec.pop_back();
                } else {
                    handleError(top, tokenStr, parseStack, parseStackVec, syncSet, start);
                }
            } else {
                applyRule(top, tokenStr, parseStack, parseStackVec, start);
            }
        }
    }

    // Funcion para manejar errores de sintaxis
    void handleError(const string &top, const string &tokenStr, stack<string> &parseStack, vector<string> &parseStackVec, const unordered_set<string> &syncSet, size_t &start) {
        cout << "----------------------------------------\n";
        cout << "Error de sintaxis, se esperaba: " << top << ". Se obtuvo: " << tokenStr << endl;
        bool syncFound = syncSet.find(tokenStr) == syncSet.end();
        while (currentToken->type != Token::END && syncFound) {
            cout << "skip token. " << currentToken->toString() << endl;
            start += currentToken->lexeme.size();
            currentToken = scanner->nextToken();
            syncFound = syncSet.find(currentToken->toString()) == syncSet.end();
        }
        parseStack.pop();
        parseStackVec.pop_back();
    }

    // Funcion que aplica la regla correspondiente
    void applyRule(const string &top, const string &tokenStr, stack<string> &parseStack, vector<string> &parseStackVec, size_t &start) {
        int ruleIdx = parseTable[top][tokenStr];
        if (ruleIdx == -1 || ruleIdx == -2) {
            cout << "----------------------------------------\n";
            cout << "Error de sintaxis, token inesperado: " << tokenStr << endl;
            if (ruleIdx == -2) {
                cout << "Sacando... " << parseStack.top() << " del stack" << endl;
                parseStack.pop();
                parseStackVec.pop_back();
            } else {
                cout << "Skipping..." << endl;
                start += currentToken->lexeme.size();
                currentToken = scanner->nextToken();
            }
        } else {
            ProdRule rule = grammar->grammarRules()[ruleIdx];
            parseStack.pop();
            parseStackVec.pop_back();
            if (rule.rhs[0].value != "epsilon") {
                for (int i = (int) rule.rhs.size() - 1; i >= 0; i--) {
                    parseStack.push(rule.rhs[i].value);
                    parseStackVec.push_back(rule.rhs[i].value);
                }
            }
            cout << "accion: " << rule.lhs << " -> ";
            for (const Symbol &sym : rule.rhs) cout << sym.value << " ";
            cout << endl;
        }
    }
};
















