/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2014 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include "CharlieNPCPluginPCH.h"

#include <Common/Base/KeyCode.h>

// use plugins if supported
VIMPORT IVisPlugin_cl* GetEnginePlugin_vFmodEnginePlugin();
#if defined( HAVOK_PHYSICS_2012_KEYCODE )
VIMPORT IVisPlugin_cl* GetEnginePlugin_vHavok();
#endif
#if defined( HAVOK_AI_KEYCODE )
VIMPORT IVisPlugin_cl* GetEnginePlugin_vHavokAi();
#endif
#if defined( HAVOK_BEHAVIOR_KEYCODE )
VIMPORT IVisPlugin_cl* GetEnginePlugin_vHavokBehavior();
#endif

//============================================================================================================
//  Set up the Plugin Class
//============================================================================================================
class CharlieNPCPlugin : public IVisPlugin_cl
{
public:

  void OnInitEnginePlugin();    
  void OnDeInitEnginePlugin();  

  const char *GetPluginName()
  {
    return "CharlieNPCPlugin";  // must match DLL name
  }
};

//  global plugin instance
CharlieNPCPlugin g_CharlieNPCComponents;

//--------------------------------------------------------------------------------------------
//  create a global instance of a VModule class
//  note: g_CharlieNPCModule is defined in stdfx.h
//--------------------------------------------------------------------------------------------
DECLARE_THIS_MODULE(g_CharlieNPCModule, MAKE_VERSION(1,0),
                    "Charlie NPC Plugin", 
                    "Havok",
                    "A sehaviour controller for NPC", &g_CharlieNPCComponents);


//--------------------------------------------------------------------------------------------
//  Use this to get and initialize the plugin when you link statically
//--------------------------------------------------------------------------------------------
VEXPORT IVisPlugin_cl* GetEnginePlugin_CharlieNPCPlugin(){  return &g_CharlieNPCComponents; }


#if (defined _DLL) || (defined _WINDLL)

  //  The engine uses this to get and initialize the plugin dynamically
  VEXPORT IVisPlugin_cl* GetEnginePlugin(){return GetEnginePlugin_CharlieNPCPlugin();}

#endif // _DLL or _WINDLL


//============================================================================================================
//  Initialize our plugin.
//============================================================================================================
//  Called when the plugin is loaded
//  We add our component initialize data here
void CharlieNPCPlugin::OnInitEnginePlugin()
{
  hkvLog::Info("CharlieNPCPlugin:OnInitEnginePlugin()");
  Vision::RegisterModule(&g_CharlieNPCModule);
  
// load plugins if supported
#if defined( HAVOK_PHYSICS_2012_KEYCODE )
  VISION_PLUGIN_ENSURE_LOADED(vHavok);
#endif
#if defined( HAVOK_AI_KEYCODE )
  VISION_PLUGIN_ENSURE_LOADED(vHavokAi);
#endif
#if defined( HAVOK_BEHAVIOR_KEYCODE )
  VISION_PLUGIN_ENSURE_LOADED(vHavokBehavior);
#endif
  VISION_PLUGIN_ENSURE_LOADED(vFmodEnginePlugin);
}

// Called before the plugin is unloaded
void CharlieNPCPlugin::OnDeInitEnginePlugin()
{
  hkvLog::Info("CharlieNPCPlugin:OnDeInitEnginePlugin()");
  Vision::UnregisterModule(&g_CharlieNPCModule);
}

/*
 * Havok SDK - Base file, BUILD(#20140423)
 * 
 * Confidential Information of Havok.  (C) Copyright 1999-2014
 * Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
 * Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
 * rights, and intellectual property rights in the Havok software remain in
 * Havok and/or its suppliers.
 * 
 * Use of this software for evaluation purposes is subject to and indicates
 * acceptance of the End User licence Agreement for this product. A copy of
 * the license is included with this software and is also available from salesteam@havok.com.
 * 
 */
