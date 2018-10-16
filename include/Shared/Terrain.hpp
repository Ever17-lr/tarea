// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_TERRAIN_HPP
#define BURGWAR_SHARED_TERRAIN_HPP

#include <Shared/TerrainLayer.hpp>
#include <vector>

namespace bw
{
	class Terrain
	{
		public:
			Terrain(std::size_t layerCount = 1);
			Terrain(const Terrain&) = delete;
			~Terrain() = default;

			inline TerrainLayer& GetLayer(std::size_t layerIndex);
			inline const TerrainLayer& GetLayer(std::size_t layerIndex) const;
			inline std::size_t GetLayerCount() const;

			void Update(float elapsedTime);

			Terrain& operator=(const Terrain&) = delete;

		private:
			std::vector<TerrainLayer> m_layers; //< Shouldn't resize because of raw pointer in Player
	};
}

#include <Shared/Terrain.inl>

#endif
