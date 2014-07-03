/***********************************************************************************************************************************************
*
*	The Project Charlie Application
*
*	Impliments a vForge scene containing CharlieNPC_cl NPC characters provides a fully self-automated control.
*	CharlieNPC_cl characters cause the CharlieNoScript behaviour to idle and navigate between random nodes of a pre-defined path
*	(See "Scenes/CharlieWorldNPC.vscene")
*
*	Requires CharlieNPC_cl to have Havok Behavior Character Component set to CharlieHAT.hkt / CharlieNoScript.hkt / NoScriptBehaviour.hkt
*	and an associated vPath object within the scene to provide control nodes.
*
*	No other input or control is required from the application (exception:- debug information)
*
*
***********************************************************************************************************************************************/

#include "ProjectCharlieAppPCH.h"
#include <Vision/Runtime/Framework/VisionApp/VAppImpl.hpp>
#include "../CharlieNPCPlugin/NPCObject.h"

VIMPORT IVisPlugin_cl* GetEnginePlugin_CharlieNPCPlugin();
class ProjectCharlieApp : public VAppImpl
{
public:


	ProjectCharlieApp(){};
	virtual ~ProjectCharlieApp(){};

	virtual	void		SetupAppConfig(VisAppConfig_cl& config) HKV_OVERRIDE;
	virtual	void		PreloadPlugins() HKV_OVERRIDE;

	virtual	void		Init() HKV_OVERRIDE;
	virtual	void		AfterSceneLoaded(bool bLoadingSuccessful) HKV_OVERRIDE;
	virtual	bool		Run() HKV_OVERRIDE;
	virtual	void		DeInit() HKV_OVERRIDE;


private:
	// no member variables required
};

VAPP_IMPLEMENT_SAMPLE(ProjectCharlieApp);

/***********************************************************************************************************************************************
*
*	Gets called after the scene has been loaded
*
*	Do absolutely nothing for this demo, everything is self-autonomous 
*
***********************************************************************************************************************************************/
void ProjectCharlieApp::AfterSceneLoaded(bool bLoadingSuccessful)
{
}

/***********************************************************************************************************************************************
*
*	bool ProjectCharlieApp::Run(
*
*	Main 'Run' loop of our application
*
*	Fetches a pointer to a Charlie NPC character referenced by the Key ("Charlie")
*	Calls the class debug Getter to fetch Behavior status
*	Displays status on screen
*
*
***********************************************************************************************************************************************/
bool ProjectCharlieApp::Run()
{
	CharlieNPC_cl* pCharlie= static_cast<CharlieNPC_cl*>(Vision::Game.SearchEntity("Charlie"));
	if(pCharlie)
	{
		CharlieNPC_cl::debugData	data;
		pCharlie->GetDebugData(data);
		Vision::Message.Print(1, 10, 100, "State = %d  speed = %f turn = %f twist = %f turnType = %d",data.state,data.speed,data.turn,data.twist,data.turnType);
	}
	return true;
}


//*************************************	STANDARD VISION APPLICATION WARAPPER FUNCTIONS ****************************************


void ProjectCharlieApp::SetupAppConfig(VisAppConfig_cl& config)
{
  config.m_sFileSystemRootName = "workspace";
#if defined(WIN32)
  config.m_videoConfig.m_iXRes = 1280; // Set the Window size X if not in fullscreen.
  config.m_videoConfig.m_iYRes = 720;  // Set the Window size Y if not in fullscreen.
  config.m_videoConfig.m_iXPos = 50;   // Set the Window position X if not in fullscreen.
  config.m_videoConfig.m_iYPos = 50;   // Set the Window position Y if not in fullscreen.
  // Name to be displayed in the windows title bar.
  config.m_videoConfig.m_szWindowTitle = "ProjectCharlie";
#endif
  config.m_videoConfig.m_bWaitVRetrace = true;
}

void ProjectCharlieApp::PreloadPlugins()
{
  // Use the following line to load a plugin. Remember that, except on Windows platform, in addition
  // you still need to statically link your plugin library (e.g. on mobile platforms) through project
  // Properties, Linker, Additional Dependencies.
	VISION_PLUGIN_ENSURE_LOADED( vHavok );
	VISION_PLUGIN_ENSURE_LOADED( vHavokBehavior );
	VISION_PLUGIN_ENSURE_LOADED( CharlieNPCPlugin );
}

void ProjectCharlieApp::Init()
{
  // Set filename and paths to our stand alone version.
  // Note: "/Data/Vision/Base" is always added by the sample framework
  VisAppLoadSettings settings("Scenes/CharlieWorldNPC.vscene");
  settings.m_customSearchPaths.Append(":workspace/Assets");
  LoadScene(settings);
}

void ProjectCharlieApp::DeInit()
{
}

