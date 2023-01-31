#include "pch.h"
#include "OverlayShader.h"

bool OverlayShader::InitOverlayShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	if (!InitShader(device, vsFilename, psFilename))
	{
		return false;
	}

	return true;
}

bool OverlayShader::SetOverlayShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, float time, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* overlayTexture, ID3D11ShaderResourceView* overlayAlphaMap)
{
	SetShaderParameters(context, world, view, projection, time);

	//pass the desired texture to the pixel shader.
	context->PSSetShaderResources(0, 1, &texture);
	context->PSSetShaderResources(1, 1, &overlayTexture);
	context->PSSetShaderResources(2, 1, &overlayAlphaMap);

	return false;
}