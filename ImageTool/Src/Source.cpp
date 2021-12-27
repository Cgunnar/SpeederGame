#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <filesystem>

#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#pragma warning(pop)

using namespace std;


std::filesystem::path TakeInputFile();

enum class Channel
{
	r = 0,
	g = 1,
	b = 2,
	a = 3,
	none = 4
};

string ChannelToString(Channel c);

string RemoveIllegalChars(const string& str);

Channel SelectChannel(Channel exlude = Channel::none)
{
	string inputSelectedChannel = "";
	while (inputSelectedChannel.empty())
	{
		cout << endl << "select channel r, g, b, or a" << endl;
		cin >> inputSelectedChannel;
		Channel c = Channel::none;
		if (inputSelectedChannel == "r") c = Channel::r;
		if (inputSelectedChannel == "g") c = Channel::g;
		if (inputSelectedChannel == "b") c = Channel::b;
		if (inputSelectedChannel == "a") c = Channel::a;

		if (c != exlude && c != Channel::none)
			return c;
		else
			cout << "can not select that channal" << endl;
	}

}

int main(int argc, char* argv[])
{

	//output
	string outOutFileName = "";
	cin >> outOutFileName;



	//input file 1
	cout << "Enter file1 to read from" << endl;

	string input1 = TakeInputFile().string();
	int input1Width = 0;
	int input1Height = 0;
	int input1Channels = 0;

	cout << "load: " << input1 << endl;
	unsigned char* input1ImagePtr = stbi_load(input1.c_str(), &input1Width, &input1Height, &input1Channels, STBI_rgb_alpha);
	assert(input1ImagePtr);
	cout << "width:\t\t" << input1Width << endl;
	cout << "height:\t\t" << input1Height << endl;
	cout << "channels:\t" << input1Channels << endl;

	Channel input1SelectedChannel = SelectChannel(input1Channels < STBI_rgb_alpha ? Channel::a : Channel::none);

	//input file 2
	cout << "Enter file2 to read from" << endl;

	string input2 = TakeInputFile().string();
	int input2Width = 0;
	int input2Height = 0;
	int input2Channels = 0;

	cout << "load: " << input2 << endl;
	unsigned char* input2ImagePtr = stbi_load(input2.c_str(), &input2Width, &input2Height, &input2Channels, STBI_rgb_alpha);
	assert(input2ImagePtr);
	cout << "width:\t\t" << input2Width << endl;
	cout << "height:\t\t" << input2Height << endl;
	cout << "channels:\t" << input2Channels << endl;

	if (input1Height != input2Height || input1Width != input1Width)
	{
		cout << "texture dimensions does not match" << endl;
		stbi_image_free(input1ImagePtr);
		stbi_image_free(input2ImagePtr);
		return 0;
	}

	Channel input2SelectedChannel = SelectChannel(input2Channels < STBI_rgb_alpha ? Channel::a : Channel::none);

	cout << "which channel should " << ChannelToString(input1SelectedChannel) <<" from file 1 be copied to ? " << endl;
	Channel out1SelectedChannel = SelectChannel();
	cout << "which channel should " << ChannelToString(input2SelectedChannel) <<" from file 2 be copied to ? " << endl;
	Channel out2SelectedChannel = SelectChannel(out1SelectedChannel);

	



	std::cout << "exit on input\n";
	auto c = std::getchar();
	return 0;
}

std::string RemoveIllegalChars(const std::string& str)
{
	std::string illegalChars = "\\/:?\"<>|";
	std::string legalString = "";
	for (int i = 0; i < str.length(); i++)
	{
		if (illegalChars.find(str[i]) == std::string::npos) {
			legalString += str[i];
		}
	}
	return legalString;
}

string ChannelToString(Channel c)
{
	string s;
	switch (c)
	{
	case Channel::r: return "r";
	case Channel::g: return "g";
	case Channel::b: return "b";
	case Channel::a: return "a";
	case Channel::none: return "";
	}
}

std::filesystem::path TakeInputFile()
{
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
			if (!filesystem::exists(input))
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
