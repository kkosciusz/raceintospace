#ifndef HARDWARE_BUTTONS_H
#define HARDWARE_BUTTONS_H

#include <memory>

#include "display/surface.h"

class HardwareButtons
{
public:
    HardwareButtons(int y, int player);
    ~HardwareButtons();

    void drawButtons(int highlighted_button = -1) const;

protected:
    void drawButton(int group, bool highlighted, display::Surface &target, int x, int y) const;

    std::shared_ptr<display::Surface> buttons;
    int _y;
};

#endif // HARDWARE_BUTTONS_H
