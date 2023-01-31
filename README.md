[![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/igor-krechetov/hsmcpp/blob/main/LICENSE)
[![Changelog](https://img.shields.io/badge/changelog-v0.28.0-green.svg)](https://github.com/igor-krechetov/hsmcpp/blob/main/CHANGELOG.md)
[![Documentation Status](https://readthedocs.org/projects/hsmcpp/badge/?version=latest)](https://hsmcpp.readthedocs.io/en/latest/?badge=latest)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/igor-krechetov/library/hsmcpp.svg)](https://registry.platformio.org/libraries/igor-krechetov/hsmcpp)

# Quality Status

[![Build Status](https://github.com/igor-krechetov/hsmcpp/actions/workflows/build.yml/badge.svg)](https://github.com/igor-krechetov/hsmcpp/actions/workflows/build.yml)
[![SCA: MISRA](https://github.com/igor-krechetov/hsmcpp/actions/workflows/sca_misra.yml/badge.svg)](https://github.com/igor-krechetov/hsmcpp/actions/workflows/sca_misra.yml)
[![SCA: CodeQL](https://github.com/igor-krechetov/hsmcpp/actions/workflows/sca_codeql.yml/badge.svg)](https://github.com/igor-krechetov/hsmcpp/actions/workflows/sca_codeql.yml)

## Unit Tests

[![Coverage Status](https://coveralls.io/repos/github/igor-krechetov/hsmcpp/badge.svg?branch=main)](https://coveralls.io/github/igor-krechetov/hsmcpp?branch=main)
| STD | glib | glibmm | Qt |
|-----|------|--------|----|
| [![Tests: STD](https://raw.githubusercontent.com/igor-krechetov/hsmcpp/build_artifacts/tests_result_std.svg)](https://github.com/igor-krechetov/hsmcpp/blob/build_artifacts/tests_result_std.log)  | [![Tests: Glib](https://raw.githubusercontent.com/igor-krechetov/hsmcpp/build_artifacts/tests_result_glib.svg)](https://github.com/igor-krechetov/hsmcpp/blob/build_artifacts/tests_result_glib.log)   | [![Tests: GLibmm](https://raw.githubusercontent.com/igor-krechetov/hsmcpp/build_artifacts/tests_result_glibmm.svg)](https://github.com/igor-krechetov/hsmcpp/blob/build_artifacts/tests_result_glibmm.log)     | [![Tests: Qt](https://raw.githubusercontent.com/igor-krechetov/hsmcpp/build_artifacts/tests_result_qt.svg)](https://github.com/igor-krechetov/hsmcpp/blob/build_artifacts/tests_result_qt.log) |

# Overview
HSMCPP is a C++ library providing an easy way (hopefully) to add hierarchical (HSM) or finite state machine (FSM) to your project. Main motivation behind making it was lack of suitable alternatives which do not involve large frameworks (often commercial). And even they couldn't satisfy projects needs that I usually have to deal with. This is in no way a "silver bullet" library, but it might be useful for you when dealing with RTOS systems, multithreading or event driven applications.

It's also applicable for single threaded and synchronous applications, but it might not be the most efficient option.

If you are not familiar with HSM/FSM and which problems they can solve in your code, I recommend reading:
- [Introduction to Hierarchical State Machines](https://barrgroup.com/embedded-systems/how-to/introduction-hierarchical-state-machines)
- [Hierarchical Finite State Machine for AI Acting Engine](https://towardsdatascience.com/hierarchical-finite-state-machine-for-ai-acting-engine-9b24efc66f2)

# Key Features
## Generic
- visual state machine editors (through [thirdparty editors](https://hsmcpp.readthedocs.io/en/latest/code-generation/editors/editors.html))
- [code generation](https://hsmcpp.readthedocs.io/en/latest/code-generation/code-generation.html) (using state machine described in SCXML format as an input)
- PlantUML diagrams generation (from SCXML files)
- asynchronous / synchronous execution
- thread safety
- supported platforms:
  - POSIX compliant
  - Windows
  - FreeRTOS
- [configurable event dispatchers](https://hsmcpp.readthedocs.io/en/latest/platforms/platforms.html#built-in-dispatchers):
  - std::thread based
  - glib based
  - glibmm based
  - Qt based
  - FreeRTOS based
  - possibility to [implement your own dispatcher](https://hsmcpp.readthedocs.io/en/latest/platforms/platforms.html#implementing-custom-dispatchers)
- [visual debugger](https://hsmcpp.readthedocs.io/en/latest/tools/hsmdebugger/hsmdebugger.html) to help analyze state machine behavior

## State machine related
- [states](https://hsmcpp.readthedocs.io/en/latest/features/states/states.html)
- [substates](https://hsmcpp.readthedocs.io/en/latest/features/substates/substates.html) (possible to define hierarchy)
- [transitions](https://hsmcpp.readthedocs.io/en/latest/features/transitions/transitions.html)
- [history](https://hsmcpp.readthedocs.io/en/latest/features/history/history.html)
- [timers](https://hsmcpp.readthedocs.io/en/latest/features/timers/timers.html)
- state and transition [callbacks](https://hsmcpp.readthedocs.io/en/latest/code-generation/scxml/scxml.html#callbacks-definition) (enter, exit, state changed, on transition)
- passing data to state and transition callbacks
- [parallel states](https://hsmcpp.readthedocs.io/en/latest/features/parallel/parallel.html)
- final states
- [conditional transitions](https://hsmcpp.readthedocs.io/en/latest/features/transitions/transitions.html#conditional-transitions)
- [conditional entry points](https://hsmcpp.readthedocs.io/en/latest/features/substates/substates.html#conditional-entry-points)
- [state actions](https://hsmcpp.readthedocs.io/en/latest/features/states/states.html#state-actions)
- [self transitions](https://hsmcpp.readthedocs.io/en/latest/features/transitions/transitions.html#self-transitions)
- transition cancelation
- support for std::function and lambdas as callbacks

# Documentation
Documentation is available [online](https://hsmcpp.readthedocs.io).

# HSM GUI Editors
Check out [documentation](https://hsmcpp.readthedocs.io/en/latest/code-generation/editors/editors.html) to learn more about available editors.

![Editing HSM in Qt Creator](https://hsmcpp.readthedocs.io/en/latest/_images/editor_qt.png)

![Editing HSM in scxmlgui](https://hsmcpp.readthedocs.io/en/latest/_images/editor_scxmlgui.png)


# hsmdebugger
[Read documentation](https://hsmcpp.readthedocs.io/en/latest/tools/hsmdebugger/hsmdebugger.html) for details on how to use debugger.

![hsmdebugger demo](https://hsmcpp.readthedocs.io/en/latest/_images/hsmdebugger_demo.gif)


# Installation
```bash
git clone https://github.com/igor-krechetov/hsmcpp.git
cd ./hsmcpp
./build.sh
cd ./build
make install
```
By default it will build all included components, tests and examples. You can disable any of them using cmake build flags. For example you probably will not have glib or glibmm libraries available on Windows so you might want to exclude them.

See [detailed instructions in documentation](https://hsmcpp.readthedocs.io/en/latest/getting-started/getting-started.html#building-the-library).

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

![Hello Wolrd HSM](https://hsmcpp.readthedocs.io/en/latest/_static/images/00_helloworld.png)

Implementation using HsmEventDispatcherSTD:
```C++
#include <chrono>
#include <thread>
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
    std::shared_ptr<hsmcpp::HsmEventDispatcherSTD> dispatcher = std::make_shared<hsmcpp::HsmEventDispatcherSTD>();
    hsmcpp::HierarchicalStateMachine<States, Events> hsm(States::OFF);

    hsm.initialize(dispatcher);

    hsm.registerState(States::OFF, [&hsm](const VariantList_t& args)
    {
        printf("Off\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });
    hsm.registerState(States::ON, [&hsm](const VariantList_t& args)
    {
        printf("On\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });

    hsm.registerTransition(States::OFF, States::ON, Events::SWITCH);
    hsm.registerTransition(States::ON, States::OFF, Events::SWITCH);

    hsm.transition(Events::SWITCH);

    dispatcher->join();

    return 0;
}
```

See [/examples/07_build](https://github.com/igor-krechetov/hsmcpp/tree/main/examples/07_build) for CMake configuration examples.

For other examples see [Getting Started guide](https://hsmcpp.readthedocs.io/en/latest/getting-started/getting-started.html#) or [/examples](https://github.com/igor-krechetov/hsmcpp/tree/main/examples).


## Notable FSM/HSM libraries
- [Qt](https://github.com/qt/qtscxml) (using QStateMachine or QScxmlStateMachine)
- [QP/C++](https://github.com/QuantumLeaps/qpcpp)
- [TinyFSM](https://github.com/digint/tinyfsm)
- [Another Finite State Machine](https://github.com/zmij/afsm)
- [HFSM2](https://github.com/andrew-gresyk/HFSM2)
- [arduino-fsm](https://github.com/jonblack/arduino-fsm)