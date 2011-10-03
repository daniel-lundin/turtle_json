#include <iostream>
#include <fstream>
#include <vector>

#include "jsonnode.h"
#include "parser.h"

using namespace std;

int main(int argc, char** argv) 
{
    ifstream ifs("data.json");
    if(!ifs)
    {
        cout << "Bad file" << endl;
        return 1;
    }
    string data;
    ifs >> data;
    cout << data << endl;
    string error;
    Node* n = parse("{\"a\": [1.34,\"2\",\"3\",\"4\"]}", error);
    dump(n);
    return 0;
}
