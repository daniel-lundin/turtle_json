#include "parser.h"
#include <sstream>
#include <cctype>
#include <iostream>
#include <stdexcept>

#include "jsonnode.h"

bool iswhitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r');
}

using namespace std;

bool tokenize(istream& json, vector<Token>& tokenlist, string& error_message)
{
    char c;
    while(json.good())
    {
        json.get(c);
        // Skip whitespace
        json >> ws;

        Token t;
        if(c == '[')
        {
            t.type = TOKEN_ARRAY_START;
            //cout << "TOKEN_ARRAY_START appended" << endl;
            tokenlist.push_back(t);
            continue;
        }
        if(c == ']')
        {
            t.type = TOKEN_ARRAY_END;
            //cout << "TOKEN_ARRAY_END appended" << endl;
            tokenlist.push_back(t);
            continue;
        }
        if(c == '{')
        {
            t.type = TOKEN_OBJECT_START;
            //cout << "TOKEN_OBJECT_START appended " << endl;
            tokenlist.push_back(t);
            continue;
        }
        if(c == '}')
        {
            t.type = TOKEN_OBJECT_END;
            //cout << "TOKEN_OBJECT_END appended" << endl;
            tokenlist.push_back(t);
            continue;
        }
        if(c == ':')
        {
            t.type = TOKEN_COLON;
            //cout << "TOKEN_COLON appended" << endl;
            tokenlist.push_back(t);
            continue;
        }
        if(c == ',')
        {
            t.type = TOKEN_COMMA;
            //cout << "TOKEN_COMMA appended" << endl;
            tokenlist.push_back(t);
            continue;
        }

        // Look for string
        if(c == '"')
        {
            // TODO: Look for escaped quotes
            ostringstream oss;
            do {
                json.get(c);
                if(c == '"')
                    break;
                if(!json.good())
                    return false;
                oss << c;
            } while(json.good());
            
            t.type = TOKEN_STRING;
            t.str = oss.str();
            //cout << "TOKEN_STRING appended: " << t.str << endl;
            tokenlist.push_back(t);
            continue;
        }

        // Look for number
        if(isdigit(c))
        {
            json.unget();
            int number;
            if((json >> number).good())
            {
                //cout << "Found number: " << number << endl;
                t.type = TOKEN_NUMBER;
                t.number = number;
                tokenlist.push_back(t);
                continue;
            }
            else
            {
                cout << "Fail reading to int" << endl;
                return false;
            }
        }
        //error_message = "Unknown token";
        //return false;
    }
    return true;
}

Node* parse(istream& json_stream, string& error_message)
{
    vector<Token> tokenlist;
    if(!tokenize(json_stream, tokenlist, error_message))
    {
        return false;
    }
    if(tokenlist.size() == 0)
        return false;
    for(int i=0;i<tokenlist.size();++i)
    {
        print_token(tokenlist[i]);
    }
    //return 0;

    Node* n = new Node();
    int idx = 0;
    return parse_structure(tokenlist, idx);
}

Node* parse_structure(const vector<Token>& tokenlist, int& index)
{
    //cout << "Parsing structure, index " << index << endl;
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
        //cout << "String " << tokenlist[index].str << endl;
        n->type = STRING;
        n->str = tokenlist[index].str;
        return n;
    }

    if(tokenlist[index].type == TOKEN_NUMBER)
    {
        //cout << "Number " << tokenlist[index].number << endl;
        n->type = INTEGER;
        n->integer = tokenlist[index].number;
        return n;
    }

    if(tokenlist[index].type == TOKEN_REAL)
    {
        //cout << "Real " << tokenlist[index].real << endl;
        n->type = REAL;
        n->real = tokenlist[index].real;
        return n;
    }

    //cout << tokenlist[index].type<<endl;
    throw std::runtime_error("Unknown type");
}

Node* parse_object(const vector<Token>& tokenlist, int& index)
{
    //cout << "Parsing object" << endl;
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
            throw std::runtime_error("object not starting with string key");
        string key = tokenlist[index].str;
        ++index;
        if(tokenlist[index].type != TOKEN_COLON)
            throw std::runtime_error("no colon after object key");
        objectNode->m_dict[key] = parse_structure(tokenlist, ++index);
        ++index;

        if(tokenlist[index].type == TOKEN_COMMA)
            ++index;
    }
    return objectNode;
}

Node* parse_array(const vector<Token>& tokenlist, int& index)
{
    //cout << "Parsing array" << endl;
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

void indent(int level)
{
    while(level--)
        cout << "   ";
}

void dump(Node* node, int level)
{
    if(node->type == OBJECT)
    {
        cout << endl;
        indent(level);
        cout << "{" << endl;
        map<string, Node*>::iterator it;
        bool first = true;
        for(it=node->m_dict.begin();it!=node->m_dict.end();++it)
        {
            if(!first)
            {
                cout << ", "<<endl;
            }
            indent(level+1);
            cout << "\"" << it->first << "\":";
            dump(it->second, level+2);
            first = false;
        }
        cout << endl;
        indent(level);
        cout << "}";
    }
    if(node->type == ARRAY)
    {
        indent(level);
        cout << "[" << endl;
        vector<Node*>::iterator it;

        for(int i=0;i<node->m_array.size();++i)
        {
            indent(level+1);
            dump(node->m_array[i], level+2);
            if(i != node->m_array.size() - 1)
                cout << ", ";
        }
        indent(level);
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
