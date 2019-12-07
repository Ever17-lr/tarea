// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedElementLibrary.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Utils.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <sol3/sol.hpp>

namespace bw
{
	SharedElementLibrary::~SharedElementLibrary() = default;

	void SharedElementLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		RegisterCommonLibrary(elementMetatable);
	}

	void SharedElementLibrary::RegisterCommonLibrary(sol::table& elementMetatable)
	{
		elementMetatable["GetDirection"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			Nz::Vector2f direction(nodeComponent.GetRotation() * Nz::Vector2f::UnitX());
			if (nodeComponent.GetScale().x < 0.f)
				direction = -direction;

			return direction;
		};

		elementMetatable["GetPosition"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.GetPosition());
		};

		elementMetatable["GetRotation"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::DegreeAnglef(AngleFromQuaternion(nodeComponent.GetRotation())); //<FIXME: not very efficient
		};

		elementMetatable["IsLookingRight"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return nodeComponent.GetScale().x > 0.f;
		};

		elementMetatable["GetProperty"] = [](sol::this_state s, const sol::table& table, const std::string& propertyName) -> sol::object
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(table);

			auto& properties = entity->GetComponent<ScriptComponent>();

			auto propertyVal = properties.GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				const EntityProperty& property = propertyVal.value();

				return TranslateEntityPropertyToLua(lua, property);
			}
			else
				return sol::nil;
		};
	}
}
