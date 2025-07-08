#include <iostream>
#include "ServerMultiClass.h"

int main()
{
    ServerMulti::startServer();
    while (true)
    {
        Sleep(10000);
    }
    return 0;
}