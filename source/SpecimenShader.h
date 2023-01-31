#pragma once
#include "LightShader.h"

class SpecimenShader : public LightShader
{
public:
	bool InitSpecimenShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);
	bool SetSpecimenShaderParameters(ID3D11DeviceContext* context,
		DirectX::SimpleMath::Matrix* world,
		DirectX::SimpleMath::Matrix* view,
		DirectX::SimpleMath::Matrix* projection,
		float time,
		Light* light,
		float opacity,
		ID3D11ShaderResourceView* texture,
		ID3D11ShaderResourceView* normalTexture,
		ID3D11ShaderResourceView* specimenTexture);

protected:
	struct SpecimenBufferType
	{
		float opacity;
		DirectX::SimpleMath::Vector3 padding;
	};

	ID3D11Buffer* m_specimenBuffer;
};


