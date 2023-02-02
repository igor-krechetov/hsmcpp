#include <chrono>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include <thread>

#include "gen/PlayerHsmBase.hpp"

using namespace hsmcpp;

class PlayerHsm : public PlayerHsmBase {
public:
    virtual ~PlayerHsm() {}

    // HSM state changed callbacks
protected:
    void onCall(const VariantVector_t& args) override {
        (void)printf("onCall\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        transition(PlayerHsmEvents::CALL_ENDED);
    }

    void onNoMedia(const VariantVector_t& args) override {
        (void)printf("onNoMedia\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        transition(PlayerHsmEvents::LOADING_DONE);
    }

    void onPaused(const VariantVector_t& args) override {
        (void)printf("onPaused\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        transition(PlayerHsmEvents::ON_CALL);
    }

    void onPlaying(const VariantVector_t& args) override {
        (void)printf("onPlaying\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        transition(PlayerHsmEvents::PAUSE);
    }

    // HSM transition callbacks
protected:
    void onCallEndedTransition(const VariantVector_t& args) override {
        (void)printf("onCallEndedTransition\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
};

int main(const int argc, const char** argv) {
    std::shared_ptr<HsmEventDispatcherSTD> dispatcher = std::make_shared<HsmEventDispatcherSTD>();
    PlayerHsm hsm;

    (void)hsm.enableHsmDebugging("./04_history.hsmlog");

    if (true == hsm.initialize(dispatcher)) {
        hsm.transition(PlayerHsmEvents::START);

        dispatcher->join();
    }

    return 0;
}