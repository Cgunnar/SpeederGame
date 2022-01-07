#include "pch.hpp"
#include "CameraControllerScript.h"
#include "StandardComponents.h"

#include "Input.h"

using namespace rfm;

void CameraControllerScript::OnUpdate(float dt)
{
	if (m_lock) return;
	Input& in = Input::Get();
	MouseState ms = in.GetMouse().GetMouseState();

	m_moveSpeed += 0.5f * in.GetMouse().GetMouseState().deltaZ;

	m_pitch += ms.deltaY * ms.mouseCof;
	m_yaw += ms.deltaX * ms.mouseCof;
	m_pitch = std::clamp(m_pitch, -PIDIV2, PIDIV2);
	m_yaw = fmod(m_yaw, PI2);

	Vector3 moveDir{ 0,0,0 };
	if (in.keyBeingPressed(Input::D)) moveDir += {1, 0, 0};
	if (in.keyBeingPressed(Input::A)) moveDir += {-1, 0, 0};
	if (in.keyBeingPressed(Input::W)) moveDir += {0, 0, 1};
	if (in.keyBeingPressed(Input::S)) moveDir += {0, 0, -1};


	Transform& cameraTransform = GetComponent<TransformComp>()->transform;
	cameraTransform.setRotation(m_pitch, m_yaw, 0);

	moveDir = cameraTransform.getRotationMatrix() * Vector4(moveDir, 0);

	if (in.keyBeingPressed(Input::Space)) moveDir += {0, 1, 0};
	if (in.keyBeingPressed(Input::C)) moveDir += {0, -1, 0};
	moveDir.normalize();

	if (in.keyBeingPressed(Input::Shift)) moveDir *= 2;
	moveDir *= m_moveSpeed;
	cameraTransform.translateW(moveDir * dt);
	
}

void CameraControllerScript::ToggleCameraLock()
{
	m_lock = !m_lock;
}
