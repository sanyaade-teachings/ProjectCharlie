/***********************************************************************************************************************************************
*
*	The Core NPC Plugin Class
*
*	Extends CharlieNPC_cl from VisBaseEntity_cl and provides a self automated control.
*	Cause the CharlieNoScript behaviour to idle and navigate between random nodes of a pre-defined path
*
*	Requires Havok Behavior Character Component set to CharlieHAT.hkt / CharlieNoScript.hkt / NoScriptBehaviour.hkt
*	Exposes:
*		m_PathName:		the Key of the pre-defined path. default = "NodePath"
*		m_fMaxSpeed:	Maximum possible speed this instance can attain. default = 0.9f
*		m_fAcceleration Speed ramping acceleration factor described a seconds 0-1.f. default =1.5f
*		m_fDecceleration Speed ramping decceleration factor described a seconds 0-1.f. default =1.5f
*
*
*
*
***********************************************************************************************************************************************/


#include "CharlieNPCPluginPCH.h"
#include "NPCObject.h"


V_IMPLEMENT_SERIAL(CharlieNPC_cl, VisBaseEntity_cl, 0, &g_CharlieNPCModule);

START_VAR_TABLE (CharlieNPC_cl, VisBaseEntity_cl, "Charlie NPC class", 0, "")
	DEFINE_VAR_STRING(CharlieNPC_cl, m_PathName, "Path Key Name", "NodePath",128, 0, 0);
	DEFINE_VAR_FLOAT(CharlieNPC_cl, m_fMaxSpeed, "Maximum walk speed (0.5 - 1.0)", "0.9", 0, "Clamp(0.3,1.0)");
	DEFINE_VAR_FLOAT(CharlieNPC_cl, m_fAcceleration, "Speed acceleration (0-max in seconds)", "1.5", 0, "Clamp(0.5,3.0)");
	DEFINE_VAR_FLOAT(CharlieNPC_cl, m_fDecceleration, "Speed decceleration (0-max in seconds)", "1.5", 0, "Clamp(0.5,3.0)");
END_VAR_TABLE

CharlieNPC_cl::CharlieNPC_cl()
:	m_pBehaviourComponent(NULL)
,	m_speed(0)
,	m_targetSpeed(0)
,	m_turn(0)
,	m_twist(0)
,	m_status(NPC_Thinking)
,	m_nextStatus(NPC_Thinking)
,	m_fIdleTime(0.f)
,	m_fMaxSpeed(0.9f)
,	m_fAcceleration(ACCELERATION_TIME)
,	m_fDecceleration(DECCELERATION_TIME)
,	m_numNodes(0)
,	m_bInitialised(false)
{
	m_PathName[0]=0;
}




//**********************************************************	CORE NPC FUNCTIONALITY	********************************************************


/***********************************************************************************************************************************************
*
*	void CharlieNPC_cl::Update(float dt)
*
*	Core update function, called every frame from the base ThinkFunction;
*
*	Input: float dt - current frame delta time in seconds.
*
*		Updates character velocities
*		Performs all state machine functionality;
*
*	returns: void
*
*
***********************************************************************************************************************************************/
void CharlieNPC_cl::Update(float dt)
{
	UpdateSpeed(dt);														// update the charcter velocity
	switch (m_status)														// from state machine determine function:
	{
		case NPC_Moving:													// Keep character moving toward target
			MoveTowards();
			break;
		case NPC_Idling:													// decrement idle time and choose new task when ready
			m_fIdleTime-=dt;						
			if(m_fIdleTime<0.f)
			{
				m_status=NPC_Thinking;
			}
			break;
		case NPC_Turning:													//Wait until behavior graph exits the turning state then start moving
			if (m_pBehaviourComponent->GetWordVar("CurrentState") != HAT_Turn)
			{
				SetStatus(NPC_Moving);
			}
			break;
		case NPC_Stopping:													//Wait until current velocity drops below minimum threshold then start idling
			if (m_speed < MIN_STOP_SPEED)
			{
				SetStatus(NPC_Idling);
			}
			break;
		case NPC_Thinking:													//Otherwise we choose a new task and destination
		default:
			ChooseNewTarget();
			break;
	}
}

