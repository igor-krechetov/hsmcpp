#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include <iostream>

#include "gen/TimerIssueBase.hpp"

class TimerIssue : public TimerIssueBase
{
    bool timer_condition(const hsmcpp::VariantVector_t& args) override
    {
        std::cout << "Timer condition" << std::endl;
        return true;
    }
};

int main(const int argc, const char** argv)
{
    std::shared_ptr<hsmcpp::HsmEventDispatcherSTD> dispatcher = hsmcpp::HsmEventDispatcherSTD::create();
    TimerIssue hsm;

    if(hsm.initialize(dispatcher))
    {
        std::cout << "Initialized the state machine" << std::endl;
    }
    dispatcher->join();

    return 0;
}