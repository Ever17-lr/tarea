// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Player.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Physics2D/Collider2D.hpp>
#include <NDK/Components.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/MatchClientVisibility.hpp>
#include <CoreLib/Terrain.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Scripting/ServerGamemode.hpp>

namespace bw
{
	Player::Player(MatchClientSession& session, Nz::UInt8 playerIndex, std::string playerName) :
	m_layerIndex(std::numeric_limits<std::size_t>::max()),
	m_inputIndex(0),
	m_name(std::move(playerName)),
	m_playerIndex(playerIndex),
	m_session(session),
	m_shouldSendWeapons(false)
	{
	}

	Player::~Player()
	{
		if (m_match)
			m_match->Leave(this);
	}

	bool Player::GiveWeapon(std::string weaponClass)
	{
		if (!m_match)
			return false;

		if (!m_playerEntity)
			return false;

		if (HasWeapon(weaponClass))
			return false;

		Terrain& terrain = m_match->GetTerrain();
		Ndk::World& world = terrain.GetLayer(m_layerIndex).GetWorld();

		ServerWeaponStore& weaponStore = m_match->GetWeaponStore();

		// Create weapon
		if (std::size_t weaponEntityIndex = weaponStore.GetElementIndex(weaponClass); weaponEntityIndex != ServerEntityStore::InvalidIndex)
		{
			const Ndk::EntityHandle& weapon = weaponStore.InstantiateWeapon(world, weaponEntityIndex, {}, m_playerEntity);
			if (!weapon)
				return false;

			std::size_t weaponIndex = m_weapons.size();
			m_weapons.emplace_back(weapon);
			m_weaponByName.emplace(std::move(weaponClass), weaponIndex);

			m_shouldSendWeapons = true;
		}

		return true;
	}

	void Player::Spawn()
	{
		if (!m_match)
			return;

		Terrain& terrain = m_match->GetTerrain();
		Ndk::World& world = terrain.GetLayer(m_layerIndex).GetWorld();

		ServerEntityStore& entityStore = m_match->GetEntityStore();
		if (std::size_t entityIndex = entityStore.GetElementIndex("entity_burger"); entityIndex != ServerEntityStore::InvalidIndex)
		{
			Nz::Vector2f spawnPosition = m_match->GetGamemode()->ExecuteCallback("ChoosePlayerSpawnPosition").as<Nz::Vector2f>();
			const Ndk::EntityHandle& playerEntity = entityStore.InstantiateEntity(world, entityIndex, spawnPosition, 0.f, {});
			if (!playerEntity)
				return;

			std::cout << "[Server] Creating player entity #" << playerEntity->GetId() << std::endl;

			playerEntity->AddComponent<InputComponent>();
			playerEntity->AddComponent<PlayerControlledComponent>(CreateHandle());

			if (playerEntity->HasComponent<HealthComponent>())
			{
				auto& healthComponent = playerEntity->GetComponent<HealthComponent>();

				healthComponent.OnDied.Connect([ply = CreateHandle()](const HealthComponent* health, const Ndk::EntityHandle& attacker)
				{
					if (!ply)
						return;

					ply->OnDeath(attacker);
				});
			}

			UpdateControlledEntity(playerEntity);

			if (!GiveWeapon("weapon_sword_emmentalibur"))
				std::cout << "Failed to give weapon" << std::endl;

			GiveWeapon("weapon_rifle");
		}
	}

