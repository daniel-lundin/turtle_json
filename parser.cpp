#include "parser.h"
#include <sstream>
#include <cctype>
#include <iostream>
#include <stdexcept>

#include "jsonnode.h"

static Node* parse_structure(const vector<Token>&, int& index);
static Node* parse_array(const vector<Token>&, int& index);
static Node* parse_object(const vector<Token>&, int& index);

using namespace std;

static
void tokenize(istream& json, vector<Token>& tokenlist)
{
    char c;
    while(json.good())
    {
        json >> ws;
        json.get(c);
        if(!json.good())
        {
            break;
        }
        // Skip whitespace

        Token t;
        //cout << json.tellg();
        t.pos = json.tellg();
        if(c == '[')
        {
            t.type = TOKEN_ARRAY_START;
            tokenlist.push_back(t);
        }
        else if(c == ']')
        {
            t.type = TOKEN_ARRAY_END;
            tokenlist.push_back(t);
        }
        else if(c == '{')
        {
            t.type = TOKEN_OBJECT_START;
            tokenlist.push_back(t);
        }
        else if(c == '}')
        {
            t.type = TOKEN_OBJECT_END;
            tokenlist.push_back(t);
        }
        else if(c == ':')
        {
            t.type = TOKEN_COLON;
            tokenlist.push_back(t);
        }
        else if(c == ',')
        {
            t.type = TOKEN_COMMA;
            tokenlist.push_back(t);
        }
        // Look for boolean values
        else if(c == 't')
        {
            streampos old_pos = json.tellg();
            char a;
            bool match = json.get(a).good() && a == 'r';
            match |= json.get(a).good() && a == 'u';
            match |= json.get(a).good() && a == 'e';
            if(match)
            {
                t.type = TOKEN_BOOLEAN;
                t.boolean = true;
                tokenlist.push_back(t);
            }
            else
            {
                json.seekg(old_pos);
            }
        }
        else if(c == 'f')
        {
            int old_pos = json.tellg();
            char a;
            bool match = json.get(a).good() && a == 'a';
            match |= json.get(a).good() && a == 'l';
            match |= json.get(a).good() && a == 's';
            match |= json.get(a).good() && a == 'e';
            if(match)
            {
                t.type = TOKEN_BOOLEAN;
                t.boolean = false;
                tokenlist.push_back(t);
            }
            else
            {
                json.seekg(old_pos);
            }
        }
        else if(c == 'n')
        {
            int old_pos = json.tellg();
            char a;
            bool match = json.get(a).good() && a == 'u';
            match |= json.get(a).good() && a == 'l';
            match |= json.get(a).good() && a == 'l';
            if(match)
            {
                t.type = TOKEN_NULL;
                tokenlist.push_back(t);
            }
            else
            {
                throw tokenize_exception("Unknown token", t.pos);
            }

        }

        // Look for string
        else if(c == '"')
        {
            // TODO: Look for escaped quotes
            ostringstream oss;
            do {
                json.get(c);
                if(!json.good())
                {
                    throw tokenize_exception("error parsing string", t.pos);
                }
                if(c == '"')
                    break;
                oss << c;
            } while(json.good());
            
            t.type = TOKEN_STRING;
            t.str = oss.str();
            tokenlist.push_back(t);
        }

        // Look for number
        else if(isdigit(c))
        {
            json.unget();
            int number;
            if((json >> number).good())
            {
                t.type = TOKEN_NUMBER;
                t.number = number;
                tokenlist.push_back(t);
            }
            else
            {
                throw tokenize_exception("Error converting number", t.pos);
            }
        }
        else {
            throw tokenize_exception("Unknown token", t.pos);
        }
    }
}

Node* parse(istream& json_stream)
{
    vector<Token> tokenlist;
    tokenize(json_stream, tokenlist);

    if(tokenlist.size() == 0)
        return 0;

    Node* n = new Node();
    int idx = 0;
    return parse_structure(tokenlist, idx);
}

static
Node* parse_structure(const vector<Token>& tokenlist, int& index)
{
    if(tokenlist[index].type == TOKEN_ARRAY_START)
    {
        return parse_array(tokenlist, index);
    }

    if(tokenlist[index].type == TOKEN_OBJECT_START)
    {
        return parse_object(tokenlist, index);
    }

    Node* n = new Node();
    if(tokenlist[index].type == TOKEN_STRING)
    {
        n->type = STRING;
        n->str = tokenlist[index].str;
        return n;
    }

    if(tokenlist[index].type == TOKEN_NUMBER)
    {
        n->type = INTEGER;
        n->integer = tokenlist[index].number;
        return n;
    }

    if(tokenlist[index].type == TOKEN_REAL)
    {
        n->type = REAL;
        n->real = tokenlist[index].real;
        return n;
    }
    if(tokenlist[index].type == TOKEN_BOOLEAN)
    {
        n->type = BOOLEAN;
        n->boolean = tokenlist[index].boolean;
        return n;
    }
    if(tokenlist[index].type == TOKEN_NULL)
    {
        n->type = NIL;
        return n;
    }

    throw std::runtime_error("Unknown type");
}

