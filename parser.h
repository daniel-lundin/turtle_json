#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// Forward declarations
class Node;

using std::string;
using std::vector;
using std::istream;
using std::ostream;
using std::runtime_error;
using std::streampos;

class tokenize_exception : public runtime_error {
public:
    tokenize_exception(const char* what, streampos pos) : runtime_error(what), pos(pos) {}
    streampos pos;
};

class parse_exception : public runtime_error {
public:
    parse_exception(const char* what, streampos pos) : runtime_error(what), pos(pos) {}
    streampos pos;
};

enum TokenType { TOKEN_OBJECT_START, 
                 TOKEN_OBJECT_END, 
                 TOKEN_ARRAY_START, 
                 TOKEN_ARRAY_END, 
                 TOKEN_STRING, 
                 TOKEN_NUMBER, 
                 TOKEN_REAL, 
                 TOKEN_BOOLEAN, 
                 TOKEN_NULL, 
                 TOKEN_COMMA, 
                 TOKEN_COLON };
struct Token
{
    TokenType type;
    std::string str;
    int number;
    double real;
    bool boolean;
    streampos pos;
};



Node* parse(istream& json_stream);
void dump(Node*, ostream& os, int level=0);
void print_token(const Token&);


#endif
