#ifndef JSON_NODE_H
#define JSON_NODE_H

#include "macros.h"

#include <string>
#include <map>
#include <vector>

enum NodeType { STRING, REAL, INTEGER, ARRAY, OBJECT };

class Node {
public:
    ~Node();

    Node* parse(const std::string& jsonstring);

    std::map<std::string, Node*> m_dict;
    std::vector<Node*> m_array;

    std::string str;
    int integer;
    float real;

    NodeType type;
};

#endif
