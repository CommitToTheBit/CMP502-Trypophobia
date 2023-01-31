#include "pch.h"
#include "RefractionShader.h"

bool RefractionShader::InitRefractionShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	if (!InitLightShader(device, vsFilename, psFilename))
	{
		return false;
	}

	D3D11_BUFFER_DESC refractionBufferDesc;
	refractionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	refractionBufferDesc.ByteWidth = sizeof(RefractionBufferType);
	refractionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	refractionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	refractionBufferDesc.MiscFlags = 0;
	refractionBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&refractionBufferDesc, NULL, &m_refractionBuffer);

	D3D11_BUFFER_DESC cameraBufferDesc;
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);

	return true;
}

bool RefractionShader::SetRefractionShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, float time, Light* light, float opacity, float refractiveIndex, bool frontFaceCulling, Camera* camera, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalTexture, ID3D11ShaderResourceView* environmentMap[6])
{
	SetLightShaderParameters(context, world, view, projection, time, light, texture, normalTexture);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_refractionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	RefractionBufferType* refractionPtr = (RefractionBufferType*)mappedResource.pData;
	refractionPtr->opacity = opacity;
	refractionPtr->refractiveIndex = refractiveIndex;
	refractionPtr->culling = (frontFaceCulling) ? 1.0 : -1.0;
	refractionPtr->padding = 0.0;
	context->Unmap(m_refractionBuffer, 0);
	context->PSSetConstantBuffers(2, 1, &m_refractionBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	context->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	CameraBufferType* cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->cameraPosition = camera->getPosition();
	cameraPtr->padding = 0.0;
	context->Unmap(m_cameraBuffer, 0);
	context->PSSetConstantBuffers(3, 1, &m_cameraBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	//pass the desired texture to the pixel shader.
	for (int i = 0; i < 6; i++)
		context->PSSetShaderResources(2+i, 1, &environmentMap[i]);

	return false;
}