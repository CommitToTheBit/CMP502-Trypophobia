#pragma once
#include "Shader.h"
#include "RenderTexture.h"

class OverlayShader : public Shader
{
public:
	bool InitOverlayShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);
	bool SetOverlayShaderParameters(ID3D11DeviceContext* context,
		DirectX::SimpleMath::Matrix* world,
		DirectX::SimpleMath::Matrix* view,
		DirectX::SimpleMath::Matrix* projection,
		float time,
		ID3D11ShaderResourceView* texture,
		ID3D11ShaderResourceView* overlayTexture,
		ID3D11ShaderResourceView* overlayAlphaMap);
};