#include <chrono>
#include <thread>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include "gen/PlayerHsmBase.hpp"

using namespace std::chrono_literals;

class PlayerHsm: public PlayerHsmBase
{
public:
    virtual ~PlayerHsm(){}

// HSM state changed callbacks
protected:
    void onCall(const VariantList_t& args) override
    {
        printf("onCall\n");
        std::this_thread::sleep_for(1000ms);
        transition(PlayerHsmEvents::CALL_ENDED);
    }

    void onNoMedia(const VariantList_t& args) override
    {
        printf("onNoMedia\n");
        std::this_thread::sleep_for(1000ms);
        transition(PlayerHsmEvents::LOADING_DONE);
    }

    void onPaused(const VariantList_t& args) override
    {
        printf("onPaused\n");
        std::this_thread::sleep_for(3000ms);
        transition(PlayerHsmEvents::ON_CALL);
    }

    void onPlaying(const VariantList_t& args) override
    {
        printf("onPlaying\n");
        std::this_thread::sleep_for(3000ms);
        transition(PlayerHsmEvents::PAUSE);
    }

// HSM transition callbacks
protected:
    void onCallEndedTransition(const VariantList_t& args) override
    {
        printf("onCallEndedTransition\n");
        std::this_thread::sleep_for(250ms);
    }
};

int main(const int argc, const char**argv)
{
    std::shared_ptr<HsmEventDispatcherSTD> dispatcher = std::make_shared<HsmEventDispatcherSTD>();
    PlayerHsm hsm;

    hsm.enableHsmDebugging("./04_history.hsmlog");
    hsm.initialize(dispatcher);
    hsm.transition(PlayerHsmEvents::START);

    dispatcher->join();

    return 0;
}