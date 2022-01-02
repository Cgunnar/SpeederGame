#include "pch.hpp"
#include "TerrainScript.h"
#include "PerlinNoise.hpp"
#include "stb_image_write.h"
#include "stb_image.h"

void TerrainScript::OnStart()
{
	constexpr uint32_t width = 1024;
	constexpr uint32_t height = 1024;

	const siv::PerlinNoise::seed_type seed = 123456u;

	const siv::PerlinNoise perlin{ seed };
	unsigned char* noise = new unsigned char[width * height]();
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			noise[y * width + x] = static_cast<unsigned char>(255 * perlin.octave2D_01((x * 0.01), (y * 0.01), 3));
		}
	}

	/*if (!stbi_write_bmp("testNoise.bmp", width, height, STBI_grey, noise))
	{
		std::cout << "write error" << std::endl;
	}*/
	if (!stbi_write_png("testNoise.png", width, height, STBI_grey, noise, width * STBI_grey))
	{
		std::cout << "write error" << std::endl;
	}

	delete[] noise;
}

void TerrainScript::OnUpdate(float dt)
{

}
