//
// Game.cpp
//

#include "pch.h"
#include "Game.h"


//toreorganise
#include <fstream>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	m_input.Initialise(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	//setup light
	m_Ambience = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
	//m_Ambience = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light.setAmbientColour(m_Ambience.x, m_Ambience.y, m_Ambience.z, m_Ambience.w);
	m_Light.setDiffuseColour(0.93f, 1.0f, 0.98f, 1.0f);
	m_Light.setPosition(-1.0f, -1.0f, -1.0f);
	m_Light.setDirection(1.0f, 1.0f, 0.0f);
	m_Light.setStrength(10.0);

	//setup camera
	//m_Camera.setPosition(Vector3(2.4f+0.75*cos(atan(-1.8/2.4)), 0.0f, 1.8f+0.75*sin(atan(-1.8/2.4))));
	//m_Camera.setRotation(Vector3(-90.0f, -180+(180.0/3.14159265)*atan(2.4/1.8), 0.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up.
	m_Camera.setPosition(Vector3(0.0, 0.0f, 10.0));
	m_Camera.setRotation(Vector3(-90.0f, -180, 0.0f));
	
#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	//take in input
	m_input.Update();								//update the hardware
	m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game
	
	//Update all game objects
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

	//Render all game content. 
    Render();

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

	
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	m_time = m_timer.GetTotalSeconds();

	//note that currently.  Delta-time is not considered in the game object movement. 
	Vector3 inputPosition = Vector3(0.0f, 0.0f, 0.0f);

	// STEP 1: Read camera translation inputs (from keyboard)
	if (m_gameInputCommands.forward)
		inputPosition.z += 1.0f;
	if (m_gameInputCommands.back)
		inputPosition.z -= 1.0f;
	if (m_gameInputCommands.right)
		inputPosition.x += 1.0f;
	if (m_gameInputCommands.left)
		inputPosition.x -= 1.0f;
	if (m_gameInputCommands.up)
		inputPosition.y += 1.0f;
	if (m_gameInputCommands.down)
		inputPosition.y -= 1.0f;

	if (inputPosition.x != 0.0f || inputPosition.y != 0.0f || inputPosition.z != 0.0f)
	{
		inputPosition.Normalize();
		inputPosition.x *= 0.8f;
		inputPosition.y *= 0.8f;

		// NB: forward/right directions are relative to the camera, but up remains relative to the space to avoid confusion...
		Vector3 deltaPosition = inputPosition.z*m_Camera.getForward() + inputPosition.x*m_Camera.getRight() + inputPosition.y*Vector3::UnitY;
		deltaPosition *= m_timer.GetElapsedSeconds()*m_Camera.getMoveSpeed();

		Vector3 position = m_Camera.getPosition()+deltaPosition;
		m_Camera.setPosition(position);
	}

	// STEP 2: Read camera rotation inputs (from mouse)
	Vector2 inputRotation = m_gameInputCommands.rotation;
	if (inputRotation.x != 0.0f || inputRotation.y != 0.0f)
	{
		Vector3 rotation = m_Camera.getRotation();
		inputRotation.x *= sin(XM_PIDIV4+0.5*XM_PI*(-rotation.x/180.0f));

		Vector3 deltaRotation = (-inputRotation.y)*Vector3::UnitX + (-inputRotation.x)*Vector3::UnitY;
		deltaRotation *= m_timer.GetElapsedSeconds()*m_Camera.getRotationSpeed();

		rotation += deltaRotation;
		rotation.x = std::min(-0.001f, std::max(rotation.x, -179.999f)); // NB: Prevents gimbal lock
		m_Camera.setRotation(rotation);
	}

	// STEP 3: Process inputs
	m_Camera.Update();
	m_Light.setPosition(m_Camera.getPosition().x, m_Camera.getPosition().y, m_Camera.getPosition().z);
	UpdateModels(m_time);

	m_view = m_Camera.getCameraMatrix();
	m_projection = m_Camera.getPerspective();
	m_world = Matrix::Identity;

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

  
	if (m_input.Quit())
	{
		ExitGame();
	}
}

void Game::UpdateModels(float time)
{

}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{	
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

    // Draw Text to the screen
    m_sprites->Begin();
		m_font->DrawString(m_sprites.get(), L"", XMFLOAT2(10, 10), Colors::White);
    m_sprites->End();

	//Set Rendering states. 
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullClockwise());
//	context->RSSetState(m_states->Wireframe());

	// If m_time == 0.0, then render all static textures (once only!)
	if (!m_preRendered)
	{
		RenderStaticTextures();

		RenderStaticSpecimenEnvironments();
		RenderStaticLiquidEnvironments();

		RenderStaticEnvironments(); // FIXME: Add depth mapping!
		RenderStaticReflectionEnvironments();

		m_preRendered = true;
	}

	// STEP 1: Run render to textures...
	// First render pass: Rendering any textures (including normal maps, etc...)
	RenderDynamicTextures();

	// Second render pass: Rendering...
	RenderDynamicSpecimenEnvironments();
	RenderDynamicLiquidEnvironments();

	RenderDynamicEnvironment(); // FIXME: Add depth mapping!
	RenderDynamicExternalEnvironments();
	RenderDynamicAirToGlassEnvironments();
	RenderDynamicInternalEnvironments();


	// STEP 2: Render 'real' scene...
	// Draw Skybox
	RenderSkyboxOnto(&m_Camera);

	// Draw Basic Models
	for (int i = 0; i < m_BasicCount; i++)
		RenderBasicsOnto(&m_Camera, &m_Light, i);

	// Draw Glass Models
	for (int i = 0; i < m_GlassCount; i++)
		RenderGlassOnto(&m_Camera, &m_Light, i);

    // Show the new frame.
    m_deviceResources->Present();
}





