#include "pch.h"
#include "RMShader.h"


HRESULT RMShader::Init(ID3D11Device * device)
{
	D3D11_BUFFER_DESC matrixBufferDesc;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;


	matrixBufferDesc.ByteWidth = sizeof(CameraBuffer);
	if (FAILED(device->CreateBuffer(&matrixBufferDesc, NULL, &_cameraBuffer))) {
	//	AfxMessageBox(L"Shader Manager Error");
		return E_FAIL;
	}

	matrixBufferDesc.ByteWidth = sizeof(TransBuffer);
	if (FAILED(device->CreateBuffer(&matrixBufferDesc, NULL, &_transBuffer))) {
	//	AfxMessageBox(L"Shader Manager Error");

		return E_FAIL;
	}

	matrixBufferDesc.ByteWidth = sizeof(ColorBuffer);
	if (FAILED(device->CreateBuffer(&matrixBufferDesc, NULL, &_colorBuffer))) {
	//	MessageBox(L"Shader Manager Error");

		return E_FAIL;
	}

	matrixBufferDesc.ByteWidth = sizeof(BoneBuffer);
	if (FAILED(device->CreateBuffer(&matrixBufferDesc, NULL, &_boneBuffer))) {
	//	MessageBox(_hWnd, L"Shader Manager Error");

		return E_FAIL;
	}

	matrixBufferDesc.ByteWidth = sizeof(CameraPosBuffer);
	if (FAILED(device->CreateBuffer(&matrixBufferDesc, nullptr, &_cameraPosBuffer)))
		return E_FAIL;

	matrixBufferDesc.ByteWidth = sizeof(LightBuffer);
	if (FAILED(device->CreateBuffer(&matrixBufferDesc, nullptr, &_lightBuffer)))
		return E_FAIL;

	return S_OK;
}

bool RMShader::SetShaderParameters(ID3D11DeviceContext * dc, vector<XMMATRIX>& matBoneList)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;

	//셰이더와 연결된 매트릭스 버퍼 주소를 가져온다.
	auto matBuff = _boneBuffer;

	//버퍼 정보에 접근한다.
	if (FAILED(dc->Map(matBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
		return false;
	}

	//버퍼의 행렬 데이터 위치를 가져온다.
	BoneBuffer* dataPtr = (BoneBuffer*)mappedResource.pData;

	//행렬 정보 넘기기
	for(UINT i = 0; i< matBoneList.size(); i++)
		dataPtr->matBones[i] = matBoneList[i];

	dc->Unmap(matBuff, 0);

	//bufferSturct.h 의 정보와 일치해야 한다.
	bufferNumber = 3;

	dc->VSSetConstantBuffers(bufferNumber, 1, &matBuff);



	return true;
}

bool RMShader::SetShaderParameters(ID3D11DeviceContext * dc, XMMATRIX matWorld)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;

	//셰이더와 연결된 매트릭스 버퍼 주소를 가져온다.
	auto matBuff = _transBuffer;

	//버퍼 정보에 접근한다.
	if (FAILED(dc->Map(matBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
		return false;
	}

	//버퍼의 행렬 데이터 위치를 가져온다.
	TransBuffer* dataPtr = (TransBuffer*)mappedResource.pData;

	//행렬 정보 넘기기
	dataPtr->matWorld = XMMatrixTranspose(matWorld);
	
	dc->Unmap(matBuff, 0);
	
	//bufferSturct.h 의 정보와 일치해야 한다.
	bufferNumber = 1;

	dc->VSSetConstantBuffers(bufferNumber, 1, &matBuff);

	

	return true;
}

bool RMShader::SetShaderParameters(ID3D11DeviceContext * dc, XMMATRIX matView, XMMATRIX matProj)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber =0;

	//셰이더와 연결된 매트릭스 버퍼 주소를 가져온다.
	auto matBuff = _cameraBuffer;

	//버퍼 정보에 접근한다.
	if (FAILED(dc->Map(matBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
		return false;
	}

	//버퍼의 행렬 데이터 위치를 가져온다.
	CameraBuffer* dataPtr = (CameraBuffer*)mappedResource.pData;

	//행렬 정보 넘기기
	dataPtr->matView = XMMatrixTranspose(matView);
	dataPtr->matProj = XMMatrixTranspose(matProj);

	dc->Unmap(matBuff, 0);

	dc->VSSetConstantBuffers(bufferNumber, 1, &matBuff);

	return false;
}

bool RMShader::SetShaderParameters(ID3D11DeviceContext * dc, XMFLOAT4 color)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;

	//셰이더와 연결된 매트릭스 버퍼 주소를 가져온다.
	auto buff = _colorBuffer;

	//버퍼 정보에 접근한다.
	if (FAILED(dc->Map(buff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
		return false;
	}

	//버퍼의 행렬 데이터 위치를 가져온다.
	ColorBuffer* dataPtr = (ColorBuffer*)mappedResource.pData;

	//행렬 정보 넘기기
	dataPtr->fColor = color;
	dc->Unmap(buff, 0);

	bufferNumber = 2;

	dc->VSSetConstantBuffers(bufferNumber, 1, &buff);

	return true;
}

bool RMShader::SetShaderParameters(
	ID3D11DeviceContext* dc,
	LightClass* light,
	const XMFLOAT3& cameraPos,
	const PointLightParam* pointLights,
	int numPointLights)
{
	D3D11_MAPPED_SUBRESOURCE mapped = {};

	// ===== CameraPosBuffer → b4 (VS & PS) =====
	{
		auto buff = _cameraPosBuffer;
		if (FAILED(dc->Map(buff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
			return false;

		auto data = reinterpret_cast<CameraPosBuffer*>(mapped.pData);
		data->cameraPosition = cameraPos;
		data->pad1 = 0.0f;

		dc->Unmap(buff, 0);

		UINT slot = 4;
		dc->VSSetConstantBuffers(slot, 1, &buff);
		dc->PSSetConstantBuffers(slot, 1, &buff);
	}

	// ===== LightBuffer → b5 (PS) =====
	{
		auto buff = _lightBuffer;
		if (FAILED(dc->Map(buff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
			return false;

		auto data = reinterpret_cast<LightBuffer*>(mapped.pData);

		// 방향성 라이트
		data->ambientColor = light->GetAmbientColor();
		data->dirDiffuseColor = light->GetDiffuseColor();
		data->lightDirection = light->GetDirection();
		data->specularPower = light->GetSpecularPower();
		data->specularColor = light->GetSpecularColor();

		// 포인트라이트 배열 채우기
		for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
		{
			if (pointLights && i < numPointLights)
			{
				data->pointLights[i] = pointLights[i];
			}
			else
			{
				data->pointLights[i].position = XMFLOAT3(0.f, 0.f, 0.f);
				data->pointLights[i].range = 0.f;
				data->pointLights[i].color = XMFLOAT4(0.f, 0.f, 0.f, 0.f);
			}
		}

		dc->Unmap(buff, 0);

		UINT slot = 5;
		dc->PSSetConstantBuffers(slot, 1, &buff);
	}

	return true;
}





Shader* RMShader::loadResource(wstring fileName, void * param)
{
	wstring vsFilename = fileName + L".hlsl";

	Shader* shader = new Shader;
	if (!shader->InsertShader(DEVICEMANAGER.GetDevice(), _hWnd, (WCHAR*)vsFilename.c_str())) {
		SAFE_DELETE(shader);
	}
	
	return shader;
}


void RMShader::releaseResource(Shader* data)
{
	SAFE_DELETE(data);
}

