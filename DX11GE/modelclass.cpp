////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"


ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_vertices = 0;
	m_indices = 0;

	m_textureCount = 0;
	m_normalCount = 0;
	m_faceCount = 0;

	m_Texture = 0;
	m_NormalTexture = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* device, const WCHAR* modelFilename, const WCHAR* textureFilename)
{
	bool result;

	// Load in the model data,
	result = LoadModel(modelFilename, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	if (!result)
	{
		return false;
	}

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	// Load the texture for this model.
	result = LoadTexture(device, textureFilename);
	if(!result)
	{
		return false;
	}

	return true;
}


void ModelClass::Shutdown()
{
	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	// Release the model data.
	ReleaseModel();

	// Release the model texture.
	ReleaseTexture();

	return;
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}


int ModelClass::GetIndexCount()
{
	return m_indexCount;
}


ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}

bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	if (!m_vertices || !m_indices)
		return false;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = m_indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


bool ModelClass::LoadTexture(ID3D11Device* device, const WCHAR* filename)
{
	bool result;


	// Create the texture object.
	m_Texture = new TextureClass;
	if(!m_Texture)
	{
		return false;
	}

	// Initialize the texture object.
	result = m_Texture->Initialize(device, filename);
	if(!result)
	{
		return false;
	}

	return true;
}

ID3D11ShaderResourceView* ModelClass::GetNormalTexture()
{
	return m_NormalTexture ? m_NormalTexture->GetTexture() : nullptr;
}

bool ModelClass::SetNormalTexture(ID3D11Device* device, const WCHAR* filename)
{
	// 이미 있으면 정리
	if (m_NormalTexture)
	{
		m_NormalTexture->Shutdown();
		delete m_NormalTexture;
		m_NormalTexture = 0;
	}

	m_NormalTexture = new TextureClass;
	if (!m_NormalTexture)
		return false;

	bool result = m_NormalTexture->Initialize(device, filename);
	if (!result)
		return false;

	return true;
}


void ModelClass::ReleaseTexture()
{
	// Release the texture object.
	if(m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	if (m_NormalTexture)
	{
		m_NormalTexture->Shutdown();
		delete m_NormalTexture;
		m_NormalTexture = 0;
	}

	return;
}

bool ModelClass::LoadModel(CString filename, UINT flag)
{

	// Load model using ASSIMP
	Assimp::Importer importer;
	string strPath = std::string(CT2CA(filename.operator LPCWSTR()));
	const aiScene* pScene = importer.ReadFile(strPath, flag);

	if (!pScene) return false;

	const aiMesh* pMesh = pScene->mMeshes[0];	// Assume it's a single mesh

	if (!pMesh)
		return false;

	m_vertexCount = pMesh->mNumVertices;
	m_faceCount = pMesh->mNumFaces;
	m_indexCount = m_faceCount * 3;

	if (m_vertexCount == 0 || m_indexCount == 0)
		return false;

	m_vertices = new VertexType[m_vertexCount];

	// Fill each vertex data
	for (unsigned int i = 0; i < (unsigned int)m_vertexCount; i++)
	{
		// Get position
		m_vertices[i].position.x = pMesh->mVertices[i].x;
		m_vertices[i].position.y = pMesh->mVertices[i].y;
		m_vertices[i].position.z = pMesh->mVertices[i].z;

		// Get UV
		if (pMesh->HasTextureCoords(0)) {
			m_vertices[i].texture.x = pMesh->mTextureCoords[0][i].x;
			m_vertices[i].texture.y = pMesh->mTextureCoords[0][i].y;
		}
		else {
			m_vertices[i].texture.x = 0;
			m_vertices[i].texture.y = 0;
		}

		// Get normal
		m_vertices[i].normal.x = pMesh->mNormals[i].x;
		m_vertices[i].normal.y = pMesh->mNormals[i].y;
		m_vertices[i].normal.z = pMesh->mNormals[i].z;
	}

	m_indices = new unsigned long[m_indexCount];

	for (unsigned int i = 0; i < (unsigned int)m_faceCount; i++)
	{
		aiFace face = pMesh->mFaces[i];

		// 위치 읽어오기
		m_indices[i * 3] = face.mIndices[0];
		m_indices[i * 3 + 1] = face.mIndices[1];
		m_indices[i * 3 + 2] = face.mIndices[2];
	}

	return true;
}

void ModelClass::ReleaseModel()
{
	if (m_vertices)
	{
		delete[] m_vertices;
		m_vertices = 0;
	}

	if (m_indices)
	{
		delete[] m_indices;
		m_vertices = 0;
	}

	return;
}
