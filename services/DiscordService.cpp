#include <iostream>
#include <thread>
#include "../third_party/discord-sdk-src/cpp/discord.h"

class DiscordService
{
    static DiscordService *instance;
    std::unique_ptr<discord::Core> core;
    bool started{false};
    std::thread *threadHandler;
    volatile bool interrupted{false};

    // Private constructor so that no objects can be created.
    DiscordService()
    {
    }

    bool Initialize()
    {
        discord::Core *aux{};
        auto result = discord::Core::Create(963884877428711434, DiscordCreateFlags_NoRequireDiscord, &aux);
        this->core.reset(aux);
        if (!this->core)
        {
            std::cout << "Failed to instantiate discord core! (err " << static_cast<int>(result)
                      << ")\n";
            return false;
        }

        this->core->SetLogHook(
            discord::LogLevel::Debug, [](discord::LogLevel level, const char *message)
            { std::cerr << "Log(" << static_cast<uint32_t>(level) << "): " << message << "\n"; });

        return true;
    }

    void Loop()
    {
        do
        {
            this->core->RunCallbacks();

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        } while (!interrupted);
    }

public:
    static DiscordService *getInstance()
    {
        if (!instance)
            instance = new DiscordService;
        return instance;
    }

    void UpdateActivity(std::string details, std::string state)
    {
        if (!Start())
            return;
        discord::Activity activity{};
        activity.SetDetails(const_cast<char *>(details.c_str()));
        activity.SetState(const_cast<char *>(state.c_str()));
        activity.GetAssets().SetSmallText("i mage");
        activity.GetAssets().SetLargeText("u mage");
        activity.SetType(discord::ActivityType::Playing);
        this->core->ActivityManager().UpdateActivity(activity, [](discord::Result result)
                                                     { 
                                                         if(result != discord::Result::Ok)
                                                         std::cout << "Failed updating activity!\n"; });
    }

    bool Start()
    {
        if (!this->core)
        {
            if (!Initialize())
                return false;
        }
        if (!started)
        {
            started = true;
            threadHandler = new std::thread(&DiscordService::Loop, this);
        }
        return true;
    }

    void Stop()
    {
        if (started)
        {
            interrupted = true;
            threadHandler->join();
            interrupted = false;
        }
    }
};

DiscordService *DiscordService::instance = 0;
