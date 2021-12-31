#include "pch.hpp"
#include "Application.h"
#include "rfEntity.hpp"

#define VLD_MEMORYDEBUG
#ifdef VLD_MEMORYDEBUG
#include <vld.h>
#endif // VLD_MEMORYDEBUG

int main()
{
	Application* myGame = new Application();

	myGame->Run();
	delete myGame;
	rfe::EntityReg::Clear();
	return 0;
}