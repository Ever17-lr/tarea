// Copyright (C) 2018 J�r�me Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/MatchSessions.hpp>

namespace bw
{
	template<typename T, typename ...Args>
	T* MatchSessions::CreateSessionManager(Args&&... args)
	{
		m_managers.emplace_back(std::make_unique<T>(this, std::forward<Args>(args)...));
		return static_cast<T*>(m_managers.back().get());
	}

	template<typename F>
	void MatchSessions::ForEachSession(F&& cb)
	{
		for (const auto& pair : m_sessionIdToSession)
			cb(pair.second);
	}
}
