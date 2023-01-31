#include "pch.h"
#include "Shader.h"


Shader::Shader()
{
}


Shader::~Shader()
{
}

bool Shader::InitShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	//LOAD SHADER:	VERTEX
	auto vertexShaderBuffer = DX::ReadData(vsFilename);
	HRESULT result = device->CreateVertexShader(vertexShaderBuffer.data(), vertexShaderBuffer.size(), NULL, &m_vertexShader);
	if (result != S_OK)
	{
		//if loading failed.  
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the MeshClass and in the shader.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Get a count of the elements in the layout.
	unsigned int numElements;
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer.data(), vertexShaderBuffer.size(), &m_layout);


	//LOAD SHADER:	PIXEL
	auto pixelShaderBuffer = DX::ReadData(psFilename);
	result = device->CreatePixelShader(pixelShaderBuffer.data(), pixelShaderBuffer.size(), NULL, &m_pixelShader);
	if (result != S_OK)
	{
		//if loading failed. 
		return false;
	}

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	device->CreateSamplerState(&samplerDesc, &m_sampleState);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC timeBufferDesc;
	timeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	timeBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	timeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	timeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	timeBufferDesc.MiscFlags = 0;
	timeBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&timeBufferDesc, NULL, &m_timeBuffer);

	return true;
}

