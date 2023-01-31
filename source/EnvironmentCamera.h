#pragma once
#include "Camera.h"

class EnvironmentCamera
{
public:
	EnvironmentCamera();
	~EnvironmentCamera();

	void							Update();
	Camera*							getCamera(int i);
	void							setPosition(DirectX::SimpleMath::Vector3 newPosition);
	DirectX::SimpleMath::Vector3	getPosition();

private:
	Camera*							m_cameras[6];
	DirectX::SimpleMath::Vector3	m_position;
};