// Rendering Models
void Game::RenderBasicsOnto(Camera* camera, Light* light, int i)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	m_LightShaderPair.EnableShader(context);
	m_LightShaderPair.SetLightShaderParameters(context, &m_BasicModelTransforms[i], &camera->getCameraMatrix(), &camera->getPerspective(), m_time, light, (*m_BasicModelTextures[i])->getShaderResourceView(), (*m_BasicModelNMTextures[i])->getShaderResourceView());
	(*m_BasicModels[i]).Render(context);
}

void Game::RenderSpecimensOnto(Camera* camera, Light* light, int i)
{
	if (i > 0)
		return;

	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	float theta = XM_PIDIV2 * m_time;
	Vector3 axis = SimpleMath::Vector3(sin(XM_PI / 32) * sin(0.009 * theta), cos(XM_PI / 32), sin(XM_PI / 16) * cos(0.009 * theta));
	Matrix spin = SimpleMath::Matrix::CreateRotationY(XM_PIDIV2) * SimpleMath::Matrix::CreateFromAxisAngle(axis, 0.018f * theta);
	Vector3 translation = Vector3(0.03* sin(0.07 * m_time), 0.0, 0.03 * cos(0.07 * m_time))+Vector3(0.0, 0.1*sin(0.5 * m_time), 0.0);

	m_LightShaderPair.EnableShader(context);
	m_LightShaderPair.SetLightShaderParameters(context, &(Matrix::CreateTranslation(translation) * spin * Matrix::CreateScale(0.6f)* m_GlassModelTransforms[i]), &camera->getCameraMatrix(), &camera->getPerspective(), m_time, light, m_DemoRenderPass->getShaderResourceView(), m_DemoNMRenderPass->getShaderResourceView());
	m_Cube.Render(context);
}

void Game::RenderLiquidsOnto(Camera* camera, Light* light, int i, ID3D11ShaderResourceView* specimen)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	float theta = XM_PIDIV2 * m_time;
	Vector3 axis = SimpleMath::Vector3(sin(XM_PI / 16) * sin(0.009 * theta), cos(XM_PI / 16), sin(XM_PI / 16) * cos(0.009 * theta));
	Matrix spin = SimpleMath::Matrix::CreateRotationY(XM_PIDIV2) * SimpleMath::Matrix::CreateFromAxisAngle(axis, 0.018f * theta);

	m_SpecimenShaderPair.EnableShader(context);
	m_SpecimenShaderPair.SetSpecimenShaderParameters(context, &(spin*Matrix::CreateScale(0.8f)*m_GlassModelTransforms[i]), &camera->getCameraMatrix(), &camera->getPerspective(), m_time, light, m_LiquidOpacity[i], m_brineTexture.Get(), m_NeutralNMRenderPass->getShaderResourceView(), specimen);
	m_Sphere.Render(context);
}



void Game::RenderSpecimenAlphasOnto(Camera* camera, int i, ID3D11ShaderResourceView* alpha)
{
	if (i > 0)
		return;

	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	float theta = XM_PIDIV2 * m_time;
	Vector3 axis = SimpleMath::Vector3(sin(XM_PI / 32) * sin(0.009 * theta), cos(XM_PI / 32), sin(XM_PI / 16) * cos(0.009 * theta));
	Matrix spin = SimpleMath::Matrix::CreateRotationY(XM_PIDIV2) * SimpleMath::Matrix::CreateFromAxisAngle(axis, 0.018f * theta);
	Vector3 translation = Vector3(0.03 * sin(0.07 * m_time), 0.0, 0.03 * cos(0.07 * m_time)) + Vector3(0.0, 0.1 * sin(0.5 * m_time), 0.0);

	m_AlphaShaderPair.EnableShader(context);
	m_AlphaShaderPair.SetAlphaShaderParameters(context, &(Matrix::CreateTranslation(translation) * spin * Matrix::CreateScale(0.6f) * m_GlassModelTransforms[i]), &camera->getCameraMatrix(), &camera->getPerspective(), m_time, 1.0, alpha);
	m_Cube.Render(context);
}

void Game::RenderLiquidAlphasOnto(Camera* camera, int i, ID3D11ShaderResourceView* alpha)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	float theta = XM_PIDIV2 * m_time;
	Vector3 axis = SimpleMath::Vector3(sin(XM_PI / 16) * sin(0.009 * theta), cos(XM_PI / 16), sin(XM_PI / 16) * cos(0.009 * theta));
	Matrix spin = SimpleMath::Matrix::CreateRotationY(XM_PIDIV2) * SimpleMath::Matrix::CreateFromAxisAngle(axis, 0.018f * theta);

	m_AlphaShaderPair.EnableShader(context);
	m_AlphaShaderPair.SetAlphaShaderParameters(context, &(spin*Matrix::CreateScale(0.8f)*m_GlassModelTransforms[i]), &camera->getCameraMatrix(), &camera->getPerspective(), m_time, m_LiquidOpacity[i], alpha);
	m_Sphere.Render(context);
}



