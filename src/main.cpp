namespace ObScript
{
	namespace QuitGame
	{
		inline static constexpr auto LONG_NAME{ "QuitGame"sv };

		static bool IsInModule(std::uintptr_t a_ptr)
		{
			auto& mod = REL::Module::get();
			auto seg = mod.segment(REL::Segment::textx);
			auto end = seg.address() + seg.size();
			return (seg.address() < a_ptr) && (a_ptr < end);
		}

		static bool Execute(const RE::SCRIPT_PARAMETER*, RE::SCRIPT_FUNCTION::ScriptData*, RE::TESObjectREFR*, RE::TESObjectREFR*, RE::Script*, RE::ScriptLocals*, double&, std::uint32_t&)
		{
			std::thread{
				[]
				{
					RE::ConsoleLog::GetSingleton()->Print("Bye.");
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					RE::Main::GetSingleton()->quitGame = true;
				}
			}.detach();

			return true;
		}

		static void Install()
		{
			if (auto function = RE::SCRIPT_FUNCTION::LocateConsoleCommand(LONG_NAME))
			{
				if (IsInModule(reinterpret_cast<std::uintptr_t>(function->executeFunction)))
				{
					function->executeFunction = Execute;
					SKSE::log::info("Registered function {:s}"sv, LONG_NAME);
				}
				else
				{
					SKSE::log::warn("{:s} function is already overridden!"sv, LONG_NAME);
				}
			}
			else
			{
				SKSE::log::error("Failed to locate function {:s}."sv, LONG_NAME);
			}
		}
	}

	static void Install()
	{
		QuitGame::Install();
	}
}

namespace
{
	static void MessageCallback(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type)
		{
		case SKSE::MessagingInterface::kPostLoad:
		{
			ObScript::Install();
			break;
		}
		default:
			break;
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SKSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
