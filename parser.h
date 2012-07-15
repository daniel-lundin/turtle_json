#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

namespace turtle_json {
// Forward declarations
class JsonNode;

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
struct Token {
    TokenType type;
    std::string str;
    int number;
    double real;
    bool boolean;
    streampos pos;
};



JsonNode* parse(istream& json_stream);
void dump(JsonNode*, ostream& os, int level=0);


} // END NAMESPACE turtle_json
#endif