/***********************************************************************************************************************************************
*
*	void CharlieNPC_cl::SetStatus(NPCStates status)
*
*	Input: NPCStates status - the new state we want to enter into
*		Compares new state against existing state and performs, or ignores, appropriate tasks to achieve the new state
*
*	Return: void
*
*
***********************************************************************************************************************************************/
void CharlieNPC_cl::SetStatus(NPCStates status)
{
float	forwardDot;
float	rightDot;
	switch (status)															// based on the NEW state
	{
		case NPC_Moving:													// character needs to Move...
			if(m_status == NPC_Moving)	break;								// already Moving?, then do nothing

			if(m_status == NPC_Thinking)									// only test for a turn if this is destination set by 'thinking'
			{
				GetTargetDotProducts(forwardDot,rightDot);					// fetch the dot products of our forward and right vectors
				forwardDot = 1.f-forwardDot;								// translate into positive semicircle positive (2.0f == 180°)
				if(forwardDot >0.75f)										// angle delta > 67.5°, we need to use a turn tranistion
				{
					if(forwardDot > 1.5f)									// angle delta > 135°, lets do a full jump and 180° turn
					{														// so set the TurnType variable to TURN_JUMP
						m_pBehaviourComponent->SetWordVar("TurnType",TURN_Jump);
					}
					else													// other wise its just the normal 90° shuffle
					{
						if (rightDot<0.f)									// so do we need to turn right??
						{													// so set the TurnType cariable to TURN_RIGHT
							m_pBehaviourComponent->SetWordVar("TurnType",TURN_Right);
						}
						else												// if not must be left!
						{													// so set the TurnType cariable to TURN_LEFT
							m_pBehaviourComponent->SetWordVar("TurnType",TURN_Left);
						}
					}
					m_pBehaviourComponent->TriggerEvent("Idle_to_Turn");	// and then fire the turn event to force the animation to play
					m_status = NPC_Turning;
					break;
				}
			}
			m_pBehaviourComponent->TriggerEvent("Idle_to_Move");			// otherwise fire the standard move event and set our character walking
			m_status = NPC_Moving;
			break;
		case NPC_Idling:													// character wants to Idle
			if(m_status == NPC_Idling)	break;								// already Idling?, ignore.
			m_pBehaviourComponent->TriggerEvent("Move_to_Idle");			// else just fire the idle event to stop our character where he stands
			m_status = NPC_Idling;
			break;
		case NPC_Stopping:													// character needs to Stop
			m_targetSpeed=0.f;												// 0.0f is our target velocity
			m_status = NPC_Stopping;										// status indicates keep current animation, but deccelarate until stopped.
			break;
		default:
			m_status=NPC_Thinking;											// other-wise we must be Thinking. 
			break;
	}
}

/***********************************************************************************************************************************************
*
*	void CharlieNPC_cl::MoveTowards()

	Performs all steering and navigation logic to ambulate our character to its destination target.

		Calculates distance from character to target.
		Tests proximity to target and if within range causes next task to be performed
		Slows down characters velocity if getting close..
		Fetchs dot products of forward and right vectors, and calculates relevant turn and twist values
		Sets behavior variables 'turn' and 'twist' to their repective values
*
*
***********************************************************************************************************************************************/
void CharlieNPC_cl::MoveTowards()
{
	float dist =GetTargetDistance();										// fetch absolute distance from chracter to target position
	if(dist < NEAR_TARGET_DISTANCE)											// are we near enough??
	{
		SetStatus(m_nextStatus);											// then do the next thing as decided by previous 'Think'
		return;
	}
	if(dist < MIN_RUN_DISTANCE)												// are we getting close?
	{
		m_targetSpeed=hkvMath::Min(0.5f,m_targetSpeed);						// slow character down to a maximum walk speed
	}

	float	forwardDot,rightDot;
	{
		GetTargetDotProducts(forwardDot,rightDot);							// fetch forward and right vector dot products
		m_turn = hkvMath::Min(0.9f,(1.f-forwardDot));						// translate forward dot into range 0.0f - 2.0f (representing 180° in either direction) and clamp at 0.9f, the maximum turn factor
		m_turn = m_turn*((rightDot<0.f)?-1.f:1.f);							// use the right vector to sign our turn factor
		m_twist = m_turn*TWIST_ANGLE;										// finaly scale by the Twist Angle to calculate the twist factor..
	}

	m_pBehaviourComponent->SetFloatVar("turn",m_turn);						// Set the behavior variables
	m_pBehaviourComponent->SetFloatVar("twist",m_twist);
}


