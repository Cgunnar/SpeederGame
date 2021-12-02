#include "pch.hpp"
#include "Application.h"
#include "rfEntity.hpp"

#define VLD_MEMORYDEBUG
#ifdef VLD_MEMORYDEBUG
#include <vld.h>
#endif // VLD_MEMORYDEBUG


int main()
{
	Application myGame;

	myGame.Run();
	rfe::EntityReg::clear();
	return 0;
}