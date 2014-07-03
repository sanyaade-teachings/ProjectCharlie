
MOVE_THRESHOLD = 0.1
TWIST_ANGLE=45.0
RIGHT_VEC = hkVector4.new(1,0,0,0)

-- helper table to store the state of the game pad.
g_Charlie = 
{	
	m_leftStickY = 0,
	m_leftStickX = 0,
	m_speed = 0,
	m_staticSpeed = 0,
	m_turn = 0,
	m_twist = 0,
}

function g_Charlie:update()
 	self.m_leftStickY = hkbGetVariable("LeftStickY")
	self.m_leftStickX = hkbGetVariable("LeftStickX")
	
	self.m_speed = math.sqrt(self.m_leftStickY*self.m_leftStickY + self.m_leftStickX*self.m_leftStickX)
	self.m_staticSpeed = math.max(0,self.m_leftStickY)
	self.m_turn = 0
	if (math.abs(self.m_leftStickY*self.m_leftStickX) > 0) then
		local vec = hkVector4.new(self.m_leftStickX,self.m_leftStickY,0,0)
		self.m_turn = vec:dot3(RIGHT_VEC)
	end
	self.m_twist = self.m_turn*TWIST_ANGLE
end

-- called every time the locomotion state is updated
function onCharlieIdle()
	
	g_Charlie:update()

	if ( g_Charlie.m_leftStickY > MOVE_THRESHOLD ) then
		hkbFireEvent("Idle_to_Move")

	elseif ( g_Charlie.m_leftStickY < -0.4 ) then
			hkbSetVariable("TurnType",2)
			hkbFireEvent("Idle_to_Turn")
	elseif ( math.abs(g_Charlie.m_leftStickX) > 0.2 ) then
		if (g_Charlie.m_leftStickX<0) then
			hkbSetVariable("TurnType",1)
		else
			hkbSetVariable("TurnType",0)
		end
		hkbFireEvent("Idle_to_Turn")

				
	end
end


function onCharlieMove()
	
	g_Charlie:update()

	if ( g_Charlie.m_speed < MOVE_THRESHOLD ) then
		hkbFireEvent("Move_to_Idle")
	else
		hkbSetVariable("speed",g_Charlie.m_speed)
		hkbSetVariable("turn",g_Charlie.m_turn)
		hkbSetVariable("twist",g_Charlie.m_twist)
	end
		
end



function onCharlieEvent()
	local eventName = hkbGetHandleEventName()
	if ( eventName == "Jump") then 
		if (g_Charlie.m_speed >0.5) then
			hkbSetVariable("JumpType",1)
		else
			hkbSetVariable("JumpType",0)
		end
		hkbFireEvent("to_Jump")
	end
end

