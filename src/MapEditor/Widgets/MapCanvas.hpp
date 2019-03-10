// Copyright (C) 2019 J�r�me Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_MAPWIDGET_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_MAPWIDGET_HPP

#include <CoreLib/EntityProperties.hpp>
#include <MapEditor/Gizmos/EditorGizmo.hpp>
#include <MapEditor/Widgets/WorldCanvas.hpp>
#include <NDK/World.hpp>
#include <memory>

namespace bw
{
	class EditorWindow;

	class MapCanvas : public WorldCanvas
	{
		public:
			MapCanvas(EditorWindow& editor, QWidget* parent = nullptr);
			~MapCanvas() = default;

			void ClearEntities();
			void ClearEntitySelection();

			const Ndk::EntityHandle& CreateEntity(const std::string& entityClass, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties);
			void DeleteEntity(Ndk::EntityId entityId);

			inline const Ndk::EntityList& GetMapEntities() const;

			void EditEntityPosition(Ndk::EntityId entityId);

			void UpdateEntityPositionAndRotation(Ndk::EntityId entityId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation);

			NazaraSignal(OnCanvasMouseButtonPressed, MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseButtonEvent& /*mouseButton*/);
			NazaraSignal(OnCanvasMouseButtonReleased, MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseButtonEvent& /*mouseButton*/);
			NazaraSignal(OnCanvasMouseEntered, MapCanvas* /*emitter*/);
			NazaraSignal(OnCanvasMouseLeft, MapCanvas* /*emitter*/);
			NazaraSignal(OnCanvasMouseMoved, MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseMoveEvent& /*mouseButton*/);
			NazaraSignal(OnEntityPositionUpdated, MapCanvas* /*emitter*/, Ndk::EntityId /*entityId*/, const Nz::Vector2f& /*position*/);

		private:
			void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseEntered() override;
			void OnMouseLeft() override;
			void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved) override;

			NazaraSlot(Ndk::Entity, OnEntityDestruction, m_onGizmoEntityDestroyed);

			std::unique_ptr<EditorGizmo> m_entityGizmo;
			EditorWindow& m_editor;
			Ndk::EntityList m_mapEntities;
	};
}

#include <MapEditor/Widgets/MapCanvas.inl>

#endif