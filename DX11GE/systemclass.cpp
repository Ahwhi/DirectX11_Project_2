////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "systemclass.h"

SystemClass::SystemClass()
{

}


SystemClass::SystemClass(const SystemClass& other)
{
}


SystemClass::~SystemClass()
{
}


bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;

	// Initialize the width and height of the screen to zero before sending the variables into the function.
	screenWidth = 0;
	screenHeight = 0;

	// Initialize the windows api.
	InitializeWindows(screenWidth, screenHeight);

	_hWnd = this->m_hwnd;
	_hInstance = this->m_hinstance;

	if (FAILED(InitManager()))
		return false;

	DEVICEMANAGER.SetDeferredContext(&_mainDC);

	//create mainGame Object
	_mg = make_unique<MainGame>();
	_mg->Init(_mainDC);
	

	return true;
}


void SystemClass::Shutdown()
{
	SAFE_RELEASE(_mainDC);

	//Manager Release
	SCENEMANAGER.Release();
	RM_SHADER.ClearResource();
	RM_MODEL.ClearResource();
	RM_TEXTURE.ClearResource();

	// Shutdown the window.
	ShutdownWindows();
	
	return;
}


void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));
	
	// Loop until there is a quit message from the window or the user.
	done = false;
	while(!done)
	{
		// Handle the windows messages.
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			result = Frame();
			if(!result)
			{
				done = true;
			}
		}

	}

	return;
}


bool SystemClass::Frame()
{
	INPUTMANAGER.DetectInput();

	float duration = TIMEMANAGER.GetElapedTime();

	// 회전은 여기서 한 번만 세팅
	_mainCam.SetRotation(
		INPUTMANAGER.GetPitch(),
		INPUTMANAGER.GetYaw(),
		0.0f);

	_mainCam.KeyboardInput(duration);

	if (!_mg) return false;
	_mg->Update();
	_mg->Render(_mainDC);

	return true;
}


LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
//	switch(umsg)
//	{
		// Check if a key has been pressed on the keyboard.
		//case WM_KEYDOWN:
		//{
		//	// If a key is pressed send it to the input object so it can record that state.
		//	m_Input->KeyDown((unsigned int)wparam);
		//	return 0;
		//}

		//// Check if a key has been released on the keyboard.
		//case WM_KEYUP:
		//{
		//	// If a key is released then send it to the input object so it can unset the state for that key.
		//	m_Input->KeyUp((unsigned int)wparam);
		//	return 0;
		//}

		// Any other messages send to the default message handler as our application won't make use of them.
//		default:
//		{
			return DefWindowProc(hwnd, umsg, wparam, lparam);
//		}
//	}
}


void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// Get an external pointer to this object.	
	ApplicationHandle = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_applicationName = L"B777012";

	// Setup the windows class with default settings.
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);
	
	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if(FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screenWidth  = WINSIZEX;
		screenHeight = WINSIZEY;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth)  / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, 
						    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
						    posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	ShowCursor(false);

	return;
}


void SystemClass::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if(FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}

HRESULT SystemClass::InitManager()
{
	if (FAILED(DEVICEMANAGER.CreateDevice(WINSIZEX, WINSIZEY))) {
		MessageBox(m_hwnd,L"Failed Init Device!",L"이 프로그램이 종료됩니다", MB_ICONQUESTION|MB_OKCANCEL);
		return E_FAIL;
	}

	if (FAILED(INPUTMANAGER.Init())) {
		MessageBox(m_hwnd, L"Failed Init INPUTMANAGER!", L"이 프로그램이 종료됩니다", MB_ICONQUESTION | MB_OKCANCEL);
		return E_FAIL;
	}

	if (FAILED(TIMEMANAGER.Init())) {
		MessageBox(m_hwnd, L"Failed Init TIME_MANAGER!", L"이 프로그램이 종료됩니다", MB_ICONQUESTION | MB_OKCANCEL);
	}

	if (FAILED(SCENEMANAGER.Init())) {
		MessageBox(m_hwnd, L"Failed Init SCENE_MANAGER!", L"이 프로그램이 종료됩니다", MB_ICONQUESTION | MB_OKCANCEL);
		return E_FAIL;
	}

	ID3D11Device* device = DEVICEMANAGER.GetDevice();

	if (FAILED(RM_SHADER.Init(device))) {
		MessageBox(m_hwnd, L"Failed Init RM_SHADER!", L"이 프로그램이 종료됩니다", MB_ICONQUESTION | MB_OKCANCEL);
		return E_FAIL;
	}

	/*if (FAILED(FONTMANAGER.Init())) {
		AfxMessageBox(L"Failed Init FONT_MANAGER!");
		return E_FAIL;
	}*/

	return S_OK;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);		
			return 0;
		}

		// All other messages pass to the message handler in the system class.
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}