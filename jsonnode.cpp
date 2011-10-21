#include "jsonnode.h"


JsonNode::~JsonNode()
{
    if(this->type == ARRAY)
    {
        while(!this->m_array.empty())
        {
            delete this->m_array.back();
            this->m_array.pop_back();
        }
    }
    if(this->type == ARRAY)
    {
        std::map<std::string, JsonNode*>::iterator it = this->m_dict.begin();
        std::map<std::string, JsonNode*>::iterator end = this->m_dict.end();
        for(;it != end;++it)
        {
            delete it->second;
        }
    }
}

JsonNode* JsonNode::string_node(const char* str)
{
    JsonNode* n = new JsonNode;
    n->type=STRING;
    n->str = str;
    return n;
}

JsonNode* JsonNode::real_node(double real)
{
    JsonNode* n = new JsonNode;
    n->type = REAL;
    n->real = real;
    return n;
}

JsonNode* JsonNode::integer_node(int integer)
{
    JsonNode* n = new JsonNode;
    n->type = INTEGER;
    n->integer = integer;
    return n;
}

JsonNode* JsonNode::boolean_node(bool value)
{
    JsonNode* n = new JsonNode;
    n->type = BOOLEAN;
    n->boolean = value;
    return n;
}

JsonNode* JsonNode::null_node()
{
    JsonNode* n = new JsonNode;
    n->type = NIL;
    return n;
}

JsonNode* JsonNode::array_node()
{
    JsonNode* n = new JsonNode;
    n->type = ARRAY;
    return n;
}

JsonNode* JsonNode::object_node()
{
    JsonNode* n = new JsonNode;
    n->type = OBJECT;
    return n;
}




