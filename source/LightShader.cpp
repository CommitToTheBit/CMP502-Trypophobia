#include "pch.h"
#include "LightShader.h"

bool LightShader::InitLightShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	if (!InitShader(device, vsFilename, psFilename))
	{
		return false;
	}

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);

	return true;
}

bool LightShader::SetLightShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, float time, Light* light, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalTexture)
{
	SetShaderParameters(context, world, view, projection, time);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	LightBufferType* lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = light->getAmbientColour();
	lightPtr->diffuse = light->getDiffuseColour();
	lightPtr->position = light->getPosition();
	lightPtr->strength = light->getStrength();
	context->Unmap(m_lightBuffer, 0);
	context->PSSetConstantBuffers(1, 1, &m_lightBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	//pass the desired texture to the pixel shader.
	context->PSSetShaderResources(0, 1, &texture);
	context->PSSetShaderResources(1, 1, &normalTexture);

	return false;
}