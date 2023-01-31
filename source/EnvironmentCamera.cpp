#include "pch.h"
#include "EnvironmentCamera.h"

EnvironmentCamera::EnvironmentCamera()
{
	for (int i = 0; i < 6; i++)
		m_cameras[i] = new Camera();

	// Layout of cube maps; designed to resemble a 'stereotypical' cube net
	m_cameras[0]->setRotation(DirectX::SimpleMath::Vector3(-90.0, -90.0, 0.0)); // Views negative x-direction
	m_cameras[1]->setRotation(DirectX::SimpleMath::Vector3(-90.0, 0.0, 0.0)); // Views positive z-direction
	m_cameras[2]->setRotation(DirectX::SimpleMath::Vector3(-90.0, 90.0, 0.0)); // Views positive x-direction
	m_cameras[3]->setRotation(DirectX::SimpleMath::Vector3(-90.0, 180.0, 0.0)); // Views negative z-direction
	m_cameras[4]->setRotation(DirectX::SimpleMath::Vector3(-0.001, 0.0, 0.0)); // Views positive y-direction (camera cannot look directly up due to LookAt function)
	m_cameras[5]->setRotation(DirectX::SimpleMath::Vector3(-180.0, 0.0, 0.0)); // Views negative y-direction

	m_cameras[4]->setReflection(true);
	m_cameras[5]->setReflection(true);

	m_position = DirectX::SimpleMath::Vector3(0.0, 0.0, 0.0);

	Update();
}

EnvironmentCamera::~EnvironmentCamera()
{
}

void EnvironmentCamera::Update()
{
	for (int i = 0; i < 6; i++)
	{
		m_cameras[i]->setPosition(m_position);
		m_cameras[i]->Update();
	}
}

Camera* EnvironmentCamera::getCamera(int i)
{
	return m_cameras[(i%6+6)%6];
}

void EnvironmentCamera::setPosition(DirectX::SimpleMath::Vector3 newPosition)
{
	m_position = newPosition;
}

DirectX::SimpleMath::Vector3 EnvironmentCamera::getPosition()
{
	return m_position;
}