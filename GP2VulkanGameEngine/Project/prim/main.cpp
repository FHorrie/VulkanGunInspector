#include "prim/App.h"

#include <cstdlib>
#include <stdexcept>
#include <iostream>

int main() {
    FH::FirstApp MyApp{};

    try 
    {
        MyApp.Run();
    } 
    catch (const std::exception& exc) 
    {
        std::cerr << exc.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}