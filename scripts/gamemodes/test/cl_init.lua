GM.ShakeData = nil

function GM:OnFrame(elapsedTime)
	local playerPosition = engine_GetPlayerPosition(0)
	if (playerPosition) then
		local cameraCenter = Vec2(-640, -320) + playerPosition
		--cameraCenter.x = math.clamp(cameraCenter.x, 0, 30000)
		--cameraCenter.y = math.clamp(cameraCenter.y, -1000, 2000 - engine_GetCameraViewport().y)

		local shakeData = self.ShakeData
		if (shakeData) then
			cameraCenter = cameraCenter + Vec2(math.random(-1, 1), math.random(-1, 1)) * shakeData.Strength
			shakeData.Strength = shakeData.Strength - shakeData.StrengthDecrease * elapsedTime
			if (shakeData.Strength < 0) then
				self.ShakeData = nil
			end
		end

		engine_SetCameraPosition(cameraCenter)
	end
end

function GM:OnInit()
end

function GM:OnTick()
end

function GM:OnChangeLayer(oldLayer, newLayer)
	-- FIXME: This shouldn't be handled by this callback

	if (oldLayer ~= NoLayer) then
		for _, ent in pairs(match.GetEntitiesByClass("entity_visible_layer", oldLayer)) do
			ent:OnLeaveLayer(oldLayer)
		end
	end

	if (newLayer ~= NoLayer) then
		for _, ent in pairs(match.GetEntitiesByClass("entity_visible_layer", newLayer)) do
			ent:OnEnterLayer(newLayer)
		end
	end
end

function GM:ShakeCamera(duration, strength)
	self.ShakeData = {
		Strength = strength,
		StrengthDecrease = strength / duration
	}
end