void Game::RenderRefractionOnto(Camera* camera, Light* light, int i)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	ID3D11ShaderResourceView* environmentMap[6];
	for (int j = 0; j < 6; j++)
		environmentMap[j] = m_DynamicExternalEnvironments[i][j]->getShaderResourceView();

	context->RSSetState(m_states->CullCounterClockwise());
	m_RefractionShaderPair.EnableShader(context);
	m_RefractionShaderPair.SetRefractionShaderParameters(context, &m_GlassModelTransforms[i], &camera->getCameraMatrix(), &camera->getPerspective(), m_time, light, m_GlassModelOpacity[i], m_GlassModelRefractiveIndex[i], false, camera, m_glassTexture.Get(), m_NeutralNMRenderPass->getShaderResourceView(), environmentMap);
	(*m_GlassModels[i]).Render(context);

	context->RSSetState(m_states->CullClockwise());
}

void Game::RenderGlassOverlayOnto(Camera* camera, int i, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* overlay, ID3D11ShaderResourceView* alpha)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	// FIXME: Add depth mapping!
	m_OverlayShaderPair.EnableShader(context);
	m_OverlayShaderPair.SetOverlayShaderParameters(context, &m_GlassModelTransforms[i], &camera->getCameraMatrix(), &camera->getPerspective(), m_time, texture, overlay, alpha); // alpha will slot in here!
	m_Sphere.Render(context);
}

void Game::RenderGlassOnto(Camera* camera, Light* light, int i)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	ID3D11ShaderResourceView* refractionMap[6];
	for (int j = 0; j < 6; j++)
		refractionMap[j] = m_DynamicInternalEnvironments[i][j]->getShaderResourceView();

	ID3D11ShaderResourceView* reflectionMap[6];
	for (int j = 0; j < 6; j++)
		reflectionMap[j] = m_StaticReflectionEnvironments[i][j]->getShaderResourceView();

	m_GlassShaderPair.EnableShader(context);
	m_GlassShaderPair.SetGlassShaderParameters(context, &m_GlassModelTransforms[i], &camera->getCameraMatrix(), &camera->getPerspective(), m_time, light, m_GlassModelOpacity[i], 1.00/m_GlassModelRefractiveIndex[i], true, camera, m_glassTexture.Get(), m_NeutralNMRenderPass->getShaderResourceView(), refractionMap, reflectionMap);
	(*m_GlassModels[i]).Render(context);
}

void Game::RenderSkyboxOnto(Camera* camera)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	ID3D11ShaderResourceView* environmentMap[6];
	for (int j = 0; j < 6; j++)
		//environmentMap[j] = m_StaticSpecimenEnvironments[0][j][1]->getShaderResourceView();
		//environmentMap[j] = m_StaticSpecimenAlphaEnvironments[0][j][1]->getShaderResourceView();
		//environmentMap[j] = m_StaticLiquidEnvironments[0][j][1]->getShaderResourceView();
		//environmentMap[j] = m_StaticLiquidAlphaEnvironments[0][j][1]->getShaderResourceView();
		//environmentMap[j] = m_StaticEnvironments[1][j]->getShaderResourceView();
		//environmentMap[j] = m_StaticReflectionEnvironments[0][j]->getShaderResourceView();
		//environmentMap[j] = m_DynamicSpecimenEnvironments[0][j]->getShaderResourceView();
		//environmentMap[j] = m_DynamicSpecimenAlphaEnvironments[0][j]->getShaderResourceView();
		//environmentMap[j] = m_DynamicLiquidEnvironments[0][j]->getShaderResourceView();
		//environmentMap[j] = m_DynamicLiquidAlphaEnvironments[0][j]->getShaderResourceView();
		//environmentMap[j] = m_DynamicExternalEnvironments[0][j]->getShaderResourceView();
		//environmentMap[j] = m_DynamicAirToGlassEnvironments[0][j]->getShaderResourceView();
		//environmentMap[j] = m_DynamicInternalEnvironments[0][j]->getShaderResourceView();
		environmentMap[j] = m_SkyboxRenderPass[j]->getShaderResourceView();
		
	context->OMSetDepthStencilState(m_states->DepthNone(), 0); // NB: Note use of DepthNone()
	context->RSSetState(m_states->CullCounterClockwise());
	m_SkyboxShaderPair.EnableShader(context);
	m_SkyboxShaderPair.SetSkyboxShaderParameters(context, &Matrix::CreateTranslation(camera->getPosition()), &camera->getCameraMatrix(), &camera->getPerspective(), m_time, environmentMap); // FIXME: Flat normal map here... but holes when viewed through glass??
	m_Cube.Render(context);

	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullClockwise());
}





