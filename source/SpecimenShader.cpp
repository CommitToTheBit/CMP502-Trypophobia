#include "pch.h"
#include "SpecimenShader.h"

bool SpecimenShader::InitSpecimenShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	if (!InitLightShader(device, vsFilename, psFilename))
	{
		return false;
	}

	D3D11_BUFFER_DESC specimenBufferDesc;
	specimenBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	specimenBufferDesc.ByteWidth = sizeof(SpecimenBufferType);
	specimenBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	specimenBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	specimenBufferDesc.MiscFlags = 0;
	specimenBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&specimenBufferDesc, NULL, &m_specimenBuffer);

	return true;
}

bool SpecimenShader::SetSpecimenShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, float time, Light* light, float opacity, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalTexture, ID3D11ShaderResourceView* specimenTexture)
{
	SetLightShaderParameters(context, world, view, projection, time, light, texture, normalTexture);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_specimenBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	SpecimenBufferType* specimenPtr = (SpecimenBufferType*)mappedResource.pData;
	specimenPtr->opacity = opacity;
	specimenPtr->padding = DirectX::SimpleMath::Vector3(0.0, 0.0, 0.0);
	context->Unmap(m_specimenBuffer, 0);
	context->PSSetConstantBuffers(2, 1, &m_specimenBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	//pass the desired texture to the pixel shader.
	context->PSSetShaderResources(2, 1, &specimenTexture);

	return false;
}