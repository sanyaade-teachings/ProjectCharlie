/***********************************************************************************************************************************************
*
*	The Part Scripted Charlie Application
*
*	Impliments a vForge scene containing a single VisBaseEntity_cl characters providing automated control of the embedded Behavior Graph.
*
*	Requires VisBaseEntity_cl to have Havok Behavior Character Component set to CharlieHAT.hkt / CharlieScript.hkt / ScriptBehaviour.hkt
*
*
***********************************************************************************************************************************************/

#include "CharliePartScriptAppPCH.h"
#include <Vision/Runtime/Framework/VisionApp/VAppImpl.hpp>


#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/HavokBehaviorEnginePlugin.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorModule.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorComponent.hpp>

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Input/VVirtualThumbStick.hpp>

enum CHARLIE_CONTROL 
{
  CHARLIE_STICK_X = VAPP_INPUT_CONTROL_LAST_ELEMENT + 1,
  CHARLIE_STICK_Y,
  CHARLIE_JUMP,
  CHARLIE_LEFT,
  CHARLIE_RIGHT,
  CHARLIE_UP,
  CHARLIE_DOWN
};


static const float		MOVE_THRESHOLD = 0.1;
static const float		TWIST_ANGLE=45.0;
static const hkvVec3	RIGHT_VEC = hkvVec3(1,0,0);


class CharliePartScriptApp : public VAppImpl
{
public:
	CharliePartScriptApp();
	virtual ~CharliePartScriptApp(){};

	virtual	void SetupAppConfig(VisAppConfig_cl& config) HKV_OVERRIDE;
	virtual	void PreloadPlugins() HKV_OVERRIDE;

	virtual	void Init() HKV_OVERRIDE;
	virtual	void AfterSceneLoaded(bool bLoadingSuccessful) HKV_OVERRIDE;
	virtual	bool Run() HKV_OVERRIDE;
	virtual	void DeInit() HKV_OVERRIDE;


private:
	VisBaseEntity_cl*			m_pCharlie;								// ptr to the 'Charlie' entity
	hkbWorld*					m_pBehaviourWorld;						// ptr to our Havok Behavior World
	vHavokBehaviorComponent*	m_pBehaviourComponent;					// ptr to the Havok Behavior Character Componant associated to the entity
	VVirtualThumbStick*			m_pThumbStick;							// ptr to out optional thumbstick object
	float						m_leftStickY,m_leftStickX;				// our internal representaion of our joypad values to control the behaviour
};

VAPP_IMPLEMENT_SAMPLE(CharliePartScriptApp);

CharliePartScriptApp::CharliePartScriptApp()
:	m_pCharlie(NULL)
,	m_pBehaviourWorld(NULL)
,	m_pBehaviourComponent(NULL)
,	m_pThumbStick(NULL)
,	m_leftStickY(0)
,	m_leftStickX(0)
{
}

