#pragma once
#include "LightShader.h"
#include "Camera.h";

class RefractionShader : public LightShader
{
public:
	bool InitRefractionShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);
	bool SetRefractionShaderParameters(ID3D11DeviceContext* context,
		DirectX::SimpleMath::Matrix* world,
		DirectX::SimpleMath::Matrix* view,
		DirectX::SimpleMath::Matrix* projection,
		float time,
		Light* light,
		float opacity,
		float refractiveIndex,
		bool frontFaceCulling,
		Camera* camera,
		ID3D11ShaderResourceView* texture,
		ID3D11ShaderResourceView* normalTexture,
		ID3D11ShaderResourceView* environmentMap[6]);

protected:
	struct RefractionBufferType
	{
		float opacity;
		float refractiveIndex;
		float culling;
		float padding;
	};

	struct CameraBufferType
	{
		DirectX::SimpleMath::Vector3 cameraPosition;
		float padding;
	};

	ID3D11Buffer* m_refractionBuffer;
	ID3D11Buffer* m_cameraBuffer;
};
