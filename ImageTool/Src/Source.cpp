#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;


std::filesystem::path TakeInputFile();

int main(int argc, char* argv[])
{
    string input = TakeInputFile().string();
    int width = 0;
    int height = 0;
    int bpp = 0;

    cout << "load: " << input << endl;
    unsigned char* imagePtr = stbi_load(input.c_str(), &width, &height, &bpp, STBI_rgb_alpha);


    std::cout << "exit on input\n";
    auto c = std::getchar();
    return 0;
}

std::filesystem::path TakeInputFile()
{
    cout << "enter input file" << endl;
    string input;
    while (true)
    {
        cin >> input;
        if (input == "cd")
        {
            cin >> input;
            if (filesystem::is_directory(input))
            {
                filesystem::current_path(input);
            }
            else
            {
                cout << input << " is not a directory" << endl;
            }
        }
        else if (input == "ls")
        {
            for (const auto& entry : filesystem::directory_iterator(filesystem::current_path()))
            {
                cout << entry.path().filename().string() << endl;
            }
        }
        else
        {
            if(!filesystem::exists(input))
            {
                cout << "file does not exist, try again" << endl;
            }
            else
            {
                return input;
            }
        }
    }
}
