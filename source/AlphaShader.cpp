#include "pch.h"
#include "AlphaShader.h"

bool AlphaShader::InitAlphaShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	if (!InitShader(device, vsFilename, psFilename))
	{
		return false;
	}

	D3D11_BUFFER_DESC alphaBufferDesc;
	alphaBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	alphaBufferDesc.ByteWidth = sizeof(AlphaBufferType);
	alphaBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	alphaBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	alphaBufferDesc.MiscFlags = 0;
	alphaBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&alphaBufferDesc, NULL, &m_alphaBuffer);

	return true;
}

bool AlphaShader::SetAlphaShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, float time, float alpha, ID3D11ShaderResourceView* alphaMap)
{
	SetShaderParameters(context, world, view, projection, time);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_alphaBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	AlphaBufferType* alphaPtr = (AlphaBufferType*)mappedResource.pData;
	alphaPtr->alpha = alpha;
	alphaPtr->padding = DirectX::SimpleMath::Vector3(0.0, 0.0, 0.0);
	context->Unmap(m_alphaBuffer, 0);
	context->PSSetConstantBuffers(1, 1, &m_alphaBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	//pass the desired texture to the pixel shader.
	context->PSSetShaderResources(0, 1, &alphaMap);

	return false;
}