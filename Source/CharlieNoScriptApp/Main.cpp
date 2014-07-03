/***********************************************************************************************************************************************
*
*	The Non Scripted Charlie Application
*
*	Impliments a vForge scene containing a single VisBaseEntity_cl characters providing total control of the embedded Behavior Graph.
*
*	Requires VisBaseEntity_cl to have Havok Behavior Character Component set to CharlieHAT.hkt / CharlieNoScript.hkt / NoScriptBehaviour.hkt
*
*
***********************************************************************************************************************************************/

#include "CharlieNoScriptAppPCH.h"
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


class CharlieNoScriptApp : public VAppImpl
{
public:
	enum HAT_StateIDs		//maps the CharlieHAT StateMachine01
	{
		HAT_Move=1,
		HAT_Idle,
		HAT_Turn,
		HAT_Jump
	};

	enum HAT_TurnIds		//maps the CharlieHAT Turn Selector
	{
		TURN_Left,
		TURN_Right,
		TURN_Jump
	};
	enum HAT_JumpIds		//maps the CharlieHAT Jump Selector
	{
		JUMP_Standing,
		JUMP_Running
	};


	CharlieNoScriptApp();
	virtual ~CharlieNoScriptApp(){};

	virtual	void SetupAppConfig(VisAppConfig_cl& config) HKV_OVERRIDE;
	virtual	void PreloadPlugins() HKV_OVERRIDE;

	virtual	void Init() HKV_OVERRIDE;
	virtual	void AfterSceneLoaded(bool bLoadingSuccessful) HKV_OVERRIDE;
	virtual	bool Run() HKV_OVERRIDE;
	virtual	void DeInit() HKV_OVERRIDE;

			void doCharlieIdle();
			void doCharlieMove();


private:
	VisBaseEntity_cl*			m_pCharlie;									// ptr to the Charlie Entity
	hkbWorld*					m_pBehaviourWorld;							// ptr to the current Havok Behavior World
	vHavokBehaviorComponent*	m_pBehaviourComponent;						// ptr to the associated Havok Behavior Character Component
	VVirtualThumbStick*			m_pThumbStick;								// ptr to out optional thumbstick object
	float						m_leftStickY,m_leftStickX;					// our internal representaion of our joypad values to control the behaviour
	float						m_speed;									// forward velocity - matches Behavior Graph Variable
	float						m_turn;										// turn factor - matches Behavior Graph Variable
	float						m_twist;									// twist factor - matches Behavior Graph Variable
};

VAPP_IMPLEMENT_SAMPLE(CharlieNoScriptApp);

CharlieNoScriptApp::CharlieNoScriptApp()
:	m_pCharlie(NULL)
,	m_pBehaviourWorld(NULL)
,	m_pBehaviourComponent(NULL)
,	m_pThumbStick(NULL)
,	m_leftStickY(0)
,	m_leftStickX(0)
,	m_speed(0)
,	m_turn(0)
,	m_twist(0)
{
}

/***********************************************************************************************************************************************
*
*	One time initialisation performed after the scene has been loaded
*
*		Fetchs a pointer to the Havok Behavior World instance (not actually used, kept for reference)
*		Fetchs a pointer to the attached Havok Behavior Character Component
*		Validates the pointers (Debug only)
*
*	Builds device appropriate control interface mappings, including virtual thumbpad if required.
*
***********************************************************************************************************************************************/
void CharlieNoScriptApp::AfterSceneLoaded(bool bLoadingSuccessful)
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
*	Sets the internal variables m_LeftStickX and m_LeftStickY to these values.
*	Determines Jump behaviour and set state if required
*	Calculates forward velocity, turn factor and twist factor.
*	Fetches variable status from Behavior Componant and displays debug information to the screen
*	Based upon Behavior Graph state calls appropriate functions to update character
*
***********************************************************************************************************************************************/
bool CharlieNoScriptApp::Run()
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


	if (GetInputMap()->GetTrigger(CHARLIE_JUMP))							// Test for the Jump button
	{																		
		m_pBehaviourComponent->SetWordVar("JumpType",((m_speed >0.5)?JUMP_Running:JUMP_Standing));
																			// determine which jump animation and set the 'JumpType' variable
		m_pBehaviourComponent->TriggerEvent("to_Jump");						// and fire a 'Jump' event
	}
																			
	m_speed = hkvMath::sqrt(m_leftStickY*m_leftStickY + m_leftStickX*m_leftStickX);
	m_turn = 0;																// Calculate a forward velocity from the control stick positions
	if (hkvMath::Abs(m_leftStickY*m_leftStickX) > 0)						// and if possible
	{
		hkvVec3 vec = hkvVec3(m_leftStickX,m_leftStickY,0.f);				// projecting stick vector onto a right vector
		m_turn = vec.dot(RIGHT_VEC);										// to calculate a turn factor
	}
	m_twist = m_turn*TWIST_ANGLE;											// then calculate the appropriate twist factor

	int state = m_pBehaviourComponent->GetWordVar("CurrentState");			// fetch the current state of the Behvior Graph

	float speed = m_pBehaviourComponent->GetFloatVar("speed");				// whilst here, can also fetch all other states and veriables for debbuging purposes..
	float turn = m_pBehaviourComponent->GetFloatVar("turn");				//
	float twist = m_pBehaviourComponent->GetFloatVar("twist");
	Vision::Message.Print(1, 10, 180, "stick  %f %f",m_leftStickX,m_leftStickY);
	Vision::Message.Print(1, 10, 200, "State = %d  speed = %f turn = %f twist = %f",state,speed,turn,twist);

	switch(state)															// based on current Behavior Graph states
	{
		case (HAT_Move):
			doCharlieMove();												// perform the Move functionality
			break;
		case (HAT_Idle):
			doCharlieIdle();												// perform the Idle functionality
			break;
		default:															// else do nothing...
			break;
	}
	return true;
}

