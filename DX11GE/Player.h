#pragma once
#include "ModelLoader.h"
#include "SkinModel.h"

class Player : public AlignedAllocationPolicy<16>
{
public:
	Player();
	~Player() {}
	//Camera Update
	void Update(float duration);
	SkinModel* skinModel;

	XMVECTOR GetLookAt() { return _lookAtVec; }

private:
	ModelLoader* modelLoader;
	

	//이동, 회전 속도
	float _speed, _mouseSpeed;

	//위치, 회전 값
	XMFLOAT3	_position, _rotation;

	XMVECTOR _lookAtVec;		//direction

	//Set Function
	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	//Get Function
	XMVECTOR GetPosition();
	XMVECTOR GetRotation();
	XMFLOAT3 GetPositionF() { return _position; }

	void MouseInput();
	void KeyboardInput(float duration);

};

