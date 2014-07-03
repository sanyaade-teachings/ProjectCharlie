
#ifndef NPCOBJECT_H_INCLUDED
#define NPCOBJECT_H_INCLUDED

#include "CharlieNPCPluginModule.h"
#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/HavokBehaviorEnginePlugin.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorModule.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorComponent.hpp>
#include <Vision/Runtime/Engine/SceneElements/VisApiPath.hpp>
#include <Vision/Runtime/Base/Math/Random/VRandom.hpp>

static const float		TWIST_ANGLE=45.0;
static const float		ACCELERATION_TIME = 1.5f;
static const float		DECCELERATION_TIME = 1.5f;
static const float		MIN_STOP_SPEED = 0.2f;
static const float		NEAR_TARGET_DISTANCE = 200.f;
static const float		MIN_RUN_DISTANCE = 800.f;
static const float		MIN_NODE_DISTANCE = 2000.f;
static const hkvVec3	FORWARD_VECTOR(1.f,0.f,0.f);
static const hkvVec3	RIGHT_VECTOR(0.f,1.f,0.f);


class CharlieNPC_cl : public VisBaseEntity_cl
{
public:
	V_DECLARE_SERIAL_DLLEXP(CharlieNPC_cl, CHARLIEPLUGIN_IMPEXP);
	V_DECLARE_VARTABLE(CharlieNPC_cl, CHARLIEPLUGIN_IMPEXP);

	enum NPCStates
	{
		NPC_Thinking=0,
		NPC_Idling,
		NPC_Moving,
		NPC_Turning,
		NPC_Stopping
	};

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

	struct debugData
	{
		int		state;
		float	speed;
		float	turn;
		float	twist;
		int		turnType;
	};

public:
CHARLIEPLUGIN_IMPEXP	CharlieNPC_cl();

CHARLIEPLUGIN_IMPEXP	void	Update(float dt);
CHARLIEPLUGIN_IMPEXP	void	SetStatus(NPCStates status);
CHARLIEPLUGIN_IMPEXP	void	MoveTowards();
CHARLIEPLUGIN_IMPEXP	void	ChooseNewTarget();
CHARLIEPLUGIN_IMPEXP	void	GetDebugData(CharlieNPC_cl::debugData& data);
CHARLIEPLUGIN_IMPEXP	void	GetTargetDotProducts(float& forwardDot, float& rightDot);
CHARLIEPLUGIN_IMPEXP	void	UpdateSpeed(float dt);
CHARLIEPLUGIN_IMPEXP	float	GetTargetDistance();

protected:
						void	GetPathNodes();
						void	ThinkFunction()			HKV_OVERRIDE;
						void	Serialize(VArchive& ar) HKV_OVERRIDE;

private:
	vHavokBehaviorComponent*	m_pBehaviourComponent;					// ptr to the associated Havok Behavior Character Component
	float						m_speed;								// forward velocity - matches Behavior Graph Variable
	float						m_targetSpeed;							// target velocity
	float						m_turn;									// turn factor - matches Behavior Graph Variable
	float						m_twist;								// twist factor - matches Behavior Graph Variable
	NPCStates					m_status;								// NPC state machine state

	NPCStates					m_nextStatus;							// future state machine state, to be invoked on reaching target
	float						m_fIdleTime;							// time to spent Idling at target, if applicable

	float						m_fMaxSpeed;							// maximum velocity - exposed in vForge
	float						m_fAcceleration;						// acceleration time - exposed in vForge
	float						m_fDecceleration;						// decceleration time - exposed in vForge
	int							m_numNodes;								// number of nodes in associated vPath
	bool						m_bInitialised;							// one shot initialisation control

	char						m_PathName[128];						// Key name of associated vPath - exposed in vForge
	VArray<hkvVec3>				m_v3Nodes;								// cached array of nodes read from associated vPath
	hkvVec3						m_v3Target;								// the current target position
	VRandom						m_random;								// instance of random generator

};



#endif		//NPCOBJECT_H_INCLUDED