/***********************************************************************************************************************************************
*
*	void CharlieNPC_cl::ChooseNewTarget()
*
*	Randomly chooses a node on the pre-defined path and ensures new target is not too close to current position
*	Chooses a new random velocity for character
*	adjusts velocity to reflect distance that needs to be travelled
*	chooses task to perform upon reaching target, and how long to stop for if applicable
*	sets character state to Moving
*
*
***********************************************************************************************************************************************/
void CharlieNPC_cl::ChooseNewTarget()
{
	float	distance;	
	int		tests=4;														// maximum numbers of passes through node choice before aborting. Prevents endless looping if no nodes far enough away....
	do 
	{
		m_v3Target = m_v3Nodes[m_random.GetInt()%m_numNodes];				// choose a node on the path
		distance = GetTargetDistance();										// and get its absolute distance

	} while ((--tests >0) && (distance<MIN_NODE_DISTANCE));					// repeat until far enough away from current position or we hit endless loop protection limit

	m_targetSpeed = m_random.GetFloat();									// chooses a speed
	m_targetSpeed = hkvMath::Max(0.4f,m_targetSpeed);						// clamp to minmum of a slow walk
	m_targetSpeed += (distance/20000.f);									// add a function of the distance in
	m_targetSpeed = hkvMath::Min(m_fMaxSpeed,m_targetSpeed);				// ands clamp to the chararcters maximum speed
	m_nextStatus = (m_random.GetInt()%2==0)?NPC_Stopping:NPC_Thinking;		// when we get there we will either Stop or choose an immediate new target
	m_fIdleTime = 1.f+(m_random.GetFloat()*5.f);							// if we do stop, how long for...
	SetStatus(NPC_Moving);													// set character in motion
}

/***********************************************************************************************************************************************
*
*	void CharlieNPC_cl::GetTargetDotProducts(float& forwardDot, float& rightDot)
*
*	Input:	float& forwardDot - reference to float to place forward vector result into
*			float& rightDot - reference to float to place right vector result into
*
*	Generates the characters forward and right vectors.
*	Calculates the dot products of the forward and right vectors aginst a vector to the target point.
*	
*	Return:	void with results placed in inward references.
*
*
***********************************************************************************************************************************************/
void CharlieNPC_cl::GetTargetDotProducts(float& forwardDot, float& rightDot)
{
	const hkvMat3 rotMat = GetWorldMatrix().getRotationalPart();			// fetch characters rotation matrix
	hkvVec3 forward = FORWARD_VECTOR;										// create the forward vector
	hkvVec3 right = RIGHT_VECTOR;											// and the right...
	forward = rotMat.transformDirection(forward);							// transform the forward vector to character rotation
	right = rotMat.transformDirection(right);								// and the right vector....
	hkvVec3 target=m_v3Target-GetPosition();								// calculate the vector from character to target
	target.normalize();														// normalise...
	forwardDot = target.dot(forward);										// and get dot products
	rightDot = target.dot(right);											//
}

/***********************************************************************************************************************************************
*
*	float CharlieNPC_cl::GetTargetDistance()
*
*	Return: absolute distance between character and target position.
*
***********************************************************************************************************************************************/
float CharlieNPC_cl::GetTargetDistance()
{
	hkvVec3 target=m_v3Target-GetPosition();
	return(target.getLength());
}

/***********************************************************************************************************************************************
*
*	void CharlieNPC_cl::UpdateSpeed(float dt)
*
*	Input: float dt - the current frame delta time in seconds.
*
*	Accelerates or deccelarates the characters velocity to match the target velocity.
*
*	Return: void
*
***********************************************************************************************************************************************/
void CharlieNPC_cl::UpdateSpeed(float dt)
{
	if (m_speed == m_targetSpeed)	return;									// on target?? do nothing
	if (m_speed < m_targetSpeed)											// below target speed??
	{
		m_speed +=(dt/m_fAcceleration);										// accelerate and clamp if required.
		if (m_speed > m_targetSpeed)	m_speed = m_targetSpeed;
	}
	else																	// must be too fast
	{
		m_speed -= (dt/m_fDecceleration);									// deccelerate and clamp if required.
		if (m_speed < m_targetSpeed)	m_speed = m_targetSpeed;
	}
	m_pBehaviourComponent->SetFloatVar("speed",m_speed);					// set the 'speed' variable in the behavior component
}