/***********************************************************************************************************************************************
*
*	void CharlieNoScriptApp::doCharlieIdle()
*
*	Updates Idle state.
*	Monitors control stick input and determines appropriate move or turn behavior based on relative directions
*	Return: void
*
*
***********************************************************************************************************************************************/
void CharlieNoScriptApp::doCharlieIdle()
{
	if ( m_leftStickY > MOVE_THRESHOLD )									// stick forward velocity exceeds threhold??
	{																		
		m_pBehaviourComponent->TriggerEvent("Idle_to_Move");				// then fire the Move event, no need to turn
	}
	else if ( m_leftStickY < -0.4 )											// stick is pulled back??
	{																		// we want a full 180° turn
		m_pBehaviourComponent->SetWordVar("TurnType",TURN_Jump);			// so set the TurnType variable to TURN_JUMP
		m_pBehaviourComponent->TriggerEvent("Idle_to_Turn");				// fire the Turn event..
	}
	else if ( hkvMath::Abs(m_leftStickX) > 0.2 )							// other wise is the stick moved in the X axis??
	{
		if (m_leftStickX<0)													// pushing right??
		{
			m_pBehaviourComponent->SetWordVar("TurnType",TURN_Right);		// then set the TurnType variable to TURN_RIGHT 
		}
		else
		{
			m_pBehaviourComponent->SetWordVar("TurnType",TURN_Left);		// else set the TurnType variable to TURN_LEFT
		}
		m_pBehaviourComponent->TriggerEvent("Idle_to_Turn");				// fire the Turn event..
	}
}																			// else drops through if nothing to do....

/***********************************************************************************************************************************************
*
*	void CharlieNoScriptApp::doCharlieMove()
*
*	Updates Move state.
*	Monitors control stick input and determines appropriate state change.
*	Updates Behavior Graph variables with pre-calculated local values.
*	Return: void
*
*
***********************************************************************************************************************************************/
void CharlieNoScriptApp::doCharlieMove()
{

	if ( m_speed < MOVE_THRESHOLD )											// stick forward velocity less than threhold??
	{
		m_pBehaviourComponent->TriggerEvent("Move_to_Idle");				// then fire the Idle event, no need to continue
	}
	else
	{
		m_pBehaviourComponent->SetFloatVar("speed",m_speed);				// set the Behavior Graph variable 'speed'
		m_pBehaviourComponent->SetFloatVar("turn",m_turn);					// set the Behavior Graph variable 'turn'
		m_pBehaviourComponent->SetFloatVar("twist",m_twist);				// set the Behavior Graph variable 'twist'
	}
}

//*************************************	STANDARD VISION APPLICATION WARAPPER FUNCTIONS ****************************************

void CharlieNoScriptApp::SetupAppConfig(VisAppConfig_cl& config)
{
  config.m_sFileSystemRootName = "workspace";
#if defined(WIN32)
  config.m_videoConfig.m_iXRes = 1280; // Set the Window size X if not in fullscreen.
  config.m_videoConfig.m_iYRes = 720;  // Set the Window size Y if not in fullscreen.
  config.m_videoConfig.m_iXPos = 50;   // Set the Window position X if not in fullscreen.
  config.m_videoConfig.m_iYPos = 50;   // Set the Window position Y if not in fullscreen.
  config.m_videoConfig.m_szWindowTitle = "ProjectCharlie No Script";
#endif
  config.m_videoConfig.m_bWaitVRetrace = true;
}

void CharlieNoScriptApp::PreloadPlugins()
{
  // Use the following line to load a plugin. Remember that, except on Windows platform, in addition
  // you still need to statically link your plugin library (e.g. on mobile platforms) through project
  // Properties, Linker, Additional Dependencies.
	VISION_PLUGIN_ENSURE_LOADED( vHavok );
	VISION_PLUGIN_ENSURE_LOADED( vHavokBehavior );
}

void CharlieNoScriptApp::Init()
{
  // Set filename and paths to our stand alone version.
  // Note: "/Data/Vision/Base" is always added by the sample framework
  VisAppLoadSettings settings("Scenes/CharlieWorldNoScript.vscene");
  settings.m_customSearchPaths.Append(":workspace/Assets");
  LoadScene(settings);
}

void CharlieNoScriptApp::DeInit()
{
#if defined(_VISION_MOBILE)
	V_SAFE_DELETE(m_pThumbStick);
#endif
	m_pCharlie=NULL;
	m_pBehaviourWorld=NULL;
	m_pBehaviourComponent=NULL;
}