// Render Passes
void Game::RenderStaticTextures()
{
	RenderShaderTexture(m_NeutralRenderPass, m_NeutralRendering);
	RenderShaderTexture(m_NeutralNMRenderPass, m_NeutralNMRendering);

	for (int i = 0; i < 6; i++)
		RenderShaderTexture(m_SkyboxRenderPass[i], m_SkyboxRendering[i]);

	//Rendered as a failsafe!
	RenderDynamicTextures();
}

void Game::RenderDynamicTextures()
{
	//RenderShaderTexture(m_NeutralRenderPass, m_NeutralRendering);
	//RenderShaderTexture(m_NeutralNMRenderPass, m_NeutralNMRendering);

	RenderShaderTexture(m_DemoRenderPass, m_DemoRendering);
	RenderShaderTexture(m_DemoNMRenderPass, m_DemoNMRendering);
	RenderShaderTexture(m_SphericalPoresRenderPass, m_SphericalPoresRendering);
	RenderShaderTexture(m_SphericalPoresNMRenderPass, m_SphericalPoresNMRendering);
}

void Game::RenderShaderTexture(RenderTexture* renderPass, Shader rendering)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	renderPass->setRenderTarget(context);
	renderPass->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);
	rendering.EnableShader(context);
	rendering.SetShaderParameters(
		context,
		&SimpleMath::Matrix::CreateScale(2.0f),
		&(Matrix)Matrix::Identity,
		&(Matrix)Matrix::Identity,
		m_time);
	m_Cube.Render(context);
	context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
}



void Game::RenderStaticSpecimenEnvironments()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	// Render specimens from each glass model's perspective, using each of our six environment cameras
	for (int i = 0; i < m_GlassCount; i++)
	{
		m_environmentCamera.setPosition(m_GlassModelPositions[i]);
		m_environmentCamera.Update();

		// Not a great fix, but better than replicating dynamic lighting!
		m_Light.setPosition(m_GlassModelPositions[i].x, m_GlassModelPositions[i].y, m_GlassModelPositions[i].z);

		for (int j = 0; j < 6; j++)
		{
			for (int k = 0; k < m_GlassCount; k++)
			{
				if (i == k)
					continue;

				if (m_environmentCamera.getCamera(j)->getReflection())
					context->RSSetState(m_states->CullCounterClockwise());

				m_StaticSpecimenEnvironments[i][j][k]->setRenderTarget(context);
				m_StaticSpecimenEnvironments[i][j][k]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);
				RenderSpecimensOnto(m_environmentCamera.getCamera(j), &m_Light, k);

				m_StaticSpecimenAlphaEnvironments[i][j][k]->setRenderTarget(context);
				m_StaticSpecimenAlphaEnvironments[i][j][k]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);
				RenderSpecimenAlphasOnto(m_environmentCamera.getCamera(j), k, m_StaticSpecimenAlphaEnvironments[i][j][k]->getShaderResourceView());

				context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
				if (m_environmentCamera.getCamera(j)->getReflection())
					context->RSSetState(m_states->CullClockwise());
			}
		}
	}
	m_Light.setPosition(m_Camera.getPosition().x, m_Camera.getPosition().y, m_Camera.getPosition().z);

	RenderDynamicSpecimenEnvironments();
}

void Game::RenderStaticLiquidEnvironments()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	// Render specimens from each glass model's perspective, using each of our six environment cameras
	for (int i = 0; i < m_GlassCount; i++)
	{
		m_environmentCamera.setPosition(m_GlassModelPositions[i]);
		m_environmentCamera.Update();

		// Not a great fix, but better than replicating dynamic lighting!
		m_Light.setPosition(m_GlassModelPositions[i].x, m_GlassModelPositions[i].y, m_GlassModelPositions[i].z);

		for (int j = 0; j < 6; j++)
		{
			for (int k = 0; k < m_GlassCount; k++)
			{
				if (i == k)
					continue;

				if (m_environmentCamera.getCamera(j)->getReflection())
					context->RSSetState(m_states->CullCounterClockwise());

				m_StaticLiquidEnvironments[i][j][k]->setRenderTarget(context);
				m_StaticLiquidEnvironments[i][j][k]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);
				RenderLiquidsOnto(m_environmentCamera.getCamera(j), &m_Light, k, m_StaticSpecimenEnvironments[i][j][k]->getShaderResourceView());

				m_StaticLiquidAlphaEnvironments[i][j][k]->setRenderTarget(context);
				m_StaticLiquidAlphaEnvironments[i][j][k]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);
				RenderLiquidAlphasOnto(m_environmentCamera.getCamera(j), k, m_StaticSpecimenAlphaEnvironments[i][j][k]->getShaderResourceView());

				context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
				if (m_environmentCamera.getCamera(j)->getReflection())
					context->RSSetState(m_states->CullClockwise());
			}
		}
	}
	m_Light.setPosition(m_Camera.getPosition().x, m_Camera.getPosition().y, m_Camera.getPosition().z);

	RenderDynamicLiquidEnvironments();
}



