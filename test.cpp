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
    ifs.seekg(0, ios::end);
    //cout << "length: " << ifs.tellg() << endl;
    ifs.seekg(0, ios::beg);
    string error;
    //parse(ifs, error);
    dump(parse(ifs, error));

    return 0;
}
