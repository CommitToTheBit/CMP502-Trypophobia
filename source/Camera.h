#pragma once
class Camera
{
public:
	Camera();
	~Camera();

	void							Update();
	DirectX::SimpleMath::Matrix		getCameraMatrix();
	void							setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);
	DirectX::SimpleMath::Matrix		getPerspective();
	void							setPosition(DirectX::SimpleMath::Vector3 newPosition);
	DirectX::SimpleMath::Vector3	getPosition();
	DirectX::SimpleMath::Vector3	getForward();
	DirectX::SimpleMath::Vector3	getRight();
	void							setRotation(DirectX::SimpleMath::Vector3 newRotation);
	DirectX::SimpleMath::Vector3	getRotation();
	void							setReflection(bool newReflection);
	bool							getReflection();
	float							getMoveSpeed();
	float							getRotationSpeed();

private:
	DirectX::SimpleMath::Matrix		m_cameraMatrix;			//camera matrix to be passed out and used to set camera position and angle for wrestling
	DirectX::SimpleMath::Matrix		m_perspective;
	DirectX::SimpleMath::Vector3	m_lookat;
	DirectX::SimpleMath::Vector3	m_position;
	DirectX::SimpleMath::Vector3	m_forward;
	DirectX::SimpleMath::Vector3	m_right;
	DirectX::SimpleMath::Vector3	m_up;
	DirectX::SimpleMath::Vector3	m_orientation;			//vector storing pitch yaw and roll. 
	bool							m_reflection;

	float	m_movespeed ;	
	float	m_camRotRate;

};