void Game::RenderStaticEnvironments()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	// Render specimens from each glass model's perspective, using each of our six environment cameras
	for (int i = 0; i < m_GlassCount; i++)
	{
		m_environmentCamera.setPosition(m_GlassModelPositions[i]);
		m_environmentCamera.Update();

		// Not a great fix, but better than replicating dynamic lighting!
		m_Light.setPosition(m_GlassModelPositions[i].x, m_GlassModelPositions[i].y, m_GlassModelPositions[i].z);

		for (int j = 0; j < 6; j++)
		{
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullCounterClockwise());

			m_StaticEnvironments[i][j]->setRenderTarget(context);
			m_StaticEnvironments[i][j]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);

			RenderSkyboxOnto(m_environmentCamera.getCamera(j));

			for (int k = 0; k < m_BasicCount; k++)
				RenderBasicsOnto(m_environmentCamera.getCamera(j), &m_Light, k);

			context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullClockwise());
		}
	}
	m_Light.setPosition(m_Camera.getPosition().x, m_Camera.getPosition().y, m_Camera.getPosition().z);
}

void Game::RenderStaticReflectionEnvironments()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	// Render specimens from each glass model's perspective, using each of our six environment cameras
	for (int i = 0; i < m_GlassCount; i++)
	{
		m_environmentCamera.setPosition(m_GlassModelPositions[i]);
		m_environmentCamera.Update();

		// Not a great fix, but better than replicating dynamic lighting!
		m_Light.setPosition(m_GlassModelPositions[i].x, m_GlassModelPositions[i].y, m_GlassModelPositions[i].z);

		for (int j = 0; j < 6; j++)
		{
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullCounterClockwise());

			m_StaticReflectionEnvironments[i][j]->setRenderTarget(context);
			m_StaticReflectionEnvironments[i][j]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);

			RenderSkyboxOnto(m_environmentCamera.getCamera(j));

			for (int k = 0; k < m_BasicCount; k++)
				RenderBasicsOnto(m_environmentCamera.getCamera(j), &m_Light, k);

			// Draw PseudoGlass Models
			for (int k = 0; k < m_GlassCount; k++)
			{
				if (i == k)
					continue;

				RenderGlassOverlayOnto(m_environmentCamera.getCamera(j), k, m_StaticEnvironments[i][j]->getShaderResourceView(), m_StaticLiquidEnvironments[i][j][k]->getShaderResourceView(), m_StaticLiquidAlphaEnvironments[i][j][k]->getShaderResourceView());
			}

			context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullClockwise());
		}
	}
	m_Light.setPosition(m_Camera.getPosition().x, m_Camera.getPosition().y, m_Camera.getPosition().z);
}


void Game::RenderDynamicSpecimenEnvironments()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	m_environmentCamera.setPosition(m_Camera.getPosition());
	m_environmentCamera.Update();
 
	// NB: Dynamic, due to player movement
	for (int i = 0; i < m_GlassCount; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullCounterClockwise());

			m_DynamicSpecimenEnvironments[i][j]->setRenderTarget(context);
			m_DynamicSpecimenEnvironments[i][j]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);
			RenderSpecimensOnto(m_environmentCamera.getCamera(j), &m_Light, i);

			m_DynamicSpecimenAlphaEnvironments[i][j]->setRenderTarget(context);
			m_DynamicSpecimenAlphaEnvironments[i][j]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);
			RenderSpecimenAlphasOnto(m_environmentCamera.getCamera(j), i, m_DynamicSpecimenAlphaEnvironments[i][j]->getShaderResourceView());

			context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullClockwise());
		}
	}
}

void Game::RenderDynamicLiquidEnvironments()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	m_environmentCamera.setPosition(m_Camera.getPosition());
	m_environmentCamera.Update();

	// NB: Dynamic, due to player movement
	for (int i = 0; i < m_GlassCount; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullCounterClockwise());

			m_DynamicLiquidEnvironments[i][j]->setRenderTarget(context);
			m_DynamicLiquidEnvironments[i][j]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);
			RenderLiquidsOnto(m_environmentCamera.getCamera(j), &m_Light, i, m_DynamicSpecimenEnvironments[i][j]->getShaderResourceView());

			m_DynamicLiquidAlphaEnvironments[i][j]->setRenderTarget(context);
			m_DynamicLiquidAlphaEnvironments[i][j]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);
			RenderLiquidAlphasOnto(m_environmentCamera.getCamera(j), i, m_DynamicSpecimenAlphaEnvironments[i][j]->getShaderResourceView());

			context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullClockwise());
		}
	}
}


void Game::RenderDynamicEnvironment()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	m_environmentCamera.setPosition(m_Camera.getPosition());
	m_environmentCamera.Update();

	for (int i = 0; i < 6; i++)
	{
		if (m_environmentCamera.getCamera(i)->getReflection())
			context->RSSetState(m_states->CullCounterClockwise());

		m_DynamicEnvironment[i]->setRenderTarget(context);
		m_DynamicEnvironment[i]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);

		RenderSkyboxOnto(m_environmentCamera.getCamera(i));

		for (int j = 0; j < m_BasicCount; j++)
			RenderBasicsOnto(m_environmentCamera.getCamera(i), &m_Light, j);

		context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
		if (m_environmentCamera.getCamera(i)->getReflection())
			context->RSSetState(m_states->CullClockwise());
	}
}

