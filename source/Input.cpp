#include "pch.h"
#include "Input.h"


Input::Input()
{
}

Input::~Input()
{
}

void Input::Initialise(HWND window)
{
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow(window);
	m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
	m_quitApp = false;

	m_GameInput.forward		= false;
	m_GameInput.back		= false;
	m_GameInput.right		= false;
	m_GameInput.left		= false;
	m_GameInput.up          = false;
	m_GameInput.down		= false;

	m_GameInput.rotation	= DirectX::SimpleMath::Vector2::Zero;
}

void Input::Update()
{
	auto kb = m_keyboard->GetState();	//updates the basic keyboard state
	m_KeyboardTracker.Update(kb);		//updates the more feature filled state. Press / release etc. 
	auto mouse = m_mouse->GetState();   //updates the basic mouse state
	m_MouseTracker.Update(mouse);		//updates the more advanced mouse state. 

	if (kb.Escape)// check has escape been pressed.  if so, quit out. 
	{
		m_quitApp = true;
	}

	m_GameInput.forward		= kb.W;
	m_GameInput.back		= kb.S;
	m_GameInput.right		= kb.D;
	m_GameInput.left		= kb.A;
	m_GameInput.up			= kb.Space;
	m_GameInput.down		= kb.LeftShift;

	m_GameInput.rotation	= DirectX::SimpleMath::Vector2(mouse.x, mouse.y);
}

bool Input::Quit()
{
	return m_quitApp;
}

InputCommands Input::getGameInput()
{
	return m_GameInput;
}
