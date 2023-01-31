#include "pch.h"
#include "GlassShader.h"

bool GlassShader::InitGlassShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	if (!InitRefractionShader(device, vsFilename, psFilename))
	{
		return false;
	}

	return true;
}

bool GlassShader::SetGlassShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, float time, Light* light, float opacity, float refractiveIndex, bool frontFaceCulling, Camera* camera, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalTexture, ID3D11ShaderResourceView* refractionMap[6], ID3D11ShaderResourceView* reflectionMap[6])
{
	SetRefractionShaderParameters(context, world, view, projection, time, light, opacity, refractiveIndex, frontFaceCulling, camera, texture, normalTexture, refractionMap);

	//pass the desired texture to the pixel shader.
	//pass the desired texture to the pixel shader.
	for (int i = 0; i < 6; i++)
		context->PSSetShaderResources(8+i, 1, &reflectionMap[i]);

	return false;
}