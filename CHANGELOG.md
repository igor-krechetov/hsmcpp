# Changelog
All notable changes to this project will be documented in this file.

## [0.3.0] - 2021-03-04
### Added
- Support for custom event dispatchers. Allows integration of HSM with any event dispatching mechanism used on the project.
- GLib event dispatcher
- CMake build options: VERBOSE, STRUCTURE_VALIDATION, DISPATCHER_GLIB
### Fixed
- Removed unnecessary event emit during transition

## [0.2.0] - 2021-03-02
### Added
- Support for substate entry points
- Conditional transitions
- Lambda functors can now be used as callbacks

## [0.1.0] - 2021-02-15
### Added

- Initial commit of HSMCPP. This version is usable, but requires some code cleanup.