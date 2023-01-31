#pragma once
#include "Shader.h"
#include "RenderTexture.h"

class SkyboxShader : Shader
{
public:
	using Shader::InitShader;
	using Shader::SetShaderParameters;
	using Shader::EnableShader;

	bool InitSkyboxShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);
	bool SetSkyboxShaderParameters(ID3D11DeviceContext* context,
		DirectX::SimpleMath::Matrix* world,
		DirectX::SimpleMath::Matrix* view,
		DirectX::SimpleMath::Matrix* projection,
		float time,
		ID3D11ShaderResourceView* environmentMap[6]);
};

