#include <iostream>
#include "../ObjReader.h"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        cerr << "Missing obj file!" << endl;
        return -1;
    }

    ObjReader reader(argv[1]);
    reader.parse();

    return 0;
}
