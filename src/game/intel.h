#ifndef INTEL_H
#define INTEL_H

#include <memory>

namespace display
{
class LegacySurface;
}

void Intel(char plr);
void IntelPhase(char plr, char pt);
std::shared_ptr<display::LegacySurface> LoadCIASprite();

#endif // INTEL_H
