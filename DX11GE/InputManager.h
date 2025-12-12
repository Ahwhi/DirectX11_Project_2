#pragma once
#include <dinput.h>
#include <directxmath.h>
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
using namespace DirectX;

#pragma once
#include "SingletonBase.h"
class InputManager : public SingletonBase <InputManager>
{
public:
	HRESULT Init();
	void release(void);
	void DetectInput();
	
	float GetHorizontal() { return m_Horizontal; }
	float GetVertical() { return m_Vertical; }
	float GetShift() { return m_Shift; }
	float GetControl() { return m_Control; }
	float GetNum1() { return m_Num1; }

	float GetYaw() { return m_Yaw; }
	float GetPitch() { return m_Pitch; }

	InputManager() {}
	~InputManager() {}

	void SetYaw(float yaw) { m_Yaw = yaw; }
	void SetPitch(float pit) { m_Pitch = pit; }

private:
	bool InitDirectInput(HINSTANCE hInstance, HWND hwnd);
	LPDIRECTINPUT8 DirectInput;
	IDirectInputDevice8* DIKeyboard;
	IDirectInputDevice8* DIMouse;

	float m_Horizontal;
	float m_Vertical;
	float m_Shift;
	float m_Control;
	float m_Num1;

	DIMOUSESTATE mouseLastState;

	float m_Yaw;
	float m_Pitch;
	
};

