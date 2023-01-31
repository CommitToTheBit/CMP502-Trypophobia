#include "pch.h"
#include "SkyboxShader.h"

bool SkyboxShader::InitSkyboxShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	if (!InitShader(device, vsFilename, psFilename))
	{
		return false;
	}

	return true;
}

bool SkyboxShader::SetSkyboxShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, float time, ID3D11ShaderResourceView* environmentMap[6])
{
	SetShaderParameters(context, world, view, projection, time);

	//pass the desired texture to the pixel shader.
	for (int i = 0; i < 6; i++)
		context->PSSetShaderResources(i, 1, &environmentMap[i]);

	return false;
}