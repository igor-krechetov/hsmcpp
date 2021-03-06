# Changelog
All notable changes to this project will be documented in this file.

## [0.14.1] - 2021-06-05
### Added
- support for history states in scxml2gen
- example for history usage
- generateHsmDiagram() CMake function
### Fixed
- hsmdebugger: selecting jar file for plantuml wasn't working on Ubuntu

## [0.14.0] - 2021-05-31
### Added
- support for history states (not supported in generator yet)
### Fixed
- handle scenario when initial state of HSM has substates (from now on initialize() API must be called after defining HSM structure)

## [0.13.4] - 2021-05-24
### Added
- HSMBUILD_DEBUGGING build option
- diagrams for wiki

## [0.13.3] - 2021-05-17
### Fixed
- crash in hsmdebugger while quickly switching frames

## [0.13.2] - 2021-05-16
### Added
- support for Windows build
- simple build scripts
### Fixed
- scxml2gen: added support of Python 3 versions before 3.8 (due to a bug in XML parser)

## [0.13.1] - 2021-05-03
### Added
- hsmdebugger: app icon
- hsmdebugger: setting to set plantuml path
- hsmdebugger: startup scripts for Windows and Linux
### Fixed
- hsmdebugger: Windows support

## [0.13.0] - 2021-04-30
### Added
- hsmdebugger: search frames feature
- hsmdebugger: settings
- hsmdebugger: caching of generated images
### Fixed
- crash caused by mistake in enableHsmDebugging()

## [0.12.1] - 2021-04-25
### Added
- add configurable hsm dump path
- scxml2gen: add support to blocks generation
- scxml2gen: update template
- scxml2gen: add generation of getStateName(), getEventName() functions
- hsmdebugger: add recent files
- examples: simplify 03_debugging

## [0.12.0] - 2021-04-19
### Added
- initial version of hsmdebugger

## [0.11.0] - 2021-04-14
### Added
- support for parallel states and transitions
- scxml2gen: support for parallel states and transitions

## [0.10.0] - 2021-04-04
### Added
- support for conditional entrypoints in states
## [0.9.0] - 2021-03-30
### Added
- scxml2gen: support for include of external files

## [0.8.0] - 2021-03-27
### Added
- scxml2gen: PlantUML state diagrams generator (from SCXML)
- scxml2gen: support for uppercase generator variables in templates

## [0.7.0] - 2021-03-26
### Added
- SCXML based code generator
- CMake function to use code generation from CMakeLists
- Example of generator usage (02_generated)

## [0.6.0] - 2021-03-23
### Added
- Qt based event dispatcher
- Qt based example
### Updated
- improve log macroses
- rename emit() -> emitEvent() in IHsmEventDispatcher (due to name collision with Qt)

## [0.5.1] - 2021-03-23
### Added
- cmake templates showing how to include hsmcpp in a project
- new examples
- HsmEventDispatcherSTD::join()
- HsmEventDispatcherSTD::stop()
### Updated
- improved build configuration
- rename build options: HSMBUILD_VERBOSE, HSMBUILD_STRUCTURE_VALIDATION, HSMBUILD_THREAD_SAFETY, HSMBUILD_DISPATCHER_GLIB, HSMBUILD_DISPATCHER_GLIBMM, HSMBUILD_DISPATCHER_STD, HSMBUILD_TESTS, HSMBUILD_EXAMPLES
- dispatchers are now compiled into separate modules (with own pkg-config files)
- examples and tests are compiled based on enabled dispatchers
- move pkg-config templates to a subfolder (/pkgconfig)

## [0.5.0] - 2021-03-17
### Added
- glib based event dispatcher
### Updated
- reduce output from gtests
- improve gtest CMakeLists
### Fixed
- improve thread safety in all available dispatchers
- assigning same Variant object to itself
- move dispatchers creation to main thread in gtests
### Build
- new CMake build option: DISPATCHER_GLIBMM
## [0.4.2] - 2021-03-10
### Added
- add timeout to sync transitions
- add transitionSync() API for convenience
### Updated
- remove HsmHandlerClass from HierarchicalStateMachine class template parameters
- rename HsmEventDispatcherGLib -> HsmEventDispatcherGLibmm

## [0.4.1] - 2021-03-08
### Updated
- thread safety for HSM
### Build
- New CMake build options: THREAD_SAFETY

## [0.4.0] - 2021-03-05
### Added
- std::thread based event dispatcher
### Updated
- Dispatchers should now be set using initialize() methond of HSM. If dispatcher is not running yet it will be automatically started.
### Fixed
- Fix a race-condition bug in AsyncHsm (unittests).
### Build
- New CMake build options: DISPATCHER_STD, BUILD_TESTS, BUILD_EXAMPLES
- Unittests are now compiled into two separate binaries: GLib and STD based.

## [0.3.0] - 2021-03-04
### Added
- Support for custom event dispatchers. Allows integration of HSM with any event dispatching mechanism used on the project.
- GLibmm event dispatcher
### Fixed
- Remov unnecessary event emit during transition
### Build
- CMake build options: VERBOSE, STRUCTURE_VALIDATION, DISPATCHER_GLIB

## [0.2.0] - 2021-03-02
### Added
- Support for substate entry points
- Conditional transitions
- Lambda functors can now be used as callbacks

## [0.1.0] - 2021-02-15
### Added

- Initial commit of HSMCPP. This version is usable, but requires some code cleanup.