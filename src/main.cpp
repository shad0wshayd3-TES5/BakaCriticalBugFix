namespace Hooks
{
	class hkQuitGame
	{
	private:
		static bool QuitGame()
		{
			if (auto log = RE::ConsoleLog::GetSingleton())
				log->Print("Bye.");

			std::thread(
				[]()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					if (auto main = RE::Main::GetSingleton())
						main->quitGame = true;
				})
				.detach();

			return true;
		}

	public:
		static void Install()
		{
			static REL::Relocation target{ REL::ID(22896) };
			target.replace_func(0x28, QuitGame);
		}
	};

	class hkShutdown
	{
	private:
		static void Shutdown()
		{
			REX::W32::TerminateProcess(REX::W32::GetCurrentProcess(), EXIT_SUCCESS);
		}

		inline static REL::THook _Shutdown{ REL::ID(36544), 0x1AE, Shutdown };
	};

	static void Install()
	{
		hkQuitGame::Install();
	}
}

namespace
{
	static void MessageCallback(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type)
		{
		case SKSE::MessagingInterface::kPostLoad:
			Hooks::Install();
			break;
		default:
			break;
		}
	}
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse, { .trampoline = true });
	SKSE::GetMessagingInterface()->RegisterListener(MessageCallback);
	return true;
}
