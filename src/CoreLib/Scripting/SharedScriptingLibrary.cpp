// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <CoreLib/SharedMatch.hpp>

namespace bw
{
	SharedScriptingLibrary::SharedScriptingLibrary(SharedMatch& sharedMatch) :
	AbstractScriptingLibrary(sharedMatch.GetLogger()),
	m_match(sharedMatch)
	{
	}

	SharedScriptingLibrary::~SharedScriptingLibrary() = default;

	void SharedScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState.open_libraries();

		RegisterGlobalLibrary(context);
		RegisterMatchLibrary(context, luaState.create_named_table("match"));
		RegisterMetatableLibrary(context);
		RegisterPhysicsLibrary(context, luaState.create_named_table("physics"));
		RegisterScriptLibrary(context, luaState.create_named_table("scripts"));
		RegisterTimerLibrary(context, luaState.create_named_table("timer"));
	}

	void SharedScriptingLibrary::RegisterMatchLibrary(ScriptingContext& context, sol::table& library)
	{
		library["GetCurrentTime"] = [this]()
		{
			return m_match.GetCurrentTime() / 1000.f;
		};

		library["GetEntitiesByClass"] = [&](sol::this_state s, const std::string& entityClass, std::optional<LayerIndex> layerIndexOpt)
		{
			sol::state_view state(s);
			sol::table result = state.create_table();

			std::size_t index = 1;
			auto entityFunc = [&](const Ndk::EntityHandle& entity)
			{
				if (!entity->HasComponent<ScriptComponent>())
					return;

				auto& entityScript = entity->GetComponent<ScriptComponent>();
				if (entityScript.GetElement()->fullName == entityClass)
					result[index++] = entityScript.GetTable();
			};

			if (layerIndexOpt)
			{
				LayerIndex layerIndex = layerIndexOpt.value();
				if (layerIndex >= m_match.GetLayerCount())
					throw std::runtime_error("Invalid layer index");

				m_match.GetLayer(layerIndex).ForEachEntity(entityFunc);
			}
			else
				m_match.ForEachEntity(entityFunc);

			return result;
		};

		library["GetTickDuration"] = [&]()
		{
			return m_match.GetTickDuration();
		};
	}

	void SharedScriptingLibrary::RegisterPhysicsLibrary(ScriptingContext& context, sol::table& library)
	{
		library["Trace"] = [this](sol::this_state L, LayerIndex layer, Nz::Vector2f startPos, Nz::Vector2f endPos) -> sol::object
		{
			if (layer >= m_match.GetLayerCount())
				throw std::runtime_error("Invalid layer index");

			Ndk::World& world = m_match.GetLayer(layer).GetWorld();
			auto& physSystem = world.GetSystem<Ndk::PhysicsSystem2D>();

			Ndk::PhysicsSystem2D::RaycastHit hitInfo;
			if (physSystem.RaycastQueryFirst(startPos, endPos, 1.f, 0, 0xFFFFFFFF, 0xFFFFFFFF, &hitInfo))
			{
				sol::state_view state(L);
				sol::table result = state.create_table();
				result["fraction"] = hitInfo.fraction;
				result["hitPos"] = hitInfo.hitPos;
				result["hitNormal"] = hitInfo.hitNormal;

				const Ndk::EntityHandle& hitEntity = hitInfo.body;
				if (hitEntity->HasComponent<ScriptComponent>())
					result["hitEntity"] = hitEntity->GetComponent<ScriptComponent>().GetTable();

				return result;
			}
			else
				return sol::nil;
		};
	}

	void SharedScriptingLibrary::RegisterScriptLibrary(ScriptingContext& context, sol::table& library)
	{
		// empty for now
	}

	void SharedScriptingLibrary::RegisterTimerLibrary(ScriptingContext& context, sol::table& library)
	{
		sol::state& state = context.GetLuaState();
		library["Create"] = [&](Nz::UInt64 time, sol::object callbackObject)
		{
			m_match.GetTimerManager().PushCallback(m_match.GetCurrentTime() + time, [this, &state, callbackObject]()
			{
				sol::protected_function callback(state, sol::ref_index(callbackObject.registry_index()));

				auto result = callback();
				if (!result.valid())
				{
					sol::error err = result;
					bwLog(GetLogger(), LogLevel::Error, "engine_SetTimer failed: {0}", err.what());
				}
			});
		};
	}
}
