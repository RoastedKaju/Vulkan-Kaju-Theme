#include <utils.hpp>
#include <fstream>
#include <sstream>

std::string readTextFile(const std::string &filePath)
{
    std::ifstream inFile(filePath);
    if (inFile.is_open())
    {
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        const std::string output = buffer.str();
        inFile.close();
        return output;
    }
    return std::string();
}
