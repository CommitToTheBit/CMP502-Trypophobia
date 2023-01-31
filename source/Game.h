//
// Game.h
//
#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "modelclass.h"
#include "Light.h"
#include "Input.h"
#include "RenderTexture.h"

#include "Camera.h"
#include "EnvironmentCamera.h"
#include "SpecimenShader.h"

#include "Shader.h"
#include "LightShader.h"
#include "SkyboxShader.h"
#include "RefractionShader.h"
#include "GlassShader.h"
#include "AlphaShader.h"
#include "OverlayShader.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
#ifdef DXTK_AUDIO
    void NewAudioDevice();
#endif

    // Properties
    void GetDefaultSize( int& width, int& height ) const;
	
private:

	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	}; 

    void Update(DX::StepTimer const& timer);
    void UpdateModels(float time);

    void Render();

    // Rendering models
    void RenderBasicsOnto(Camera* camera, Light* light, int i);

    void RenderSpecimensOnto(Camera* camera, Light* light, int i);
    void RenderLiquidsOnto(Camera* camera, Light* light, int i, ID3D11ShaderResourceView* specimen);

    void RenderSpecimenAlphasOnto(Camera* camera, int i, ID3D11ShaderResourceView* alpha);
    void RenderLiquidAlphasOnto(Camera* camera, int i, ID3D11ShaderResourceView* alpha);

    void RenderRefractionOnto(Camera* camera, Light* light, int i);
    void RenderGlassOverlayOnto(Camera* camera, int i, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* overlay, ID3D11ShaderResourceView* alpha);
    void RenderGlassOnto(Camera* camera, Light* light, int index);

    void RenderSkyboxOnto(Camera* camera);

    // Render passes
    void RenderStaticTextures();
    void RenderDynamicTextures();
    void RenderShaderTexture(RenderTexture* renderPass, Shader rendering);

    void RenderStaticSpecimenEnvironments();
    void RenderStaticLiquidEnvironments();

    void RenderStaticEnvironments();
    void RenderStaticReflectionEnvironments();

    void RenderDynamicSpecimenEnvironments();
    void RenderDynamicLiquidEnvironments();

    void RenderDynamicEnvironment();
    void RenderDynamicExternalEnvironments();
    void RenderDynamicAirToGlassEnvironments();
    void RenderDynamicInternalEnvironments();

    //void RenderStaticSpecimenTextures();
    //void RenderDynamicSpecimenTextures();

    //void RenderStaticLiquidTextures();
    //void RenderDynamicLiquidTextures();

    //void RenderStaticGlassEnvironments();
    //void RenderDynamicGlassFirstInternals();
    //void RenderDynamicGlassFirstExternals();

    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    float                                   m_time;
    bool                                    m_preRendered;

	//input manager. 
	Input									m_input;
	InputCommands							m_gameInputCommands;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;	
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

	// Scene Objects
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;
	std::unique_ptr<DirectX::GeometricPrimitive>                            m_testmodel;

	//lights
	Light																	m_Light;
    DirectX::SimpleMath::Vector4                                            m_Ambience;

	//Cameras
	Camera																	m_Camera;
    EnvironmentCamera                                                       m_environmentCamera;

	//textures 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture1;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture2;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_normalTexture1;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_brineTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_glassTexture;

	//Shaders
	LightShader																m_LightShaderPair;
    SkyboxShader                                                            m_SkyboxShaderPair;
    SpecimenShader                                                          m_SpecimenShaderPair;
    RefractionShader                                                        m_RefractionShaderPair;
    GlassShader                                                             m_GlassShaderPair;
    AlphaShader                                                             m_AlphaShaderPair;
    OverlayShader                                                           m_OverlayShaderPair;

    //GlassShader                                                             m_GlassFrontShaderPair;
    //GlassShader                                                             m_GlassBackShaderPair;

    // Models
    ModelClass																m_Cube;
    ModelClass                                                              m_Sphere;
    ModelClass                                                              m_SpecimenJar1;
    ModelClass                                                              m_Teapot;
    ModelClass                                                              m_DeathStar;

    //int                                                                     m_BasicCount;
    //ModelClass*                                                             m_BasicModels[5];
    //DirectX::SimpleMath::Vector3                                            m_BasicModelPositions[5];
    //RenderTexture**                                                         m_BasicModelTextures[5];
    //RenderTexture**                                                         m_BasicModelNMTextures[5];

    int                                                                     m_BasicCount;
    ModelClass*                                                             m_BasicModels[15];
    float                                                                   m_BasicModelScales[15];
    DirectX::SimpleMath::Vector3                                            m_BasicModelAxes[15];
    float                                                                   m_BasicModelAngles[15];
    DirectX::SimpleMath::Vector3                                            m_BasicModelPositions[15];
    DirectX::SimpleMath::Matrix                                             m_BasicModelTransforms[15];
    RenderTexture**                                                         m_BasicModelTextures[15];
    RenderTexture**                                                         m_BasicModelNMTextures[15];

    int                                                                     m_GlassCount;
    ModelClass*                                                             m_GlassModels[4];
    float                                                                   m_GlassModelScales[4];
    DirectX::SimpleMath::Vector3                                            m_GlassModelAxes[4];
    float                                                                   m_GlassModelAngles[4];
    DirectX::SimpleMath::Vector3                                            m_GlassModelPositions[4];
    DirectX::SimpleMath::Matrix                                             m_GlassModelTransforms[4];
    RenderTexture**                                                         m_GlassModelTextures[4];
    RenderTexture**                                                         m_GlassModelNMTextures[4];
    float                                                                   m_GlassModelRefractiveIndex[4];
    float                                                                   m_GlassModelOpacity[4];
    float                                                                   m_GlassModelSheen[4];

    float                                                                   m_LiquidOpacity[3];

    //int                                                                     m_Specimens[2];
    //ModelClass*                                                             m_SpecimenModels[2][3];

	// Generated Textures
    RenderTexture*                                                          m_SkyboxRenderPass[6];
    Shader                                                                  m_SkyboxRendering[6];

    RenderTexture*                                                          m_NeutralRenderPass;
    Shader                                                                  m_NeutralRendering;

    RenderTexture*                                                          m_NeutralNMRenderPass;
    Shader                                                                  m_NeutralNMRendering;

    RenderTexture*                                                          m_DemoRenderPass;
    Shader                                                                  m_DemoRendering;

    RenderTexture*                                                          m_DemoNMRenderPass;
    Shader                                                                  m_DemoNMRendering;

    RenderTexture*                                                          m_SphericalPoresRenderPass;
    Shader                                                                  m_SphericalPoresRendering;

    RenderTexture*                                                          m_SphericalPoresNMRenderPass;
    Shader                                                                  m_SphericalPoresNMRendering;

    // Specimen Textures
    RenderTexture*                                                          m_StaticSpecimenEnvironments[4][6][4];      // Indices: object viewing/direction/object viewed
    RenderTexture*                                                          m_StaticLiquidEnvironments[4][6][4];        // Indices: object viewing/direction/object viewed

    RenderTexture*                                                          m_StaticSpecimenAlphaEnvironments[4][6][4]; // Indices: object viewing/direction/object viewed
    RenderTexture*                                                          m_StaticLiquidAlphaEnvironments[4][6][4];   // Indices: object viewing/direction/object viewed

    RenderTexture*                                                          m_StaticEnvironments[4][6];                 // Indices: object viewing/direction
    RenderTexture*                                                          m_StaticReflectionEnvironments[4][6];       // Indices: object viewing/direction

    RenderTexture*                                                          m_DynamicSpecimenEnvironments[4][6];        // Indices: object viewed/direction
    RenderTexture*                                                          m_DynamicLiquidEnvironments[4][6];          // Indices: object viewed/direction

    RenderTexture*                                                          m_DynamicEnvironment[6];                    // Indices: object viewing/direction
    RenderTexture*                                                          m_DynamicSpecimenAlphaEnvironments[4][6];   // Indices: object viewed/direction
    RenderTexture*                                                          m_DynamicLiquidAlphaEnvironments[4][6];     // Indices: object viewed/direction

    RenderTexture*                                                          m_DynamicExternalEnvironments[4][6];        // Indices: object refracting/direction
    RenderTexture*                                                          m_DynamicAirToGlassEnvironments[4][6];      // Indices: object refracting/direction
    RenderTexture*                                                          m_DynamicInternalEnvironments[4][6];        // Indices: object refracting/direction


#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;
#endif
    

#ifdef DXTK_AUDIO
    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;
#endif

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_projection;
};