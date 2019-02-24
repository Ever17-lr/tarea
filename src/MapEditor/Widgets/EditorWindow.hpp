// Copyright (C) 2019 J�r�me Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOW_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOW_HPP

#include <NDK/Prerequisites.hpp>
#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Map.hpp>
#include <QtWidgets/QMainWindow>
#include <tsl/hopscotch_map.h>
#include <filesystem>
#include <memory>
#include <optional>

class QAction;
class QListWidget;
class QListWidgetItem;
class Gamemode;

namespace bw
{
	class ClientScriptingContext;
	class MapCanvas;

	class EditorWindow : public BurgApp, public QMainWindow
	{
		public:
			EditorWindow();
			~EditorWindow();

			void ClearWorkingMap();

			inline const ClientEntityStore& GetEntityStore() const;

			void UpdateWorkingMap(Map map, std::filesystem::path mapPath = std::filesystem::path());

		private:
			void BuildMenu();
			void OnCompileMap();
			void OnCreateEntity();
			void OnCreateMap();
			void OnEntityDoubleClicked(QListWidgetItem* item);
			void OnEntitySelectionUpdate();
			void OnLayerChanged(int layerIndex);
			void OnLayerDoubleClicked(QListWidgetItem* item);

			void OnOpenMap();
			void OnSaveMap();

			void RegisterEntity(std::size_t entityIndex);

			std::filesystem::path m_workingMapPath;
			std::optional<ClientEntityStore> m_clientEntityStore;
			std::optional<int> m_currentLayer;
			std::shared_ptr<ClientScriptingContext> m_scriptingContext;
			tsl::hopscotch_map<Ndk::EntityId /*canvasIndex*/, std::size_t /*entityIndex*/> m_entityIndexes;
			QAction* m_compileMap;
			QAction* m_createEntityAction;
			QAction* m_createEntityActionToolbar;
			QAction* m_saveMap;
			QAction* m_saveMapToolbar;
			QListWidget* m_entityList;
			QListWidget* m_layerList;
			Map m_workingMap;
			MapCanvas* m_canvas;
	};
}

#include <MapEditor/Widgets/EditorWindow.inl>

#endif