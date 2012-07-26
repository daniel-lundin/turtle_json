#include "parser.h"
#include <sstream>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cmath>

#include "jsonnode.h"

namespace turtle_json {

static JsonNode* parse_structure(const vector<Token>&, int& index);
static JsonNode* parse_array(const vector<Token>&, int& index);
static JsonNode* parse_object(const vector<Token>&, int& index);

using namespace std;

int int_pow(int a, int b) {
    if(b == 0) return 1;
    if(b == 1) return a;
    return a * int_pow(a, b-1);
}

static
void tokenize(istream& json, vector<Token>& tokenlist) {
    char c;
    while(json.good()) {
        // Skip whitespace
        json >> ws;
        json.get(c);
        if(!json.good()) {
            break;
        }

        Token t;
        t.pos = json.tellg();
        if(c == '[') {
            t.type = TOKEN_ARRAY_START;
            tokenlist.push_back(t);
        }
        else if(c == ']') {
            t.type = TOKEN_ARRAY_END;
            tokenlist.push_back(t);
        }
        else if(c == '{') {
            t.type = TOKEN_OBJECT_START;
            tokenlist.push_back(t);
        }
        else if(c == '}') {
            t.type = TOKEN_OBJECT_END;
            tokenlist.push_back(t);
        }
        else if(c == ':') {
            t.type = TOKEN_COLON;
            tokenlist.push_back(t);
        }
        else if(c == ',') {
            t.type = TOKEN_COMMA;
            tokenlist.push_back(t);
        }
        // Look for boolean values
        else if(c == 't') {
            streampos old_pos = json.tellg();
            char a;
            bool match = json.get(a).good() && a == 'r';
            match |= json.get(a).good() && a == 'u';
            match |= json.get(a).good() && a == 'e';
            if(match) {
                t.type = TOKEN_BOOLEAN;
                t.boolean = true;
                tokenlist.push_back(t);
            }
            else {
                json.seekg(old_pos);
            }
        }
        else if(c == 'f') {
            int old_pos = json.tellg();
            char a;
            bool match = json.get(a).good() && a == 'a';
            match |= json.get(a).good() && a == 'l';
            match |= json.get(a).good() && a == 's';
            match |= json.get(a).good() && a == 'e';
            if(match) {
                t.type = TOKEN_BOOLEAN;
                t.boolean = false;
                tokenlist.push_back(t);
            }
            else {
                json.seekg(old_pos);
            }
        }
        else if(c == 'n') {
            int old_pos = json.tellg();
            char a;
            bool match = json.get(a).good() && a == 'u';
            match |= json.get(a).good() && a == 'l';
            match |= json.get(a).good() && a == 'l';
            if(match) {
                t.type = TOKEN_NULL;
                tokenlist.push_back(t);
            }
            else {
                throw tokenize_exception("Unknown token while parsing null", t.pos);
            }

        }

        // Look for string
        else if(c == '"') {
            ostringstream oss;
            char prev_c='x';
            do {
                json.get(c);
                if(!json.good()) {
                    throw tokenize_exception("error parsing string", t.pos);
                }
                if(c == '"' && prev_c != '\\')
                    break;
                oss << c;
                prev_c = c;
            } while(json.good());
            
            t.type = TOKEN_STRING;
            t.str = oss.str();
            tokenlist.push_back(t);
        }

        // Look for number
        else if(isdigit(c) || c == '-') {
            bool neggo = (c == '-');
            if(!neggo) {
                json.unget();
            }
            int int_part;
            if(!(json >> int_part).good()) {
                throw tokenize_exception("Error reading integer part", t.pos);
            }

            json.get(c);
            if(c != '.') {
                json.unget(); // Put back whatever it was
                t.type = TOKEN_NUMBER;
                t.number = (!neggo ? int_part : -int_part);
                tokenlist.push_back(t);
                continue;
            }
            double frac_part;
            streampos frac_start_pos = json.tellg();
            if(!(json >> frac_part).good()) {
                throw tokenize_exception("Error reading decimal part", t.pos);
            }

            streampos frac_end_pos = json.tellg();
            int frac_part_length = frac_end_pos - frac_start_pos;
            json.get(c);
            if(c != 'e' && c != 'E') {
                json.unget();
                t.type = TOKEN_REAL;
                // round down
                t.real = (double) int_part + (frac_part / (double) int_pow(10, frac_part_length));
                t.real *= (neggo ? -1.0 : 1.0);
                tokenlist.push_back(t);
                continue;
            }
            throw runtime_error("Scientific notation not yet implemented");
        }
        else {
            char err_msg[128];
            sprintf(err_msg, "Unknown token %c", c);
            throw tokenize_exception(err_msg, t.pos);
        }
    }
}

JsonNode* parse(istream& json_stream) {
    vector<Token> tokenlist;
    tokenize(json_stream, tokenlist);

    if(tokenlist.size() == 0)
        return 0;

    int idx = 0;
    return parse_structure(tokenlist, idx);
}

static
JsonNode* parse_structure(const vector<Token>& tokenlist, int& index) {
    if(tokenlist[index].type == TOKEN_ARRAY_START) {
        return parse_array(tokenlist, index);
    }

    if(tokenlist[index].type == TOKEN_OBJECT_START) {
        return parse_object(tokenlist, index);
    }

    if(tokenlist[index].type == TOKEN_STRING) {
        return JsonNode::string_node(tokenlist[index].str.c_str());
    }

    if(tokenlist[index].type == TOKEN_NUMBER) {
        return JsonNode::integer_node(tokenlist[index].number);
    }

    if(tokenlist[index].type == TOKEN_REAL) {
        return JsonNode::real_node(tokenlist[index].real);
    }

    if(tokenlist[index].type == TOKEN_BOOLEAN) {
        return JsonNode::boolean_node(tokenlist[index].boolean);
    }

    if(tokenlist[index].type == TOKEN_NULL) {
        return JsonNode::null_node();
    }

    throw std::runtime_error("Unknown type");
}

JsonNode* parse_object(const vector<Token>& tokenlist, int& index) {
    JsonNode* objectNode = JsonNode::object_node();
    if(tokenlist[index].type != TOKEN_OBJECT_START) {
        throw std::runtime_error("No { a start of dictionary");
    }
    ++index;

    while(tokenlist[index].type != TOKEN_OBJECT_END) {
        if(tokenlist[index].type != TOKEN_STRING)
            throw parse_exception("object not starting with string key", tokenlist[index].pos);
        string key = tokenlist[index].str;
        ++index;
        if(tokenlist[index].type != TOKEN_COLON)
            throw parse_exception("no colon after object key", tokenlist[index].pos);
        objectNode->m_dict[key] = parse_structure(tokenlist, ++index);
        ++index;

        if(tokenlist[index].type == TOKEN_COMMA)
            ++index;
    }
    return objectNode;
}

JsonNode* parse_array(const vector<Token>& tokenlist, int& index) {
    JsonNode* arrayNode = JsonNode::array_node();
    if(tokenlist[index].type != TOKEN_ARRAY_START) {
        throw std::runtime_error("no [ at beginning of array");
    }
    ++index;

    while(tokenlist[index].type != TOKEN_ARRAY_END) {
        arrayNode->m_array.push_back(parse_structure(tokenlist, index));
        ++index;
        if(tokenlist[index].type == TOKEN_COMMA)
            ++index;
    }
    return arrayNode;
}

void indent(ostream& os, int level) {
    while(level > 0) {
        os << "    ";
        --level;
    }
}

void dump(JsonNode* node, ostream& os, int level) {
    if(node->type == OBJECT) {
        os << "{" << endl;
        map<string, JsonNode*>::iterator it;
        bool first = true;
        for(it=node->m_dict.begin();it!=node->m_dict.end();++it) {
            if(!first) {
                os << ", "<<endl;
            }
            indent(os, level+1);
            os << "\"" << it->first << "\": ";
            dump(it->second, os, level+1);
            first = false;
        }
        os << endl;
        indent(os, level);
        os << "}";
    }
    if(node->type == ARRAY) {
        indent(os, level);
        os << "[" << endl;
        vector<JsonNode*>::iterator it;

        for(int i=0;i<node->m_array.size();++i) {
            indent(os, level+1);
            dump(node->m_array[i], os, level+1);
            if(i != node->m_array.size() - 1)
                os << ", " << endl;
        }
        os << endl;
        indent(os, level);
        os << "]" << endl;
    }
    if(node->type == STRING) {
        os << "\"" << node->str << "\"";
    }
    if(node->type == INTEGER) {
        os << node->integer;
    }
    if(node->type == REAL) {
        os << node->real;
    }
    if(node->type == BOOLEAN) {
        os << (node->boolean ? "true" : "false");
    }
    if(node->type == NIL) {
        os << "null";
    }
}

} // END NAMESPACE turtle_json
