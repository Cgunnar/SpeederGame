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

enum class Channel
{
	r = 0,
	g = 1,
	b = 2,
	a = 3,
	none = 4
};

struct Image
{
	int width = 0;
	int height = 0;
	int channels = 0;
	Channel selectedInputChannel = Channel::none;
	Channel selectedOutPutChannel = Channel::none;
	unsigned char* data = nullptr;
};

filesystem::path TakeInputFile();
string ChannelToString(Channel c);
Channel SelectChannel();
bool AddImage(const Image& baseImage, vector<Image>& images);
bool WriteValueToChannel(const Image& baseImage);

int main(int argc, char* argv[])
{
	std::vector<Image> inputImages;
	cout << "Enter file, this will be used as a base to write to" << endl;

	string baseFile = TakeInputFile().string();
	Image baseImage;

	cout << "load: " << baseFile << endl;
	baseImage.data = stbi_load(baseFile.c_str(), &baseImage.width, &baseImage.height, &baseImage.channels, STBI_rgb_alpha);
	assert(baseImage.data);
	cout << "width:\t\t" << baseImage.width << endl;
	cout << "height:\t\t" << baseImage.height << endl;
	cout << "channels:\t" << baseImage.channels << endl;

	while (WriteValueToChannel(baseImage))
	{
		cout << "Value was written to image." << endl;
	}
	
	
	while (AddImage(baseImage, inputImages) && inputImages.size() < 5)
	{
		cout << "input image added" << endl;
	}
	

	cout << "working..." << endl;
	for (int i = 0; i < baseImage.width * baseImage.height * 4; i += 4)
	{
		for (const auto& img : inputImages)
		{
			baseImage.data[i + (int)img.selectedOutPutChannel] = img.data[i + (int)img.selectedInputChannel];
		}
	}
	string outOutFileName = "";
	cout << "enter name of output file" << endl;
	cin >> outOutFileName;

	if (filesystem::path(outOutFileName).extension().string() != ".png") outOutFileName += ".png";
	if (!stbi_write_png(outOutFileName.c_str(), baseImage.width, baseImage.height, STBI_rgb_alpha, baseImage.data, baseImage.width * STBI_rgb_alpha))
	{
		cout << "write error" << endl;
	}


	stbi_image_free(baseImage.data);
	for (auto& img : inputImages)
	{
		stbi_image_free(img.data);
	}
	cout << "done" << endl;
	return 0;
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
	return "";
}

Channel SelectChannel()
{
	string inputSelectedChannel;
	while (true)
	{
		cout << endl << "select channel r, g, b, or a" << endl;
		cin >> inputSelectedChannel;
		Channel c = Channel::none;
		if (inputSelectedChannel == "r") c = Channel::r;
		if (inputSelectedChannel == "g") c = Channel::g;
		if (inputSelectedChannel == "b") c = Channel::b;
		if (inputSelectedChannel == "a") c = Channel::a;

		if (c != Channel::none)
			return c;
		else
			cout << "can not select that channal" << endl;
	}
}


bool AddImage(const Image& baseImage, vector<Image>& images)
{
	string yn = "";
	while (yn != "y" && yn != "n")
	{
		cout << endl << "load file to read from y/n?" << endl;
		cin >> yn;
	}

	if (yn == "y")
	{
		Image newImage;
		cout << "Enter file" << endl;
		string fileName = TakeInputFile().string();
		cout << "load: " << fileName << endl;
		newImage.data = stbi_load(fileName.c_str(), &newImage.width, &newImage.height, &newImage.channels, STBI_rgb_alpha);
		assert(newImage.data);
		cout << "width:\t\t" << newImage.width << endl;
		cout << "height:\t\t" << newImage.height << endl;
		cout << "channels:\t" << newImage.channels << endl;

		if (baseImage.width != newImage.width || baseImage.height != newImage.height)
		{
			cout << "texture dimensions does not match" << endl;
			stbi_image_free(newImage.data);
			return false;
		}

		cout << "Select channel to copy from." << endl;
		newImage.selectedInputChannel = SelectChannel();


	retry:
		cout << "which channel should " << ChannelToString(newImage.selectedInputChannel) << " from file be copied to ? " << endl;
		newImage.selectedOutPutChannel = SelectChannel();
		for (const auto& img : images)
		{
			if (newImage.selectedOutPutChannel == img.selectedOutPutChannel)
			{
				cout << "that channel can not be selected again" << endl;
				goto retry;
			}
		}
		images.push_back(newImage);
		return true;
	}
	return false;
}

bool WriteValueToChannel(const Image& baseImage)
{
	unsigned int val = -1;
	Channel c;
	string yn = "";
	while (yn != "y" && yn != "n")
	{
		cout << endl << "write value to image y/n?" << endl;
		cin >> yn;
	}

	if (yn == "y")
	{
		c = SelectChannel();
		while (!(0 <= val && val <= 255))
		{
			cout << "Enter Value in range of [0, 255] to write" << endl,
			cin >> val;
		}
		cout << "working..." << endl;
		for (int i = 0; i < baseImage.width * baseImage.height * 4; i += 4)
		{
			baseImage.data[i + (int)c] = (unsigned char)val;
		}
		return true;
	}
	else
	{
		return false;
	}

	
}



filesystem::path TakeInputFile()
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