/***********************************************************************************************************************************************
*
*	One time initialisation performed after the scene has been loaded
*
*		Fetchs a pointer to the Havok Behavior World instance (not actually used, kept for reference)
*		Fetchs a pointer to the attached Havok Behavior Character Component
*		Validates the pointers (Debug only)

	Builds device appropriate control interface mappings, including virtual thumbpad if required.
*
***********************************************************************************************************************************************/
void CharliePartScriptApp::AfterSceneLoaded(bool bLoadingSuccessful)
{
	m_pBehaviourWorld = vHavokBehaviorModule::GetInstance()->getBehaviorWorld();
		VASSERT(m_pBehaviourWorld);
	m_pCharlie = Vision::Game.SearchEntity("Charlie");
		VASSERT(m_pCharlie);
	m_pBehaviourComponent = static_cast<vHavokBehaviorComponent *>(m_pCharlie->Components().GetComponentOfType(V_RUNTIME_CLASS(vHavokBehaviorComponent)));
		VASSERT(m_pBehaviourComponent);
		VASSERT(m_pBehaviourComponent->m_character);

#if defined(_VISION_MOBILE)
	m_pThumbStick = new VVirtualThumbStick();
	m_pThumbStick->Show();
	GetInputMap()->MapTriggerAxis(CHARLIE_STICK_X, *m_pThumbStick, CT_PAD_LEFT_THUMB_STICK_LEFT, CT_PAD_LEFT_THUMB_STICK_RIGHT, VInputOptions::DeadZone(0.1f, false));
	GetInputMap()->MapTriggerAxis(CHARLIE_STICK_Y, *m_pThumbStick, CT_PAD_LEFT_THUMB_STICK_UP, CT_PAD_LEFT_THUMB_STICK_DOWN, VInputOptions::DeadZone(0.1f, false));

	VTouchArea* pTouchArea = new VTouchArea(VInputManager::GetTouchScreen(), VRectanglef(0.0f, 0.0f, Vision::Video.GetXRes(), Vision::Video.GetYRes()),-2000.f);
	GetInputMap()->MapTrigger(CHARLIE_JUMP, pTouchArea, CT_TOUCH_DOUBLE_TAP,VInputOptions::Once(ONCE_ON_RELEASE));
#else
	GetInputMap()->MapTriggerAxis(CHARLIE_STICK_X, V_PAD(0), CT_PAD_LEFT_THUMB_STICK_LEFT, CT_PAD_LEFT_THUMB_STICK_RIGHT, VInputOptions::DeadZone(0.1f, false));
	GetInputMap()->MapTriggerAxis(CHARLIE_STICK_Y, V_PAD(0), CT_PAD_LEFT_THUMB_STICK_UP, CT_PAD_LEFT_THUMB_STICK_DOWN, VInputOptions::DeadZone(0.1f, false));
	GetInputMap()->MapTrigger(CHARLIE_JUMP, V_PAD(0), CT_PAD_A,VInputOptions::Once(ONCE_ON_PRESS));

	GetInputMap()->MapTrigger(CHARLIE_LEFT, V_KEYBOARD, CT_KB_LEFT);
	GetInputMap()->MapTrigger(CHARLIE_RIGHT, V_KEYBOARD, CT_KB_RIGHT);
	GetInputMap()->MapTrigger(CHARLIE_UP, V_KEYBOARD, CT_KB_UP);
	GetInputMap()->MapTrigger(CHARLIE_DOWN, V_KEYBOARD, CT_KB_DOWN);
	GetInputMap()->MapTrigger(CHARLIE_JUMP, V_KEYBOARD, CT_KB_SPACE,VInputOptions::Once(ONCE_ON_PRESS));
#endif
}

