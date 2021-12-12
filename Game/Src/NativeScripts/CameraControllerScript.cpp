#include "pch.hpp"
#include "CameraControllerScript.h"
#include "StandardComponents.h"

#include "Input.h"

using namespace rfm;

void CameraControllerScript::OnUpdate(float dt)
{
	Vector3 moveDir{ 0,0,0 };

	Input& in = Input::Get();

	if (in.keyBeingPressed(Input::D)) moveDir += {1, 0, 0};
	if (in.keyBeingPressed(Input::A)) moveDir += {-1, 0, 0};
	if (in.keyBeingPressed(Input::W)) moveDir += {0, 0, 1};
	if (in.keyBeingPressed(Input::S)) moveDir += {0, 0, -1};
	moveDir.normalize();
	moveDir *= m_moveSpeed;

	if (moveDir.length() > 0)
	{
		getComponent<TransformComp>()->transform.translate(moveDir * dt);
	}
}
