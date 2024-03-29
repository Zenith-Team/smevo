#pragma once

#include "tsuru/utils.h"
#include "game/wrappedcontroller.h"

class InputControllers {
public:
    ENUM_CLASS(ControllerID,
        Wiimote1,
        Wiimote2,
        Wiimote3,
        Wiimote4,
        Gamepad
    );

public:
    InputControllers()
        : controllers()
    { }

    // Initializes each controller with the appropriate ID
    void init() {
        for (u32 i = 0; i < 5; i++) {
            controllers[i].init((ControllerID::__type__) i);
        }
    }

    // Calls calc() on each controller
    //! Do not call this, it is not required, but is just here for it may be needed in the future
    void calc(u32 a, bool b) {
        for (u32 i = 0; i < 5; i++) {
            controllers[i].calc(a, b);
        }
    }

    // TODO: Use sead::BitFlag functions instead of our own

    // D-Pad
    inline bool buttonUp(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x5); }
    inline bool buttonDown(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x6); }
    inline bool buttonLeft(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x7); }
    inline bool buttonRight(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x8); }

    // Buttons
    inline bool buttonA(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x1) && nthBit32Right(this->controllers[controller].padHold.bits, 0xE); }
    inline bool buttonB(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x1) && nthBit32Right(this->controllers[controller].padHold.bits, 0xF); }
    inline bool buttonXY(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x3) && nthBit32Right(this->controllers[controller].padHold.bits, 0x2); }
    inline bool buttonPlus(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0xC); }
    inline bool buttonMinus(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0xB); }

    // Triggers
    inline bool triggerL(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x4) && nthBit32Right(this->controllers[controller].padHold.bits, 0x9) && nthBit32Right(this->controllers[controller].padHold.bits, 0x11); }
    inline bool triggerR(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x4) && nthBit32Right(this->controllers[controller].padHold.bits, 0xA) && nthBit32Right(this->controllers[controller].padHold.bits, 0x11); }
    inline bool triggerZL(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x4) && nthBit32Right(this->controllers[controller].padHold.bits, 0x11) && nthBit32Right(this->controllers[controller].padHold.bits, 0x12); }
    inline bool triggerZR(ControllerID::__type__ controller) const { return nthBit32Right(this->controllers[controller].padHold.bits, 0x4) && nthBit32Right(this->controllers[controller].padHold.bits, 0x11) && !nthBit32Right(this->controllers[controller].padHold.bits, 0x9) && !nthBit32Right(this->controllers[controller].padHold.bits, 0x12) && !nthBit32Right(this->controllers[controller].padHold.bits, 0xA); }

    // Gamepad specific
    inline bool tap() const { return nthBit32Right(this->controllers[ControllerID::Gamepad].padHold.bits, 0xD); }

    // Checks if a direction on the D-Pad is pressed
    // @param index The index of the direction to check, corresponds to Direction::DirectionType
    // @param controller The controller to check
    // @return True if the direction is pressed, false otherwise
    bool DPadDirection(u32 direction, InputControllers::ControllerID::__type__ controller) const {
        return ((this)->*(DPadFuncTable[direction]))(controller);
    }

    static bool (InputControllers::*DPadFuncTable[4])(ControllerID::__type__) const;

    WrappedController controllers[5];
};
