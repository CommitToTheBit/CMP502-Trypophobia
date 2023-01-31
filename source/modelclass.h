////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_


//////////////
// INCLUDES //
//////////////
#include "pch.h"
//#include <d3dx10math.h>
//#include <fstream>
//using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////

using namespace DirectX;

class ModelClass
{
private:
	struct VertexType
	{

		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texture;
		DirectX::SimpleMath::Vector3 normal;
		DirectX::SimpleMath::Vector3 tangent;
		DirectX::SimpleMath::Vector3 binormal;
	};

public:
	ModelClass();
	~ModelClass();

	bool InitializeModel(ID3D11Device *device, char* filename);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	
	int GetIndexCount();


private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	bool LoadModel(char*);

	void ReleaseModel();

	void CalculateModelVectors();
	void CalculateNormalTangentBinormal(VertexPositionNormalTexture vertex1, VertexPositionNormalTexture vertex2, VertexPositionNormalTexture vertex3, DirectX::SimpleMath::Vector3& normal, DirectX::SimpleMath::Vector3& tangent, DirectX::SimpleMath::Vector3& binormal);

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;

	//arrays for our generated objects Made by directX
	std::vector<VertexType> preFabVertices;
	std::vector<uint16_t> preFabIndices;

};

#endif