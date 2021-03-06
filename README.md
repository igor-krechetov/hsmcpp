[![Generic badge](https://img.shields.io/badge/changelog-v0.14.1-green.svg)](https://github.com/igor-krechetov/hsmcpp/blob/main/CHANGELOG.md)
[![Generic badge](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/igor-krechetov/hsmcpp/blob/main/LICENSE)
[![Generic badge](https://img.shields.io/badge/documentation-green.svg)](https://github.com/igor-krechetov/hsmcpp/wiki)

# Overview
HSMCPP is a C++ library providing an easy way (hopefully) to add hierarchical or finite state machine to your code. Main motivation behind making it was lack of suitable alternatives which do not involve large frameworks (usually commercial) and could satisfy needs of the projects I usually have to deal with. This is in no way a "silver bullet" library, but it might be useful for you when dealing with RTOS systems, multithreading or event driven applications.

It's also applicable for single threaded and synchronous applications, but it might not be the most efficient option.

If you are not familiar with HSM/FSM and which problems they can solve in your code, I recommend reading:
- [Introduction to Hierarchical State Machines](https://barrgroup.com/embedded-systems/how-to/introduction-hierarchical-state-machines)
- [Hierarchical Finite State Machine for AI Acting Engine](https://towardsdatascience.com/hierarchical-finite-state-machine-for-ai-acting-engine-9b24efc66f2)

# Key Features
## Generic
- visual state machine editors (through [thirdparty editors](https://github.com/igor-krechetov/hsmcpp/wiki/Code-Generation#scxml-editors))
- code generation (using state machine described in SCXML format as an input)
- PlantUML diagrams generation (from SCXML files)
- asynchronous / synchronous execution
- thread safety
- configurable event dispatchers:
  - std::thread based
  - glib based
  - glibmm based
  - Qt based
  - possibility to implement your own dispatcher
- visual debugger to help analyze state machine behavior

## State machine related
- states
- substates (possible to define hierarchy)
- transitions
- history
- state and transition callbacks (enter, exit, state changed, on transition)
- passing data to state and transition callbacks
- parallel states
- conditional transitions
- conditional entry points
- self transitions
- transition cancelation
- support for std::function and lambdas as callbacks

# Documentation
Documentation is available in [Wiki](https://github.com/igor-krechetov/hsmcpp/wiki).


# HSM GUI Editors
Check out [documentation](https://github.com/igor-krechetov/hsmcpp/wiki/Code-Generation#scxml-editors) to learn more about available editors.

![Editing HSM in Qt Creator](https://github.com/igor-krechetov/hsmcpp/blob/main/doc/wiki/editors/editor_qt.png)

![Editing HSM in scxmlgui](https://github.com/igor-krechetov/hsmcpp/blob/main/doc/wiki/editors/editor_scxmlgui.png)


# hsmdebugger
Read [documentation](https://github.com/igor-krechetov/hsmcpp/wiki/hsmdebugger) for details on how to use debugger.

![hsmdebugger demo](https://github.com/igor-krechetov/hsmcpp/blob/main/doc/readme/hsmdebugger_demo.gif)


# Installation
```bash
git clone https://github.com/igor-krechetov/hsmcpp.git
cd ./hsmcpp
./build.sh
cd ./build
make install
```
By default it will build all included components, tests and examples. You can disable any of them using cmake build flags. For example you probably will not have glib or glibmm libraries available on Windows so you might want to exclude them.

See detailed instructions in [documentation](https://github.com/igor-krechetov/hsmcpp/wiki/Getting-Started#building).

# Dependencies
- For library:
  - C++11 or newer
  - glib (optional, for dispatcher)
  - glibmm (optional, for dispatcher)
  - Qt (optional, for dispatcher)
- For build:
  - cmake 3.14+
  - Visual Studio 2015+ (for Windows build)
- For code generator:
  - Python 3
- For hsmdebugger:
  - Python 3
  - PyYaml (pip3 install PyYaml)
  - PySide6 (pip3 install PySide6)
  - plantuml (minimal version: V1.2020.11)

# Creating a simple State Machine
HSM structure:

![Hello Wolrd HSM](https://github.com/igor-krechetov/hsmcpp/wiki/doc/wiki/00_helloworld.png)

Implementation using HsmEventDispatcherSTD:
```C++
#include <unistd.h>
#include <hsmcpp/hsm.hpp>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>

enum class States
{
    OFF,
    ON
};

enum class Events
{
    SWITCH
};

int main(const int argc, const char**argv)
{
    std::shared_ptr<HsmEventDispatcherSTD> dispatcher = std::make_shared<HsmEventDispatcherSTD>();
    HierarchicalStateMachine<States, Events> hsm(States::OFF);

    hsm.initialize(dispatcher);

    hsm.registerState(States::OFF, [&hsm](const VariantList_t& args)
    {
        printf("Off\n");
        usleep(1000000);
        hsm.transition(Events::SWITCH);
    });
    hsm.registerState(States::ON, [&hsm](const VariantList_t& args)
    {
        printf("On\n");
        usleep(1000000);
        hsm.transition(Events::SWITCH);
    });

    hsm.registerTransition(States::OFF, States::ON, Events::SWITCH);
    hsm.registerTransition(States::ON, States::OFF, Events::SWITCH);

    hsm.transition(Events::SWITCH);

    dispatcher->join();

    return 0;
}
```

See [/examples/cmake_templates](https://github.com/igor-krechetov/hsmcpp/tree/main/examples/cmake_templates) for CMake configuration examples.

For other examples see [Wiki](https://github.com/igor-krechetov/hsmcpp/wiki/Getting-Started) or [/examples](https://github.com/igor-krechetov/hsmcpp/tree/main/examples).


## Notable FSM/HSM libraries
- [Qt](https://github.com/qt/qtscxml) (using QStateMachine or QScxmlStateMachine)
- [QP/C++](https://github.com/QuantumLeaps/qpcpp)
- [TinyFSM](https://github.com/digint/tinyfsm)
- [Another Finite State Machine](https://github.com/zmij/afsm)
- [HFSM2](https://github.com/andrew-gresyk/HFSM2)
- [arduino-fsm](https://github.com/jonblack/arduino-fsm)