Node* parse_object(const vector<Token>& tokenlist, int& index)
{
    Node* objectNode = new Node();
    objectNode->type = OBJECT;
    if(tokenlist[index].type != TOKEN_OBJECT_START)
    {
        throw std::runtime_error("No { a start of dictionary");
    }
    ++index;

    while(tokenlist[index].type != TOKEN_OBJECT_END)
    {
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

Node* parse_array(const vector<Token>& tokenlist, int& index)
{
    Node* arrayNode = new Node();
    arrayNode->type = ARRAY;
    if(tokenlist[index].type != TOKEN_ARRAY_START)
    {
        throw std::runtime_error("no [ at beginning of array");
    }
    ++index;

    while(tokenlist[index].type != TOKEN_ARRAY_END)
    {
        arrayNode->m_array.push_back(parse_structure(tokenlist, index));
        ++index;
        if(tokenlist[index].type == TOKEN_COMMA)
            ++index;
    }
    return arrayNode;
}

void indent(ostream& os, int level)
{
    while(level > 0)
    {
        os << "    ";
        --level;
    }
}

void dump(Node* node, ostream& os, int level)
{
    if(node->type == OBJECT)
    {
        //cout << endl;
        //indent(os, level);
        cout << "{" << endl;
        map<string, Node*>::iterator it;
        bool first = true;
        for(it=node->m_dict.begin();it!=node->m_dict.end();++it)
        {
            if(!first)
            {
                cout << ", "<<endl;
            }
            indent(os, level+1);
            cout << "\"" << it->first << "\": ";
            dump(it->second, os, level+1);
            first = false;
        }
        cout << endl;
        indent(os, level);
        cout << "}";
    }
    if(node->type == ARRAY)
    {
        indent(os, level);
        cout << "[" << endl;
        vector<Node*>::iterator it;

        for(int i=0;i<node->m_array.size();++i)
        {
            indent(os, level+1);
            dump(node->m_array[i], os, level+1);
            if(i != node->m_array.size() - 1)
                cout << ", " << endl;
        }
        cout << endl;
        indent(os, level);
        cout << "]" << endl;
    }
    if(node->type == STRING)
    {
        cout << "\"" << node->str << "\"";
    }
    if(node->type == INTEGER)
    {
        cout << node->integer;
    }
    if(node->type == REAL)
    {
        cout << node->real;
    }
    if(node->type == BOOLEAN)
    {
        cout << (node->boolean ? "true" : "false");
    }
    if(node->type == NIL)
    {
        cout << "null";
    }
}

void print_token(const Token& t)
{
    if(t.type == TOKEN_OBJECT_START ) cout << "{" ;
    if(t.type == TOKEN_OBJECT_END ) cout << "}" ;
    if(t.type == TOKEN_ARRAY_START ) cout << "[";
    if(t.type == TOKEN_ARRAY_END ) cout << "]" ;
    if(t.type == TOKEN_STRING ) cout << "\"" << t.str << "\"";
    if(t.type == TOKEN_NUMBER ) cout << t.number;
    if(t.type == TOKEN_REAL ) cout << "TOKEN_REAL";
    if(t.type == TOKEN_COMMA ) cout << ", ";
    if(t.type == TOKEN_COLON) cout << ": ";
     return;
    if(t.type == TOKEN_OBJECT_START ) cout << "TOKEN_OBJECT_START" << endl;
    if(t.type == TOKEN_OBJECT_END ) cout << "TOKEN_OBJECT_END" << endl;
    if(t.type == TOKEN_ARRAY_START ) cout << "TOKEN_ARRAY_START" << endl;
    if(t.type == TOKEN_ARRAY_END ) cout << "TOKEN_ARRAY_END" << endl;
    if(t.type == TOKEN_STRING ) cout << "TOKEN_STRING" << endl;
    if(t.type == TOKEN_NUMBER ) cout << "TOKEN_NUMBER" << endl;
    if(t.type == TOKEN_REAL ) cout << "TOKEN_REAL" << endl;
    if(t.type == TOKEN_COMMA ) cout << "TOKEN_COMMA" << endl;
    if(t.type == TOKEN_COLON) cout << "TOKEN_COLON" << endl;
}
