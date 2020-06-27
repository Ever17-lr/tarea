// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Audio/Audio.hpp>
#include <Nazara/Audio/SoundBuffer.hpp>
#include <Nazara/Core/AbstractLogger.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <Nazara/Network/Network.hpp>
#include <CoreLib/Utility/CrashHandler.hpp>
#include <Client/ClientApp.hpp>
#include <Main/Main.hpp>

int BurgMain(int argc, char* argv[])
{
	Nz::Initializer<Nz::Network> network;
	bw::ClientApp app(argc, argv);
	app.EnableFPSCounter(true);

	Nz::Audio::SetSpeedOfSound(343.3f * 100.f);

	Nz::SoundBufferParams params;
	params.forceMono = true;

	Nz::SoundBufferManager::SetDefaultParameters(params);

	return app.Run();
}