/***********************************************************************************************************************************************
*
*	bool ProjectCharlieApp::Run()
*
*	Main 'Run' loop of our application
*
*	Fetches the device appropriate control values and generates X and Y stick values (range -1.0f to 1.0f)
*	Sets the Behavior Variables "LeftStickX" and "LeftStickY" to these values.
*
*	Fetches variable status from Behavior Componant and displays debug information to the screen
*
*
***********************************************************************************************************************************************/
bool CharliePartScriptApp::Run()
{
#if defined(_VISION_MOBILE)
	m_leftStickX = -GetInputMap()->GetTrigger(CHARLIE_STICK_X);				// On mobile device just read the virtual stick
	m_leftStickY = -GetInputMap()->GetTrigger(CHARLIE_STICK_Y);				// to get out stick X&Y values..
#else
	if (!VInputManager::GetPad(0).IsActive())								// On PC, do we have a game pad???
	{																		// No....
		m_leftStickX=0.f;
		if(GetInputMap()->GetTrigger(CHARLIE_LEFT))							// then map the virual stick to the left cursor key...
		{
			m_leftStickX=0.75f;
		}
		else if(GetInputMap()->GetTrigger(CHARLIE_RIGHT))					// and the right cursor key...
		{
			m_leftStickX=-0.75f;
		}
		m_leftStickY=0.f;
		if(GetInputMap()->GetTrigger(CHARLIE_UP))							// and the up cursor key...
		{
			m_leftStickY=0.75f;
		}
		else if(GetInputMap()->GetTrigger(CHARLIE_DOWN))					// and the down cursor key...
		{
			m_leftStickY=-0.75f;
		}
	}
	else
	{
		m_leftStickX = -GetInputMap()->GetTrigger(CHARLIE_STICK_X);			//else just read the pad..
		m_leftStickY = -GetInputMap()->GetTrigger(CHARLIE_STICK_Y);
	}
#endif

	m_pBehaviourComponent->SetFloatVar("LeftStickX",m_leftStickX);			// Set the 'LeftStickX' variable..
	m_pBehaviourComponent->SetFloatVar("LeftStickY",m_leftStickY);			// ..and the 'LeftStickY' variable in the Behaviour Graph
	if (GetInputMap()->GetTrigger(CHARLIE_JUMP))							// Test for the Jump button
	{
		m_pBehaviourComponent->TriggerEvent("Jump");						// and fire a 'Jump' event if needed
	}						
																			// the inbuilt script will take care of everything from here..
																			// for debug purposes we can read back the internal variables and states
																			// thus ...	but do not expect the values read to reflect or be synched to the values only just written
	float speed = m_pBehaviourComponent->GetFloatVar("speed");
	float turn = m_pBehaviourComponent->GetFloatVar("turn");
	float twist = m_pBehaviourComponent->GetFloatVar("twist");
	int state = m_pBehaviourComponent->GetWordVar("CurrentState");
	Vision::Message.Print(1, 10, 180, "stick  %f %f",m_leftStickX,m_leftStickY);
	Vision::Message.Print(1, 10, 200, "State = %d  speed = %f turn = %f twist = %f",state,speed,turn,twist);

	return true;
}


//*************************************	STANDARD VISION APPLICATION WARAPPER FUNCTIONS ****************************************


void CharliePartScriptApp::SetupAppConfig(VisAppConfig_cl& config)
{
  config.m_sFileSystemRootName = "workspace";

#if defined(WIN32)
  config.m_videoConfig.m_iXRes = 1280; // Set the Window size X if not in fullscreen.
  config.m_videoConfig.m_iYRes = 720;  // Set the Window size Y if not in fullscreen.
  config.m_videoConfig.m_iXPos = 50;   // Set the Window position X if not in fullscreen.
  config.m_videoConfig.m_iYPos = 50;   // Set the Window position Y if not in fullscreen.
  config.m_videoConfig.m_szWindowTitle = "ProjectCharlie Scripted";
#endif
  config.m_videoConfig.m_bWaitVRetrace = true;
}

void CharliePartScriptApp::PreloadPlugins()
{
  // Use the following line to load a plugin. Remember that, except on Windows platform, in addition
  // you still need to statically link your plugin library (e.g. on mobile platforms) through project
  // Properties, Linker, Additional Dependencies.
	VISION_PLUGIN_ENSURE_LOADED( vHavok );
	VISION_PLUGIN_ENSURE_LOADED( vHavokBehavior );
}

void CharliePartScriptApp::Init()
{
  // Set filename and paths to our stand alone version.
  // Note: "/Data/Vision/Base" is always added by the sample framework
  VisAppLoadSettings settings("Scenes/CharlieWorldScript.vscene");
  settings.m_customSearchPaths.Append(":workspace/Assets");
  LoadScene(settings);
}

void CharliePartScriptApp::DeInit()
{

#if defined(_VISION_MOBILE)
	V_SAFE_DELETE(m_pThumbStick);
#endif
	m_pCharlie=NULL;
	m_pBehaviourWorld=NULL;
	m_pBehaviourComponent=NULL;
}

