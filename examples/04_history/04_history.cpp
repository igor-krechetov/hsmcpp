#include <chrono>
#include <thread>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include "gen/PlayerHsmBase.hpp"

using namespace hsmcpp;

class PlayerHsm: public PlayerHsmBase
{
public:
    virtual ~PlayerHsm(){}

// HSM state changed callbacks
protected:
    void onCall(const VariantVector_t& args) override
    {
        printf("onCall\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        transition(PlayerHsmEvents::CALL_ENDED);
    }

    void onNoMedia(const VariantVector_t& args) override
    {
        printf("onNoMedia\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        transition(PlayerHsmEvents::LOADING_DONE);
    }

    void onPaused(const VariantVector_t& args) override
    {
        printf("onPaused\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        transition(PlayerHsmEvents::ON_CALL);
    }

    void onPlaying(const VariantVector_t& args) override
    {
        printf("onPlaying\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        transition(PlayerHsmEvents::PAUSE);
    }

// HSM transition callbacks
protected:
    void onCallEndedTransition(const VariantVector_t& args) override
    {
        printf("onCallEndedTransition\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
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