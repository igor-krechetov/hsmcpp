// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include <Arduino.h>
#include <hsmcpp.hpp>
#include <memory>

#include "BlinkHsmBase.hpp"

/*
  This example demonstrates basic usage of hsmcpp library by implementing a LED
  blinking.

  The main idea is to separate behavior of your device from code and define it
  in a form of a state machine.

  State machine is defined in blink.scxml file and looks like this:

        ---------         ---------  SWITCH  ---------
        |       |         |  LED  |--------->|  LED  |
  [*]-->| setup |-------->|  On   |          |  Off  |
        |       |         |       |<---------|       |
        ---------         ---------  SWITCH  ---------

  SWITCH transition is repeatedly triggered on 1 second timeout, which is
  started when we transition from [setup] to [LED On] state. A corresponding
  state callback will be called from BlinkHSM When transition is executed and
  HSM moved to a new state.

  ==============================
  # Building with PlatformIO
  ==============================
  blink.scxml will be automatically generated into BlinkHsmBase C++ class during
  build. Generation is controlled by "custom_hsm_files" and "custom_hsm_gendir"
  settings in platformio.ini.

  ==============================
  # Building with ArduinoIDE
  ==============================
  You will need to manually generate HSM class before compiling this sketch.
  Generator is installed as part of the hsmcpp library and is located in:
  <Arduino>/libraries/hsmcpp/tools/scxml2gen.py

  <Arduino> is path to your sketchbook location (see File > Preferences > Sketchbook location).

  To generate BlinkHsmBase class run this command from inside your sketch folder:
  python3 <Arduino>/libraries/hsmcpp/tools/scxml2gen.py -code -scxml ./blink.scxml -class_name BlinkHsm -class_suffix Base -template_hpp <Arduino>/libraries/hsmcpp/tools/template.hpp -template_cpp <Arduino>/libraries/hsmcpp/tools/template.cpp -dest_dir ./
*/

// Implementation of state machine callbacks.
class BlinkHSM : public BlinkHsmBase {
protected:
  void onOff(const hsmcpp::VariantVector_t &args) override {
    // turn the LED off by making the voltage LOW
    digitalWrite(LED_BUILTIN, LOW);
  }

  void onOn(const hsmcpp::VariantVector_t &args) override {
    // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED_BUILTIN, HIGH);
  }
};

// global instances of state machine and dispatcher
BlinkHSM *gHSM = nullptr;
std::shared_ptr<hsmcpp::HsmEventDispatcherArduino> gDispatcher;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Create instance of the state machine
  // Usually you would want the instance of your state machine to live for
  // signifficant period of time (or forever). Because of this, it should be
  // created dynamically to use heap memory instead of stack.
  gHSM = new BlinkHSM();
  // Create event dispatcher
  gDispatcher = hsmcpp::HsmEventDispatcherArduino::create();
  // initialize state machine
  gHSM->initialize(gDispatcher);

  // call HSM transition to start blinking
  gHSM->transition(BlinkHsmEvents::START);
}

void loop() {
  // tell dispatcher to process pending events and transitions
  gDispatcher->dispatchEvents();
}