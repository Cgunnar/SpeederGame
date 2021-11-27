#pragma once

#include <fstream>
#include <assert.h>
inline void writefileBin(char* src, size_t numElemts, size_t elementSize, const std::string& path)
{
	std::ofstream output(path, std::ios::binary);
	output.write(src, numElemts * elementSize);
	output.close();
}

//pass zero in fileSize, to get the size returned as output, then call again with fileSize to fill char*
inline void readfileBin(char* dst, size_t& fileSize, const std::string& path)
{
	std::ifstream input(path, std::ios::binary);
	if (fileSize)
	{
		input.read(dst, fileSize);
	}
	else
	{
		input.seekg(0, input.end);
		fileSize = input.tellg();
		assert(fileSize != -1); // tellg fail
	}
	input.close();
}