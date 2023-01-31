#pragma once
#include "RefractionShader.h"
#include "Camera.h"

class GlassShader : public RefractionShader
{
public:
	bool InitGlassShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);
	bool SetGlassShaderParameters(ID3D11DeviceContext* context,
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
		ID3D11ShaderResourceView* refractionMap[6],
		ID3D11ShaderResourceView* reflectionMap[6]);
};