/***********************************************************************************************************************************************
*
*	void CharlieNPC_cl::GetDebugData(debugData& data)
*
*	Input: debugData& data  - a reference to a debugdata structure to be filled in by the function.
*	
*	A Debug Getter function to allow external classes to examine Behavior variable states
*
*	Return: void - data structure filled in.
*
*
***********************************************************************************************************************************************/
void CharlieNPC_cl::GetDebugData(debugData& data)
{
	data.speed = m_pBehaviourComponent->GetFloatVar("speed");
	data.turn = m_pBehaviourComponent->GetFloatVar("turn");
	data.twist = m_pBehaviourComponent->GetFloatVar("twist");
	data.state = m_pBehaviourComponent->GetWordVar("CurrentState");
	data.turnType = m_pBehaviourComponent->GetWordVar("TurnType");
}

/***********************************************************************************************************************************************
*
*	void CharlieNPC_cl::GetPathNodes()
*
*	A private local function to examine and build an array of node positions from the specified Path
*
*	Return: void
*
***********************************************************************************************************************************************/
void CharlieNPC_cl::GetPathNodes()
{
	VisPath_cl*	pNodes;
	pNodes = Vision::Game.SearchPath(m_PathName);
	if(pNodes)
	{
		m_numNodes = pNodes->GetPathNodeCount();
		for (int i=0;i<m_numNodes;i++)
		{
			const VisPathNode_cl* pNode=pNodes->GetPathNode(i);
			m_v3Nodes.Add(pNode->GetPosition());
		}
	}
}

//**********************************************************	BASE CLASS FUNCTIONALITY	********************************************************


/***********************************************************************************************************************************************
*	void CharlieNPC_cl::ThinkFunction()
*
*	Core function called every frame by Vision. Used to perform all control over our character
*
*	One time initialisation performed here rather than during creation due to dependancies over other serialised objects and indeterministic creation order
*		Fetchs a pointer to the attached Havok Behavior Character Component
*		Validates the pointers (Debug only)
*		Fetched Node information from the declared path
*		Randomises member VRandom object with the y-position - This maintains deterministic behaviour, but re-distributes the seeding between all the NPC's
*
*	Each frame call update with the current frame delta time (in seconds)
*
*	Returns: void
*
***********************************************************************************************************************************************/
void CharlieNPC_cl::ThinkFunction()
{
	if(!m_bInitialised)
	{
		m_pBehaviourComponent = static_cast<vHavokBehaviorComponent *>(Components().GetComponentOfType(V_RUNTIME_CLASS(vHavokBehaviorComponent)));
			VASSERT(m_pBehaviourComponent);
			VASSERT(m_pBehaviourComponent->m_character);
		GetPathNodes();
		m_random.Reset((int)GetPosition().y);

		m_bInitialised=true;
		return;
	}
	Update(Vision::GetTimer()->GetTimeDifference());
}

/***********************************************************************************************************************************************
*
*	void CharlieNPC_cl::Serialize(VArchive& ar)
*
*	Input: VArchive& ar - reference to Varchive object used for serialisation purposes.
*
*	Serialisation function extending the base class serialize(ar) function
*
*	Reads and write the exposed member variables as part of the serialised object

*	Return: void
*
***********************************************************************************************************************************************/

void CharlieNPC_cl::Serialize(VArchive& ar)
{
	VisBaseEntity_cl::Serialize(ar);
	if(ar.IsLoading())
	{
		ar.ReadStringBinary(m_PathName, sizeof(m_PathName));
		ar >> m_fMaxSpeed;
		ar >> m_fAcceleration;
		ar >> m_fDecceleration;
	}
	else
	{
		ar.WriteStringBinary(m_PathName);
		ar << m_fMaxSpeed;
		ar << m_fAcceleration;
		ar << m_fDecceleration;
	}
}


/*
 * Havok SDK - Base file, BUILD(#20131218)
 * 
 * Confidential Information of Havok.  (C) Copyright 1999-2013
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


