#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdexcept>

#include "jsonnode.h"
#include "parser.h"

using namespace std;
using namespace turtle_json;

void print_from_stream(ifstream& is, streampos at)
{
    is.seekg(at-streampos(20), ios::beg).good();
    char data[40];
    is.read(data, 39).good(); 
    data[39] = '\0';
    cout << data << endl;
    cout << "                  ^" << endl;
}
void read_and_dump() 
{
    cout << "--- Parsing and dumping jsonfiles/ex1.json ---" << endl;
    ifstream ifs("jsonfiles/ex1.json");
    if(!ifs)
    {
        cout << "Bad file" << endl;
        return;
    }
     

    JsonNode* root = 0;
    try {
        root = parse(ifs);
    }
    catch(tokenize_exception& e)
    {
        cout << "Tokenize error at " << e.pos << ": " << e.what() << endl;
        ifs.clear(); // clear error bits
        print_from_stream(ifs, e.pos);
        cout << e.what() << endl;
        return;
    }

    if(root == 0)
    {
        cout << "Root is null" << endl;
        return;
    }

    try {
        dump(root, cout);
    }
    catch(parse_exception& e)
    {
        cout << "Parse error: " << e.what() << endl;
        ifs.clear(); // clear error bits
        print_from_stream(ifs, e.pos);
        cout << e.what() << endl;
        return;
    }
    cout << endl << "--- Done ---" << endl;
}

void serialize_structure()
{
    cout << "--- Creating json-structure and dumping to testout.json ---" << endl;
    JsonNode* root = JsonNode::array_node();
    
    root->m_array.push_back(JsonNode::string_node("First"));
    root->m_array.push_back(JsonNode::string_node("Second"));
    root->m_array.push_back(JsonNode::string_node("Third"));
    
    JsonNode* obj = JsonNode::object_node();
    obj->m_dict["One"] = JsonNode::integer_node(1);
    obj->m_dict["Two"] = JsonNode::integer_node(2);
    obj->m_dict["Three"] = JsonNode::integer_node(3);

    root->m_array.push_back(obj);

    ofstream ofs("testout.json");
    if(!ofs.good())
    {
        cout << "Unable to open out.json" << endl;
        return;
    }
    dump(root, ofs);
    cout << endl << "--- Done ---" << endl;
}

int main(int argc, char** argv) 
{
    read_and_dump();
    serialize_structure();
    return 0;
}

