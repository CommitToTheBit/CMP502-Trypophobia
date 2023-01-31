#pragma once
#include "Shader.h"
#include "RenderTexture.h"

class AlphaShader : public Shader
{
public:
	bool InitAlphaShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);
	bool SetAlphaShaderParameters(ID3D11DeviceContext* context,
		DirectX::SimpleMath::Matrix* world,
		DirectX::SimpleMath::Matrix* view,
		DirectX::SimpleMath::Matrix* projection,
		float time,
		float alpha,
		ID3D11ShaderResourceView* alphaMap);

protected:
	struct AlphaBufferType
	{
		float alpha;
		DirectX::SimpleMath::Vector3 padding;
	};

	ID3D11Buffer* m_alphaBuffer;
};