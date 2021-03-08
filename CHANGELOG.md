# Changelog
All notable changes to this project will be documented in this file.

## [0.4.1] - 2021-03-08
### Updated
- thread safety for HSM
### Build
- New CMake build options: THREAD_SAFETY

## [0.4.0] - 2021-03-05
### Added
- sts::thread based event dispatcher
### Updated
- Dispatchers should now be set using initialize() methond of HSM. If dispatcher is not running yet it will be automatically started.
### Fixed
- Fixed a race-condition bug in AsyncHsm (unittests).
### Build
- New CMake build options: DISPATCHER_STD, BUILD_TESTS, BUILD_EXAMPLES
- Unittests are now compiled into two separate binaries: GLib and STD based.

## [0.3.0] - 2021-03-04
### Added
- Support for custom event dispatchers. Allows integration of HSM with any event dispatching mechanism used on the project.
- GLib event dispatcher
### Fixed
- Removed unnecessary event emit during transition
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