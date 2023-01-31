#pragma once
#include "Shader.h"

class LightShader : Shader
{
public:
	using Shader::InitShader;
	using Shader::SetShaderParameters;
	using Shader::EnableShader;

	bool InitLightShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);
	bool SetLightShaderParameters(ID3D11DeviceContext* context,
		DirectX::SimpleMath::Matrix* world,
		DirectX::SimpleMath::Matrix* view,
		DirectX::SimpleMath::Matrix* projection,
		float time,
		Light* light,
		ID3D11ShaderResourceView* texture,
		ID3D11ShaderResourceView* normalTexture);

protected:
	//buffer for information of a single light
	struct LightBufferType
	{
		DirectX::SimpleMath::Vector4 ambient;
		DirectX::SimpleMath::Vector4 diffuse;
		DirectX::SimpleMath::Vector3 position;
		float strength;
	};

	ID3D11Buffer* m_lightBuffer;
};

