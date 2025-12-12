#include "pch.h"
#include "SkinModel.h"

SkinModel::SkinModel()
{\
	_texShader	= RM_SHADER.AddResource(L"./data/textureShader");
	_skinShader = RM_SHADER.AddResource(L"./data/skinShader");
	SetShader(_skinShader);
}

SkinModel::~SkinModel()
{
}

void SkinModel::Release()
{
	for (auto item : _meshList)
		SAFE_DELETE(item);

	for (auto item : _nodeList)
		SAFE_DELETE(item);

	vector<NodeInfo*>().swap(_nodeList);
	vector<HierarchyMesh*>().swap(_meshList);

	BaseModel::Release();
}


void SkinModel::Render(ID3D11DeviceContext * dc)
{
	//메쉬 정보가 없으면 리턴
	if (_meshList.empty()) 	return;

	//렌더링을 위한 버퍼셋팅
	SetRenderBuffers(dc);
		
	//ModelTM Update
	UpdateModelMatrix();

	//Node Update
	UpdateNodeTM();

	for (UINT i = 0; i < _meshByMaterial.size(); i++) {
		if (_meshByMaterial[i].empty())
			continue;

		//마테리얼 셋팅
		auto texData = _materialList[i].diffuseMap;

		if (texData)
			dc->PSSetShaderResources(0, 1, &texData->data);

		//Mesh Render
		for (auto mesh : _meshByMaterial[i]) {
			
			//본 정보 셋팅
			if (!mesh->boneList.empty()) {
				SetShader(_skinShader, false);
				vector<XMMATRIX> calcBoneList;
				for (auto boneInfo : mesh->boneList)
					calcBoneList.emplace_back(XMMatrixTranspose(boneInfo.matOffset * boneInfo.linkNode->worldTM));

				RM_SHADER.SetShaderParameters(dc, calcBoneList);
			}
			//본 정보가 없는 경우
			else {
				SetShader(_texShader, false);

				if (mesh->linkNode)
					_tmModel = mesh->linkNode->worldTM * _tmModel;
			}
			RM_SHADER.SetShaderParameters(dc, _tmModel);

			//렌더
			GetShader()->IndexRender(dc, mesh->count, mesh->start);
		}
	}

	//Unbind Texture
	ID3D11ShaderResourceView* nullViews[] = { nullptr };
	dc->PSSetShaderResources(0, 1, nullViews);
}


void SkinModel::PlayAni(int idx)
{
	if (_playAniIdx != idx) {
		_aniList[idx].Stop();
		_playAniIdx = idx;
	}

	_aniList[idx].Play();
}


void SkinModel::UpdateMeshByMaterial()
{

	_meshByMaterial.clear();
	_meshByMaterial.resize(_materialList.size());


	for (auto& mesh : _meshList) {
		_meshByMaterial[mesh->matIdx].push_back(mesh);
	}
	
}


void SkinModel::UpdateNodeTM()
{
	if (_aniList.empty())
		return;


	if(_playAniIdx >= 0)
		_aniList[_playAniIdx].UpdateAnimation(TIMEMANAGER.GetElapedTime());

	for (auto& nodeItem : _nodeList) {
		XMMATRIX tm = nodeItem->localTM;
		
		if (_playAniIdx >= 0)
			_aniList[_playAniIdx].GetAniTM(nodeItem->name, tm);

		nodeItem->worldTM = tm;
		if (nodeItem->parent)
			nodeItem->worldTM = tm * nodeItem->parent->worldTM;
	}
}

bool SkinModel::LoadTexture(const WCHAR* filename)
{
	bool result;


	// Create the texture object.
	m_Texture = new TextureClass;
	if (!m_Texture)
	{
		return false;
	}

	// Initialize the texture object.
	result = m_Texture->Initialize(DEVICEMANAGER.GetDevice(), filename);
	if (!result)
	{
		return false;
	}

	return true;
}

ID3D11ShaderResourceView* SkinModel::GetTexture()
{
	return m_Texture->GetTexture();
}

int SkinModel::GetFaceCount() const
{
	int totalIndexCount = 0;

	for (auto mesh : _meshList) {
		if (!mesh) continue;
		totalIndexCount += mesh->count;   // HierarchyMesh 의 count
	}

	// Triangulate 되어 있으니 3개 인덱스 = 삼각형 1개
	return totalIndexCount / 3;
}

XMMATRIX SkinModel::GetNodeWorldTM(const std::wstring& nodeName) const
{
	// 기본값: 모델 월드 행렬 (못 찾았을 때)
	XMMATRIX result = _tmModel;

	for (auto nodeItem : _nodeList) {
		if (!nodeItem) continue;

		// nodeItem->name 이 std::wstring 이라고 가정
		if (nodeItem->name == nodeName) {
			// nodeItem->worldTM : 모델 기준
			// _tmModel          : 모델 -> 월드
			result = nodeItem->worldTM * _tmModel;
			break;
		}
	}

	return result;
}