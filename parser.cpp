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

bool tokenize(const string& json, vector<Token>& tokenlist, string& error_message)
{
    int idx = 0;
    while(json[idx])
    {
        // Skip whitespace
        while(iswhitespace(json[idx]))
        {
            //cout << "skipping whitespace " << json[idx] << endl;
            ++idx;
        }

        Token t;
        if(json[idx] == '[')
        {
            t.type = TOKEN_ARRAY_START;
            //cout << "TOKEN_ARRAY_START appended" << endl;
            tokenlist.push_back(t);
            ++idx;
            continue;
        }
        if(json[idx] == ']')
        {
            t.type = TOKEN_ARRAY_END;
            //cout << "TOKEN_ARRAY_END appended" << endl;
            tokenlist.push_back(t);
            ++idx;
            continue;
        }
        if(json[idx] == '{')
        {
            t.type = TOKEN_OBJECT_START;
            //cout << "TOKEN_OBJECT_START appended " << endl;
            tokenlist.push_back(t);
            ++idx;
            continue;
        }
        if(json[idx] == '}')
        {
            t.type = TOKEN_OBJECT_END;
            //cout << "TOKEN_OBJECT_END appended" << endl;
            tokenlist.push_back(t);
            ++idx;
            continue;
        }
        if(json[idx] == ':')
        {
            t.type = TOKEN_COLON;
            //cout << "TOKEN_COLON appended" << endl;
            tokenlist.push_back(t);
            ++idx;
            continue;
        }
        if(json[idx] == ',')
        {
            t.type = TOKEN_COMMA;
            //cout << "TOKEN_COMMA appended" << endl;
            tokenlist.push_back(t);
            ++idx;
            continue;
        }

        // Look for string
        if(json[idx] == '"')
        {
            // TODO: Look for escaped quotes
            int end_string = json.find('"', idx+1);
            if(end_string == -1)
            {
                error_message = "No end quotation mark found";
                return false;
            }
            t.type = TOKEN_STRING;
            t.str = json.substr(idx+1, end_string-idx-1);
            //cout << "TOKEN_STRING appended: " << t.str << endl;
            tokenlist.push_back(t);
            idx = end_string + 1;
            continue;
        }

        // Look for number
        if(isdigit(json[idx]))
        {
            int num_end_idx = idx+1;
            bool is_double = false;
            while(isdigit(json[num_end_idx]) || json[num_end_idx] == '.')
            {
                if(json[num_end_idx] == '.')
                    is_double = true;
                ++num_end_idx;
            }
            if(is_double)
            {
                //cout << json.substr(idx, num_end_idx-idx) << endl;
                t.type = TOKEN_REAL;
                t.real = atof(json.substr(idx, num_end_idx-idx).c_str());
            }
            else
            {
                //cout << json.substr(idx, num_end_idx-idx) << endl;
                t.type = TOKEN_NUMBER;
                t.real = atoi(json.substr(idx, num_end_idx-idx).c_str());
            }
            //cout << "NUMBER appended: " << endl;
            tokenlist.push_back(t);
            idx = num_end_idx+1;
            continue;
            
            
        }
        error_message = "Unknown token" + json[idx];
        return false;
    }
    return true;
}

Node* parse(const string& json, string& error_message)
{
    vector<Token> tokenlist;
    if(!tokenize(json, tokenlist, error_message))
    {
        return false;
    }
    if(tokenlist.size() == 0)
        return false;

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
        indent(level);
        cout << "{" << endl;
        map<string, Node*>::iterator it;
        bool first = true;
        for(it=node->m_dict.begin();it!=node->m_dict.end();++it)
        {
            if(!first)
            {
                cout << ", ";
            }
            indent(level+1);
            cout << "\"" << it->first << "\":" << endl;
            indent(level);
            dump(it->second, level+2);
            first = false;
        }
        indent(level);
        cout << "}" << endl;
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
            cout << endl;
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
