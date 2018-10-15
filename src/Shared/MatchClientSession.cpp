// Copyright (C) 2018 J�r�me Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/MatchClientSession.hpp>
#include <Shared/PlayerCommandStore.hpp>
#include <Shared/NetworkReactor.hpp>
#include <iostream>

namespace bw
{
	void MatchClientSession::Disconnect()
	{
		m_bridge->Disconnect();
	}

	void MatchClientSession::HandleIncomingPacket(Nz::NetPacket&& packet)
	{
		m_commandStore.UnserializePacket(*this, std::move(packet));
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::HelloWorld& packet)
	{
		std::cout << "[Server] Hello world: " << packet.str << std::endl;
	}
}