	void Player::SelectWeapon(std::size_t weaponIndex)
	{
		assert(weaponIndex == NoWeapon || weaponIndex < m_weapons.size());

		if (m_weaponIndex == weaponIndex || !m_playerEntity)
			return;

		if (m_weaponIndex != NoWeapon)
		{
			auto& weapon = m_weapons[m_weaponIndex]->GetComponent<WeaponComponent>();
			weapon.SetActive(false);
		}

		m_weaponIndex = weaponIndex;
		if (m_weaponIndex != NoWeapon)
		{
			auto& weapon = m_weapons[m_weaponIndex]->GetComponent<WeaponComponent>();
			weapon.SetActive(true);
		}

		Packets::EntityWeapon weaponPacket;
		weaponPacket.entityId = m_playerEntity->GetId();
		weaponPacket.weaponEntityId = (m_weaponIndex != NoWeapon) ? m_weapons[m_weaponIndex]->GetId() : 0xFFFFFFFF;

		Nz::Bitset<Nz::UInt64> entityIds;
		entityIds.UnboundedSet(weaponPacket.entityId);

		if (weaponPacket.weaponEntityId != 0xFFFFFFFF)
			entityIds.UnboundedSet(weaponPacket.weaponEntityId);

		m_match->ForEachPlayer([&](Player* ply)
		{
			MatchClientSession& session = ply->GetSession();
			session.GetVisibility().PushEntitiesPacket(entityIds, weaponPacket);
		});
	}

	std::string Player::ToString() const
	{
		return "Player(" + m_name + ")";
	}

	void Player::OnTick(bool lastTick)
	{
		if (lastTick && m_shouldSendWeapons)
		{
			Packets::PlayerWeapons weaponPacket;
			weaponPacket.playerIndex = m_playerIndex;

			Nz::Bitset<Nz::UInt64> weaponIds;
			for (const Ndk::EntityHandle& weapon : m_weapons)
			{
				assert(weapon);

				weaponPacket.weaponEntities.emplace_back(weapon->GetId());
				weaponIds.UnboundedSet(weapon->GetId());
			}

			m_session.GetVisibility().PushEntitiesPacket(std::move(weaponIds), std::move(weaponPacket));

			m_shouldSendWeapons = false;
		}

		if (auto& inputOpt = m_inputBuffer[m_inputIndex])
		{
			UpdateInputs(inputOpt.value());
			inputOpt.reset();
		}

		if (++m_inputIndex >= m_inputBuffer.size())
			m_inputIndex = 0;
	}

	void Player::UpdateControlledEntity(const Ndk::EntityHandle& entity)
	{
		m_playerEntity = entity;

		Packets::ControlEntity controlEntity;
		controlEntity.entityId = (entity) ? static_cast<Nz::UInt32>(entity->GetId()) : 0;
		controlEntity.playerIndex = m_playerIndex;

		m_session.GetVisibility().PushEntityPacket(controlEntity.entityId, controlEntity);
	}

	void Player::UpdateInputs(const PlayerInputData& inputData)
	{
		if (m_playerEntity)
		{
			assert(m_playerEntity->HasComponent<InputComponent>());

			auto& inputComponent = m_playerEntity->GetComponent<InputComponent>();
			inputComponent.UpdateInputs(inputData);
		}
	}

	void Player::UpdateInputs(std::size_t tickDelay, PlayerInputData inputData)
	{
		assert(tickDelay < m_inputBuffer.size());
		std::size_t index = (m_inputIndex + tickDelay) % m_inputBuffer.size();

		m_inputBuffer[index] = std::move(inputData);
	}

	void Player::OnDeath(const Ndk::EntityHandle& attacker)
	{
		Packets::ChatMessage chatPacket;
		if (attacker && attacker->HasComponent<PlayerControlledComponent>())
			chatPacket.content = attacker->GetComponent<PlayerControlledComponent>().GetOwner()->GetName() + " killed " + GetName();
		else
			chatPacket.content = GetName() + " suicided";

		m_match->ForEachPlayer([&](Player* otherPlayer)
		{
			otherPlayer->SendPacket(chatPacket);
		});

		m_match->GetGamemode()->ExecuteCallback("OnPlayerDeath", CreateHandle(), attacker);

		m_weapons.clear();
		m_weaponByName.clear();
		m_weaponIndex = NoWeapon;
	}

	void Player::UpdateLayer(std::size_t layerIndex)
	{
		m_layerIndex = layerIndex;
	}

	void Player::UpdateMatch(Match* match)
	{
		m_match = match;
	}
}
