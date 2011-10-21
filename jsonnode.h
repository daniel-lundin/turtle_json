#ifndef JSON_NODE_H
#define JSON_NODE_H

#include "macros.h"

#include <string>
#include <map>
#include <vector>

enum NodeType { STRING, REAL, INTEGER, BOOLEAN, NIL, ARRAY, OBJECT };

struct Node {

    std::map<std::string, Node*> m_dict;
    std::vector<Node*> m_array;

    std::string str;
    int integer;
    float real;
    bool boolean;

    NodeType type;
};

#endif
