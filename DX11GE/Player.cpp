#include "Player.h"

Player::Player()
{
	_position = XMFLOAT3(0.f, 0.f, 0.f);
	_rotation = XMFLOAT3(0.f, 0.f, 0.f);

	_speed = 10.f;
	_mouseSpeed = 10.f;

//	modelLoader = new ModelLoader;
//	skinModel = new SkinModel;

//	skinModel = modelLoader->LoadModel(L"./data/character.fbx", aiProcess_Triangulate |	aiProcess_ConvertToLeftHanded);
//	modelLoader->LoadAnimation(L"./data/Idle_24.fbx", skinModel, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
//	modelLoader->LoadAnimation(L"./data/anim_joyfuljump.fbx", skinModel, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
//	modelLoader->LoadAnimation(L"./data/Running30.fbx", skinModel, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
//	modelLoader->LoadAnimation(L"./data/Walking30.fbx", skinModel, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
//	modelLoader->LoadAnimation(L"./data/01Attack.fbx", skinModel, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

//	skinModel->GetAnimation(5).SetRepeat(true);
//	skinModel->LoadTexture(L"./data/character_d.dds");
//	skinModel->SetNormalize(true);
}

void Player::SetPosition(float x, float y, float z)
{
	//player 객체 위치
	_position.x = x;
	_position.y = y;
	_position.z = z;
	//player 메시 위치
	skinModel->SetCenter(_position);
	return;
}

void Player::SetRotation(float x, float y, float z) 
{
	_rotation.x = x;
	_rotation.y = y;
	_rotation.z = z;

	skinModel->SetRotate(_rotation);

	return;
}

XMVECTOR Player::GetPosition()
{
	return XMLoadFloat3(&_position);
}

XMVECTOR Player::GetRotation()
{
	return XMLoadFloat3(&_rotation);
}

void Player::Update(float duration)
{
	float yaw, pitch, roll;
	XMFLOAT3 up, lookAt;
	XMMATRIX rotationMatrix;

	up = XMFLOAT3(0.f, 1.f, 0.f);
	lookAt = XMFLOAT3(0.f, 0.f, 1.f);

	pitch = _rotation.x * 0.0174532925f;
	yaw = _rotation.y * 0.0174532925f;
	roll = _rotation.z * 0.0174532925f;

	//회전 값으로 카메라 회전 행렬 만듬
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	//카메라 up 벡터, LookAtVector 벡터 회전
	XMVECTOR upVec = XMVector3TransformCoord(XMLoadFloat3(&up), rotationMatrix);
	XMVECTOR lookAtVec = XMVector3TransformCoord(XMLoadFloat3(&lookAt), rotationMatrix);
	_lookAtVec = lookAtVec;

	//LookAtVector에 이동벡터에 대한 값 적용
	lookAtVec = XMLoadFloat3(&_position) + lookAtVec;
}

void Player::MouseInput()
{
	_rotation.x = INPUTMANAGER.GetPitch();
}

void Player::KeyboardInput(float duration)
{
	//위치벡터, 카메라 방향벡터, 업 벡터
	XMVECTOR posVec = XMLoadFloat3(&_position);
	XMVECTOR moveVec = XMVector3Normalize(_lookAtVec);
	XMVECTOR upVec = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	if (INPUTMANAGER.GetVertical()==1.f) { //상
		posVec += (moveVec * duration * _speed);
	} 
	else if (INPUTMANAGER.GetVertical()==-1.f) { //하
		posVec -= (moveVec * duration * _speed);
	}

	if (INPUTMANAGER.GetHorizontal()==-1.f) { //좌
		posVec -= XMVector3Cross(upVec, moveVec) * duration *  _speed;
	}
	else if (INPUTMANAGER.GetHorizontal() == 1.f) { //우
		posVec += XMVector3Cross(upVec, moveVec) * duration * _speed;
	}

	/*if (KEYMANAGER.IsStayKeyDown('Q')) {
		posVec -= (upVec * duration * _speed);
	}
	else if (KEYMANAGER.IsStayKeyDown('E')) {
		posVec += (upVec * duration * _speed);
	}*/

	//이동에 따른 위치 갱신
	XMStoreFloat3(&_position, posVec);
}
