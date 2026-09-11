#include <iostream>
#include <application.hpp>

int main(int argc, char *argv[])
{
    try
    {
        Application app{};
        if (app.init())
        {
            app.run();
        }
        app.shutdown();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "Exiting with Code 0" << std::endl;
    return EXIT_SUCCESS;
}