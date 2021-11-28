#pragma once
#include <string>

struct MyImageStruct
{
	MyImageStruct() = default;
	MyImageStruct(const MyImageStruct& other) = delete;
	MyImageStruct& operator=(const MyImageStruct& other) = delete;
	~MyImageStruct();
	
	int width = 0;
	int height = 0;
	int mipNumber = 0;
	int stride = 0;
	int bpp = 0;
	bool alphaBlending = false;
	bool alphaTesting = false;
	void* imagePtr = nullptr;
};

void readImage(MyImageStruct& pImageStruct, const std::string& fileName);