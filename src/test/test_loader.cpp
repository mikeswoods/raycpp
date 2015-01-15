#include <iostream>
#include <easylogging++.h>
#include "../ModelImport.h"

using namespace std;

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[])
{
    if (argc != 2) {
        cerr << "Missing obj file!" << endl;
        return -1;
    }

    import(argv[1]);

    return 0;
}
