namespace ObScript
{
	class QuitGame
	{
	public:
		static void Install()
		{
			if (auto function = RE::SCRIPT_FUNCTION::LocateConsoleCommand(LONG_NAME))
			{
				if (IsInModule(reinterpret_cast<std::uintptr_t>(function->executeFunction)))
				{
					function->executeFunction = Execute;

					logger::info(FMT_STRING("Registered function {:s}"sv), LONG_NAME);
				}
				else
				{
					logger::error(FMT_STRING("{:s} function is already overridden!"sv), LONG_NAME);
				}
			}
			else
			{
				logger::error(FMT_STRING("Failed to locate function {:s}."sv), LONG_NAME);
			}
		}

	private:
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

		inline static constexpr std::string_view LONG_NAME{ "QuitGame"sv };
	};
}

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []()
{
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName(Version::PROJECT);
	v.AuthorName("shad0wshayd3");
	v.UsesAddressLibrary();
	v.UsesUpdatedStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	return v;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor())
	{
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39)
	{
		logger::critical(FMT_STRING("Unsupported runtime version {}"sv), ver.string());
		return false;
	}

	return true;
}
#endif

namespace
{
	void InitializeLog()
	{
		auto path = logger::log_directory();
		if (!path)
		{
			stl::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::info);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%^%l%$] %v"s);

		logger::info(FMT_STRING("{:s} v{:s}"), Version::PROJECT, Version::NAME);
	}
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info(FMT_STRING("{:s} loaded"), Version::PROJECT);

	SKSE::Init(a_skse);

	ObScript::QuitGame::Install();

	return true;
}
