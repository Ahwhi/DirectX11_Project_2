//==================================================================
//		## MainGame ## (Process Game)
//==================================================================

#pragma once
#include "GameNode.h"
#include "textclass.h"

class MainGame : public GameNode
{
public:
	HRESULT Init(ID3D11DeviceContext* dc);
	HRESULT	InitScene();

	void Update(void);
	void Render(ID3D11DeviceContext* dc);

	MainGame() {}
	~MainGame() {}

private:
	ID3D11DeviceContext* m_deviceContext;
};
