#include "pch.hpp"
#include "Application.h"
#include "rfEntity.hpp"

int main()
{
	Application myGame;

	myGame.Run();
	rfe::EntityReg::clear();
	return 0;
}