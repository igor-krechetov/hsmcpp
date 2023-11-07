#include <malloc.h>

#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include <hsmcpp/hsm.hpp>

using namespace hsmcpp;

namespace States {
    const hsmcpp::StateID_t OFF = 0;
    const hsmcpp::StateID_t ON = 1;
}

namespace Events {
    const hsmcpp::EventID_t SWITCH = 0;
}

long getAllocatedHeapMemory() {
    struct mallinfo mi = mallinfo();

    return mi.uordblks + mi.hblkhd;
}

int main(const int argc, const char** argv) {
    long memBefore;
    long memAfter;
    long memAfterEmpty;
    long memAfterStates;
    long memAfterTransitions;
    long memAfterTransitionExecute;

    printf("\nThis utility is approximately measuring heap memory consumption by hsmcpp library.\n");
    printf("------------------------------------------------------------------\n\n");

    //-------------------------------------------
    memBefore = getAllocatedHeapMemory();

    std::shared_ptr<HsmEventDispatcherSTD> dispatcher = HsmEventDispatcherSTD::create();
    std::shared_ptr<HierarchicalStateMachine> hsm = std::make_shared<HierarchicalStateMachine>(States::OFF);

    hsm->initialize(dispatcher);
    memAfterEmpty = getAllocatedHeapMemory();

    hsm->registerState(States::OFF, [&hsm](const VariantVector_t& args) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm->transition(Events::SWITCH);
    });
    hsm->registerState(States::ON, [&hsm](const VariantVector_t& args) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm->transition(Events::SWITCH);
    });
    memAfterStates = getAllocatedHeapMemory();

    hsm->registerTransition(States::OFF, States::ON, Events::SWITCH);
    hsm->registerTransition(States::ON, States::OFF, Events::SWITCH);
    memAfterTransitions = getAllocatedHeapMemory();

    hsm->transition(Events::SWITCH);
    memAfterTransitionExecute = getAllocatedHeapMemory();

    memAfter = getAllocatedHeapMemory();

    //-------------------------------------------
    printf("memBefore=%li bytes\n\n", memBefore);
    printf("memAfterEmpty=%li, diff=%li bytes\n", memAfterEmpty, memAfterEmpty - memBefore);
    printf("memAfterStates=%li, diff=%li bytes\n", memAfterStates, memAfterStates - memAfterEmpty);
    printf("memAfterTransitions=%li, diff=%li bytes\n", memAfterTransitions, memAfterTransitions - memAfterStates);
    printf("memAfterTransitionExecute=%li, diff=%li bytes\n",
           memAfterTransitionExecute,
           memAfterTransitionExecute - memAfterTransitions);

    printf("\nTOTAL=%li bytes\n", memAfter - memBefore);

    return 0;
}