void Game::RenderDynamicExternalEnvironments()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	m_environmentCamera.setPosition(m_Camera.getPosition());
	m_environmentCamera.Update();

	for (int i = 0; i < m_GlassCount; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullCounterClockwise());

			m_DynamicExternalEnvironments[i][j]->setRenderTarget(context);
			m_DynamicExternalEnvironments[i][j]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);

			RenderSkyboxOnto(m_environmentCamera.getCamera(j));

			for (int k = 0; k < m_BasicCount; k++)
				RenderBasicsOnto(m_environmentCamera.getCamera(j), &m_Light, k);

			// Draw PseudoGlass Models
			for (int k = 0; k < m_GlassCount; k++)
			{
				if (i == k)
					continue;

				RenderGlassOverlayOnto(m_environmentCamera.getCamera(j), k, m_DynamicEnvironment[j]->getShaderResourceView(), m_DynamicLiquidEnvironments[k][j]->getShaderResourceView(), m_DynamicLiquidAlphaEnvironments[k][j]->getShaderResourceView());
			}

			context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullClockwise());
		}
	}
}


void Game::RenderDynamicAirToGlassEnvironments()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	m_environmentCamera.setPosition(m_Camera.getPosition());
	m_environmentCamera.Update();

	for (int i = 0; i < m_GlassCount; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullCounterClockwise());

			m_DynamicAirToGlassEnvironments[i][j]->setRenderTarget(context);
			m_DynamicAirToGlassEnvironments[i][j]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);

			// NB: No need to wrry about surroundings for high density to low density
			/*RenderSkyboxOnto(m_environmentCamera.getCamera(j));

			for (int k = 0; k < m_BasicCount; k++)
				RenderBasicsOnto(m_environmentCamera.getCamera(j), &m_Light, k);

			// Draw PseudoGlass Models
			for (int k = 0; k < m_GlassCount; k++)
			{
				if (i == k)
					continue;

				RenderPseudoGlassOnto(m_environmentCamera.getCamera(j), &m_Light, k, m_DynamicLiquidEnvironments[k][j]->getShaderResourceView(), m_DynamicLiquidAlphaEnvironments[k][j]->getShaderResourceView());
			}*/

			// Draw refraction
			RenderRefractionOnto(m_environmentCamera.getCamera(j), &m_Light, i);

			context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullClockwise());
		}
	}
}

void Game::RenderDynamicInternalEnvironments()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	m_environmentCamera.setPosition(m_Camera.getPosition());
	m_environmentCamera.Update();

	for (int i = 0; i < m_GlassCount; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullCounterClockwise());

			m_DynamicInternalEnvironments[i][j]->setRenderTarget(context);
			m_DynamicInternalEnvironments[i][j]->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);

			// NB: No need to wrry about surroundings for high density to low density
			/*RenderSkyboxOnto(m_environmentCamera.getCamera(j));

			for (int k = 0; k < m_BasicCount; k++)
				RenderBasicsOnto(m_environmentCamera.getCamera(j), &m_Light, k);

			// Draw PseudoGlass Models
			for (int k = 0; k < m_GlassCount; k++)
			{
				if (i == k)
					continue;

				RenderPseudoGlassOnto(m_environmentCamera.getCamera(j), &m_Light, k, m_DynamicLiquidEnvironments[k][j]->getShaderResourceView(), m_DynamicLiquidAlphaEnvironments[k][j]->getShaderResourceView());
			}*/

			// Draw refraction
			RenderGlassOverlayOnto(m_environmentCamera.getCamera(j), i, m_DynamicAirToGlassEnvironments[i][j]->getShaderResourceView(), m_DynamicLiquidEnvironments[i][j]->getShaderResourceView(), m_DynamicLiquidAlphaEnvironments[i][j]->getShaderResourceView());

			context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
			if (m_environmentCamera.getCamera(j)->getReflection())
				context->RSSetState(m_states->CullClockwise());
		}
	}
}






// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	// Models
	m_Cube.InitializeModel(device, "cube.obj");
	m_Sphere.InitializeModel(device, "Unit Sphere (High Poly).obj");
	m_SpecimenJar1.InitializeModel(device, "Specimen Jar #1.obj");
	m_Teapot.InitializeModel(device, "cube.obj");
	m_DeathStar.InitializeModel(device, "death_star.obj");

	m_BasicCount = 3;
	m_GlassCount = 3;

	m_BasicModelPositions[0] = Vector3(-2.0f,30.0f, 0.0f);
	m_BasicModelPositions[1] = Vector3(0.0f, 0.0f, 0.0f);
	m_BasicModelPositions[2] = Vector3(2.0f, 0.0f, 0.0f);
	m_BasicModelPositions[3] = Vector3(2.0f, 0.0f, -2.0f);
	m_BasicModelPositions[4] = Vector3(2.0f, 0.0f, -4.0f);

	m_BasicModelTextures[0] = &m_NeutralRenderPass;
	m_BasicModelTextures[1] = &m_DemoRenderPass;
	m_BasicModelTextures[2] = &m_DemoRenderPass;
	m_BasicModelTextures[3] = &m_DemoNMRenderPass;
	m_BasicModelTextures[4] = &m_DemoNMRenderPass;

	m_BasicModelNMTextures[0] = &m_DemoNMRenderPass;
	m_BasicModelNMTextures[1] = &m_NeutralNMRenderPass;
	m_BasicModelNMTextures[2] = &m_DemoNMRenderPass;
	m_BasicModelNMTextures[3] = &m_DemoNMRenderPass;
	m_BasicModelNMTextures[4] = &m_NeutralNMRenderPass;

	m_BasicModels[0] = &m_Sphere;
	m_BasicModelScales[0] = 1.15f;
	m_BasicModelAxes[0] = Vector3(-1.0f, 1.0f, 1.0f);
	m_BasicModelAngles[0] = XM_PI/12;
	m_BasicModelPositions[0] = Vector3(0.0, -0.35, 0.0) + Vector3(3.1 * cos( -3*XM_PI / 8), 0.0f, 3.1 * sin(-3*XM_PI / 8));
	m_BasicModelTextures[0] = &m_SphericalPoresRenderPass;
	m_BasicModelNMTextures[0] = &m_SphericalPoresNMRenderPass;

	m_BasicModels[1] = &m_Sphere;
	m_BasicModelScales[1] = 0.75f;
	m_BasicModelAxes[1] = Vector3(-1.0f, 1.0f, 0.0f);
	m_BasicModelAngles[1] = XM_PI / 12;
	m_BasicModelPositions[1] = Vector3(0.0, -0.75, 0.0) + Vector3(2.5 * cos(-XM_PI / 6), 0.0f, 2.5 * sin(-XM_PI / 6));
	m_BasicModelTextures[1] = &m_SphericalPoresRenderPass;
	m_BasicModelNMTextures[1] = &m_SphericalPoresNMRenderPass;

	m_BasicModels[2] = &m_Sphere;
	m_BasicModelScales[2] = 1.25f;
	m_BasicModelAxes[2] = Vector3(0.0f, 1.0f, -1.0f);
	m_BasicModelAngles[2] = XM_PI / 24;
	m_BasicModelPositions[2] = Vector3(0.0, -0.25, 0.0) + Vector3(3.2 * cos(5 * XM_PI / 6), 0.0f, 3.2 * sin(5 * XM_PI / 6));
	m_BasicModelTextures[2] = &m_SphericalPoresRenderPass;
	m_BasicModelNMTextures[2] = &m_SphericalPoresNMRenderPass;

	m_GlassModels[0] = &m_Sphere;
	m_GlassModelScales[0] = 2.0f;
	m_GlassModelAxes[0] = Vector3(0.0f, 1.0f, 0.0f);
	m_GlassModelAngles[0] = 0.0f;
	m_GlassModelPositions[0] = Vector3(0.0f, 0.5f, 0.0f);
	m_GlassModelRefractiveIndex[0] = 1.33f;
	m_GlassModelOpacity[0] = 0.01f;
	m_GlassModelTextures[0] = &m_NeutralRenderPass;
	m_GlassModelNMTextures[0] = &m_NeutralNMRenderPass;

	m_LiquidOpacity[0] = 0.35;

	m_GlassModels[1] = &m_Sphere;
	m_GlassModelScales[1] = 0.75f;
	m_GlassModelAxes[1] = Vector3(0.0f, 1.0f, 0.0f);
	m_GlassModelAngles[1] = 0.0f;
	m_GlassModelPositions[1] = Vector3(0.0, -0.75, 0.0)+Vector3(2.5*cos(11*XM_PI/18), 0.0f, 2.5*sin(11 * XM_PI / 18));
	m_GlassModelRefractiveIndex[1] = 1.33f;
	m_GlassModelOpacity[1] = 0.02f;
	m_GlassModelTextures[1] = &m_NeutralRenderPass;
	m_GlassModelNMTextures[1] = &m_NeutralNMRenderPass;

	m_LiquidOpacity[1] = 0.5;

	m_GlassModels[2] = &m_Sphere;
	m_GlassModelScales[2] = 0.37f;
	m_GlassModelAxes[2] = Vector3(1.0f, 1.0f, 0.0f);
	m_GlassModelAngles[2] = 0.0f;
	m_GlassModelPositions[2] = Vector3(0.0, 0.25, 0.0) + Vector3(2.37 * cos(49 * XM_PI / 72), 0.0f, 2.37 * sin(49 * XM_PI / 72));
	m_GlassModelRefractiveIndex[2] = 1.50f;
	m_GlassModelOpacity[2] = 0.01f;
	m_GlassModelTextures[2] = &m_NeutralRenderPass;
	m_GlassModelNMTextures[2] = &m_NeutralNMRenderPass;

	m_LiquidOpacity[2] = 0.0;

	for (int i = 0; i < m_BasicCount; i++)
	{
		m_BasicModelAxes[i].Normalize();
		m_BasicModelTransforms[i] = SimpleMath::Matrix::CreateScale(m_BasicModelScales[i])*SimpleMath::Matrix::CreateFromAxisAngle(m_BasicModelAxes[i], m_BasicModelAngles[i])*SimpleMath::Matrix::CreateTranslation(m_BasicModelPositions[i]);
	}

	for (int i = 0; i < m_GlassCount; i++)
	{
		m_GlassModelAxes[i].Normalize();
		m_GlassModelTransforms[i] = SimpleMath::Matrix::CreateScale(m_GlassModelScales[i])*SimpleMath::Matrix::CreateFromAxisAngle(m_GlassModelAxes[i], m_GlassModelAngles[i])*SimpleMath::Matrix::CreateTranslation(m_GlassModelPositions[i]);
	}

	// Shaders
	m_LightShaderPair.InitLightShader(device, L"light_vs.cso", L"light_ps.cso");
	m_SkyboxShaderPair.InitSkyboxShader(device, L"skybox_vs.cso", L"skybox_ps.cso");
	m_SpecimenShaderPair.InitSpecimenShader(device, L"specimen_vs.cso", L"specimen_ps.cso");
	m_RefractionShaderPair.InitRefractionShader(device, L"refraction_vs.cso", L"refraction_ps.cso");
	m_GlassShaderPair.InitGlassShader(device, L"glass_vs.cso", L"glass_ps.cso");
	m_AlphaShaderPair.InitAlphaShader(device, L"alpha_vs.cso", L"alpha_ps.cso");
	m_OverlayShaderPair.InitOverlayShader(device, L"overlay_vs.cso", L"overlay_ps.cso");

	for (int i = 0; i < 6; i++)
		m_SkyboxRendering[i].InitShader(device, L"colour_vs.cso", L"skybox_pores.cso");

	m_NeutralRendering.InitShader(device, L"light_vs.cso", L"neutral.cso");
	m_NeutralNMRendering.InitShader(device, L"light_vs.cso", L"neutral_nm.cso");
	m_DemoRendering.InitShader(device, L"light_vs.cso", L"pores.cso");
	m_DemoNMRendering.InitShader(device, L"light_vs.cso", L"pores_nm.cso");

	m_SphericalPoresRendering.InitShader(device, L"light_vs.cso", L"spherical_pores.cso");
	m_SphericalPoresNMRendering.InitShader(device, L"light_vs.cso", L"spherical_pores_nm.cso");


	//load Textures
	CreateDDSTextureFromFile(device, L"Stylized_Stone_Floor_005_basecolor.dds",		nullptr,	m_texture1.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"EvilDrone_Diff.dds", nullptr,	m_texture2.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"Stylized_Stone_Floor_005_normal.dds", nullptr,	m_normalTexture1.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"brine_texture.dds", nullptr, m_brineTexture.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"glass_texture.dds", nullptr, m_glassTexture.ReleaseAndGetAddressOf());

	//Initialise Render to texture
	for (int i = 0; i < 6; i++)
	{
		m_SkyboxRenderPass[i] = new RenderTexture(device, 1280, 720, 1, 2);
	}

	m_NeutralRenderPass = new RenderTexture(device, 1280, 720, 1, 2);
	m_NeutralNMRenderPass = new RenderTexture(device, 1280, 720, 1, 2);
	m_DemoRenderPass = new RenderTexture(device, 1280, 720, 1, 2);
	m_DemoNMRenderPass = new RenderTexture(device, 1280, 720, 1, 2);
	m_SphericalPoresRenderPass = new RenderTexture(device, 1280, 720, 1, 2);
	m_SphericalPoresNMRenderPass = new RenderTexture(device, 1280, 720, 1, 2);

	for (int i = 0; i < 6; i++)
	{
		m_DynamicEnvironment[i] = new RenderTexture(device, 1280, 720, 1, 2);
	}

	for (int i = 0; i < m_GlassCount; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			for (int k = 0; k < m_GlassCount; k++)
			{
				m_StaticSpecimenEnvironments[i][j][k] = new RenderTexture(device, 1280, 720, 1, 2);
				m_StaticLiquidEnvironments[i][j][k] = new RenderTexture(device, 1280, 720, 1, 2);

				m_StaticSpecimenAlphaEnvironments[i][j][k] = new RenderTexture(device, 1280, 720, 1, 2);
				m_StaticLiquidAlphaEnvironments[i][j][k] = new RenderTexture(device, 1280, 720, 1, 2);
			}

			m_StaticEnvironments[i][j] = new RenderTexture(device, 1280, 720, 1, 2);
			m_StaticReflectionEnvironments[i][j] = new RenderTexture(device, 1280, 720, 1, 2);

			m_DynamicSpecimenEnvironments[i][j] = new RenderTexture(device, 1280, 720, 1, 2);
			m_DynamicLiquidEnvironments[i][j] = new RenderTexture(device, 1280, 720, 1, 2);

			m_DynamicSpecimenAlphaEnvironments[i][j] = new RenderTexture(device, 1280, 720, 1, 2);
			m_DynamicLiquidAlphaEnvironments[i][j] = new RenderTexture(device, 1280, 720, 1, 2);

			m_DynamicExternalEnvironments[i][j] = new RenderTexture(device, 1280, 720, 1, 2);
			m_DynamicAirToGlassEnvironments[i][j] = new RenderTexture(device, 1280, 720, 1, 2);
			m_DynamicInternalEnvironments[i][j] = new RenderTexture(device, 1280, 720, 1, 2);
		}
	}



	m_preRendered = false;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 50.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    //if (aspectRatio < 1.0f)
    //{
    //    fovAngleY *= 2.0f;
    //}

    // This sample makes use of a right-handed coordinate system using row-major matrices.
	m_Camera.setPerspective(fovAngleY, aspectRatio, 0.01f, 100.0f);
}


void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
	m_batch.reset();
	m_testmodel.reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
