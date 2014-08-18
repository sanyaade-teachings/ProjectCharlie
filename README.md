ProjectCharlie
==============

Project Charlie is a suite of samples utilising common assets that demonstrate different approaches to incorporating Havok Animation Studio assets in a C++ environment.  
*__Important:__ The local Data folder is not held within this repository. Please use the RUN_ONCE.bat to create this directory from your own SDK.*


**v2014.0.5** *This sample was written for ProjectAnarchy version 2014.0.5 and requires minor migration adjustments for use with the current 2014.1.0 release until a complient upgrade is commited.*

---

Included is an Animation Studio project **(“CharlieHAT.hkp”)** which contains two identical behaviour graphs, one that incorporates a LUA control script and one that doesn’t.  
*Also included is a .pdf document that makes up part of an unrelated talk I have given to Universities and user groups but does provide a step-by-step coverage of how the CharlieHAT project was created from new.*

---

###Three vForge Scenes are supplied:

1.**CharlieWorldNoScript.scene**	A simple world containing a single Charlie character bound to the non-script behaviour.

2.**CharlieWorldScript.scene**	The same simple world containing a single Charlie character bound to the LUA scripted behaviour.

3.**CharlieWorldNPC.scene**		The simple world populated by multiple self-autonomous Charlie characters, all controlled by the 
CharlieNPCPlugin.vPlugin. Each Charlie character is based on a custom NPC class that exposes control values within vForge. This scene can be view with ‘Play-the-Game’

---

###Three Visual Studio solutions (Win32,Android ARM & Android x86) are supplied, each containing four projects:

1.**CharlieNoScriptApp**		A runtime application that loads the CharlieWorldNoScript.scene and demonstrates controlling a behaviour graph entirely via C++ API calls.

2.**CharliePartScript**		A runtime application based upon the CharlieWorldScript.scene that demonstrates reading control inputs and feeding an embedded LUA control script in order to influence the behaviour graph.

3.**CharlieNPCPlugin**		The controlling plugin character class that controls a non-scripted behaviour graph via a simple NPC AI state machine controller.

4.**ProjectCharlieApp**		The runtime version of CharlieWorldNPC.scene that incorporates the CharlieNPCPlugin.vplugin and demonstrates totally self-autonomous NPC behaviour of multiple characters, but allowing for external access to the custom entities from the main App.