bool Shader::SetShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, float time)
{ 
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = world->Transpose(); 	// Transpose the matrices to prepare them for the shader.
	dataPtr->view = view->Transpose();
	dataPtr->projection = projection->Transpose();

	context->Unmap(m_matrixBuffer, 0);
	context->VSSetConstantBuffers(0, 1, &m_matrixBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the VS;

	context->Map(m_timeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	TimeBufferType* timePtr = (TimeBufferType*)mappedResource.pData;
	timePtr->time = time;
	timePtr->padding = DirectX::SimpleMath::Vector3(0.0, 0.0, 0.0);
	context->Unmap(m_timeBuffer, 0);
	context->PSSetConstantBuffers(0, 1, &m_timeBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	timePtr->time = time;
	timePtr->padding = DirectX::SimpleMath::Vector3(0.0, 0.0, 0.0);

	return false;
}

void Shader::EnableShader(ID3D11DeviceContext* context)
{
	context->IASetInputLayout(m_layout);							//set the input layout for the shader to match out geometry
	context->VSSetShader(m_vertexShader.Get(), 0, 0);				//turn on vertex shader
	context->PSSetShader(m_pixelShader.Get(), 0, 0);				//turn on pixel shader
	context->PSSetSamplers(0, 1, &m_sampleState);					// Set the sampler state in the pixel shader.

}

/*bool Shader::InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC	matrixBufferDesc;
	D3D11_SAMPLER_DESC	samplerDesc;
	D3D11_BUFFER_DESC	lightBufferDesc;
	D3D11_BUFFER_DESC	gameBufferDesc;
	D3D11_BUFFER_DESC	materialBufferDesc;
	D3D11_BUFFER_DESC	cameraBufferDesc;

	//LOAD SHADER:	VERTEX
	auto vertexShaderBuffer = DX::ReadData(vsFilename);
	HRESULT result = device->CreateVertexShader(vertexShaderBuffer.data(), vertexShaderBuffer.size(), NULL, &m_vertexShader);
	if (result != S_OK)
	{
		//if loading failed.  
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the MeshClass and in the shader.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Get a count of the elements in the layout.
	unsigned int numElements;
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer.data(), vertexShaderBuffer.size(), &m_layout);
	

	//LOAD SHADER:	PIXEL
	auto pixelShaderBuffer = DX::ReadData(psFilename);	
	result = device->CreatePixelShader(pixelShaderBuffer.data(), pixelShaderBuffer.size(), NULL, &m_pixelShader);
	if (result != S_OK)
	{
		//if loading failed. 
		return false;
	}

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	device->CreateSamplerState(&samplerDesc, &m_sampleState);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);

	// Setup game buffer.
	gameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	gameBufferDesc.ByteWidth = sizeof(GameBufferType);
	gameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	gameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	gameBufferDesc.MiscFlags = 0;
	gameBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&gameBufferDesc, NULL, &m_gameBuffer);

	// Setup material buffer.
	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(MaterialBufferType);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&materialBufferDesc, NULL, &m_materialBuffer);

	// Setup camera buffer.
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);

	return true;
}

bool Shader::SetShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, float time, Light* sceneLight1, ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* normalTexture)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* lightPtr;
	GameBufferType* gamePtr;
	DirectX::SimpleMath::Matrix  tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = world->Transpose();
	tview = view->Transpose();
	tproj = projection->Transpose();
	context->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	context->Unmap(m_matrixBuffer, 0);
	context->VSSetConstantBuffers(0, 1, &m_matrixBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the VS

	context->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = sceneLight1->getAmbientColour();
	lightPtr->diffuse = sceneLight1->getDiffuseColour();	
	lightPtr->position = sceneLight1->getPosition();  
	lightPtr->padding = 0.0f;
	context->Unmap(m_lightBuffer, 0);
	context->PSSetConstantBuffers(0, 1, &m_lightBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	context->Map(m_gameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	gamePtr = (GameBufferType*)mappedResource.pData;
	gamePtr->time = time;
	gamePtr->padding = DirectX::SimpleMath::Vector3(0.0, 0.0, 0.0);
	context->Unmap(m_gameBuffer, 0);
	context->PSSetConstantBuffers(1, 1, &m_gameBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	//pass the desired texture to the pixel shader.
	context->PSSetShaderResources(0, 1, &texture1);
	context->PSSetShaderResources(1, 1, &normalTexture);

	return false;
}

void Shader::EnableShader(ID3D11DeviceContext * context)
{
	context->IASetInputLayout(m_layout);							//set the input layout for the shader to match out geometry
	context->VSSetShader(m_vertexShader.Get(), 0, 0);				//turn on vertex shader
	context->PSSetShader(m_pixelShader.Get(), 0, 0);				//turn on pixel shader
	// Set the sampler state in the pixel shader.
	context->PSSetSamplers(0, 1, &m_sampleState);

}

bool GlassShader::SetShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, float time, Light* sceneLight1, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalTexture, ID3D11ShaderResourceView* specimenTexture, ID3D11ShaderResourceView* specimenAlphaTexture, ID3D11ShaderResourceView* frontFaceCubeMap[6], ID3D11ShaderResourceView* backFaceCubeMap[6], float refractiveIndex, float refractivity, float reflectivity, DirectX::SimpleMath::Vector3 cameraPosition)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* lightPtr;
	GameBufferType* gamePtr;
	MaterialBufferType* materialPtr;
	CameraBufferType* cameraPtr;
	DirectX::SimpleMath::Matrix  tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = world->Transpose();
	tview = view->Transpose();
	tproj = projection->Transpose();
	context->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	context->Unmap(m_matrixBuffer, 0);
	context->VSSetConstantBuffers(0, 1, &m_matrixBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the VS

	context->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = sceneLight1->getAmbientColour();
	lightPtr->diffuse = sceneLight1->getDiffuseColour();
	lightPtr->position = sceneLight1->getPosition();
	lightPtr->padding = 0.0f;
	context->Unmap(m_lightBuffer, 0);
	context->PSSetConstantBuffers(0, 1, &m_lightBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	context->Map(m_gameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	gamePtr = (GameBufferType*)mappedResource.pData;
	gamePtr->time = time;
	gamePtr->padding = DirectX::SimpleMath::Vector3(0.0, 0.0, 0.0);
	context->Unmap(m_gameBuffer, 0);
	context->PSSetConstantBuffers(1, 1, &m_gameBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	context->Map(m_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	materialPtr = (MaterialBufferType*)mappedResource.pData;
	materialPtr->refractiveIndex = refractiveIndex;
	materialPtr->refractivity = refractivity;
	materialPtr->reflectivity = reflectivity;
	materialPtr->padding = 0.0f;
	context->Unmap(m_materialBuffer, 0);
	context->PSSetConstantBuffers(2, 1, &m_materialBuffer);

	context->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->cameraPosition = cameraPosition;
	cameraPtr->padding = 0.0;
	context->Unmap(m_cameraBuffer, 0);
	context->PSSetConstantBuffers(3, 1, &m_cameraBuffer);

	//pass the desired texture to the pixel shader.
	context->PSSetShaderResources(0, 1, &texture);
	context->PSSetShaderResources(1, 1, &normalTexture);
	context->PSSetShaderResources(2, 1, &specimenTexture);
	context->PSSetShaderResources(3, 1, &specimenAlphaTexture);
	for (int i = 0; i < 6; i++)
	{
		context->PSSetShaderResources(4+i, 1, &frontFaceCubeMap[i]);
		context->PSSetShaderResources(10+i, 1, &backFaceCubeMap[i]);
	}

	return false;
}*/
