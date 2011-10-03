#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <vector>

// Forward declarations
class Node;

using std::string;
using std::vector;
using std::istream;

enum TokenType { TOKEN_OBJECT_START, 
                 TOKEN_OBJECT_END, 
                 TOKEN_ARRAY_START, 
                 TOKEN_ARRAY_END, 
                 TOKEN_STRING, 
                 TOKEN_NUMBER, 
                 TOKEN_REAL, 
                 TOKEN_COMMA, 
                 TOKEN_COLON };
struct Token
{
    TokenType type;
    std::string str;
    int number;
    double real;
};

static Node* parse_structure(const vector<Token>&, int& index);
static Node* parse_array(const vector<Token>&, int& index);
static Node* parse_object(const vector<Token>&, int& index);


static bool tokenize(istream& json_stream, vector<Token>& tokenlist, string& error_message);
Node* parse(istream& json_stream, string& error_message);
void dump(Node*, int level=0);
void print_token(const Token&);


#endif
