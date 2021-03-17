# Changelog
All notable changes to this project will be documented in this file.

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