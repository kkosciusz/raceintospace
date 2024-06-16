#include "endianness.h"

#include <cassert>

#include <boost/endian/conversion.hpp>

#include "Buzz_inc.h"
#include "game_main.h"

static_assert(boost::endian::order::native == boost::endian::order::big
    || boost::endian::order::native == boost::endian::order::little,
    "don't know how to swap endianness on this system");

namespace {

inline bool need_swap()
{
    return boost::endian::order::native == boost::endian::order::big;
}

void SwapEquipment()
{
    for (int plr = 0; plr < 2; ++plr) {
        for (int i = 0; i < 3; i++) {
            Swap16bit(Data->P[plr].Probe[i].InitCost);
            Swap16bit(Data->P[plr].Probe[i].UnitWeight);
            Swap16bit(Data->P[plr].Probe[i].MaxPay);
            Swap16bit(Data->P[plr].Probe[i].Safety);
            Swap16bit(Data->P[plr].Probe[i].MisSaf);
            Swap16bit(Data->P[plr].Probe[i].MSF);
            Swap16bit(Data->P[plr].Probe[i].Steps);
            Swap16bit(Data->P[plr].Probe[i].Failures);
        }

        for (int i = 0; i < 5; i++) {
            Swap16bit(Data->P[plr].Rocket[i].InitCost);
            Swap16bit(Data->P[plr].Rocket[i].UnitWeight);
            Swap16bit(Data->P[plr].Rocket[i].MaxPay);
            Swap16bit(Data->P[plr].Rocket[i].Safety);
            Swap16bit(Data->P[plr].Rocket[i].MisSaf);
            Swap16bit(Data->P[plr].Rocket[i].MSF);
            Swap16bit(Data->P[plr].Rocket[i].Steps);
            Swap16bit(Data->P[plr].Rocket[i].Failures);
        }

        for (int i = 0; i < 7; i++) {
            Swap16bit(Data->P[plr].Manned[i].InitCost);
            Swap16bit(Data->P[plr].Manned[i].UnitWeight);
            Swap16bit(Data->P[plr].Manned[i].MaxPay);
            Swap16bit(Data->P[plr].Manned[i].Safety);
            Swap16bit(Data->P[plr].Manned[i].MisSaf);
            Swap16bit(Data->P[plr].Manned[i].MSF);
            Swap16bit(Data->P[plr].Manned[i].Steps);
            Swap16bit(Data->P[plr].Manned[i].Failures);
        }

        for (int i = 0; i < 7; i++) {
            Swap16bit(Data->P[plr].Misc[i].InitCost);
            Swap16bit(Data->P[plr].Misc[i].UnitWeight);
            Swap16bit(Data->P[plr].Misc[i].MaxPay);
            Swap16bit(Data->P[plr].Misc[i].Safety);
            Swap16bit(Data->P[plr].Misc[i].MisSaf);
            Swap16bit(Data->P[plr].Misc[i].MSF);
            Swap16bit(Data->P[plr].Misc[i].Steps);
            Swap16bit(Data->P[plr].Misc[i].Failures);
        }
    }
}

}

void Swap32bit(uint32_t& value)
{
    if (need_swap()) 
        boost::endian::endian_reverse_inplace(value);
}

void Swap32bit(int32_t& value)
{
    if (need_swap()) 
        boost::endian::endian_reverse_inplace(value);
}

void Swap16bit(uint16_t& value)
{
    if (need_swap())
        boost::endian::endian_reverse_inplace(value);
}

void Swap16bit(int16_t& value)
{
    if (need_swap())
        boost::endian::endian_reverse_inplace(value);
}

void SwapFloat(float& value)
{
    if (need_swap())
        boost::endian::endian_reverse_inplace(value);
}

// This will conditionally swap all the player structures
void SwapGameDat()
{
    Swap32bit(Data->Checksum);

    for (int j = 0; j < 28; j++) {
        Swap16bit(Data->Prestige[j].Points[0]);
        Swap16bit(Data->Prestige[j].Points[1]);
    }

    for (int j = 0; j < 2; j++) {
        Swap16bit(Data->P[j].Cash);
        Swap16bit(Data->P[j].Budget);
        Swap16bit(Data->P[j].Prestige);

        for (int i = 0; i < 5; i++) {
            Swap16bit(Data->P[j].PrestHist[i][0]);
            Swap16bit(Data->P[j].PrestHist[i][1]);
            Swap16bit(Data->P[j].PresRev[i]);
            Swap16bit(Data->P[j].Spend[i][0]);
            Swap16bit(Data->P[j].Spend[i][1]);
            Swap16bit(Data->P[j].Spend[i][2]);
            Swap16bit(Data->P[j].Spend[i][3]);
        }

        Swap16bit(Data->P[j].tempPrestige[0]);
        Swap16bit(Data->P[j].tempPrestige[1]);

        for (int i = 0; i < 40; i++) {
            Swap16bit(Data->P[j].BudgetHistory[i]);
            Swap16bit(Data->P[j].BudgetHistoryF[i]);
        }

        for (int i = 0; i < 65; i++) {
            Swap16bit(Data->P[j].Pool[i].Prestige);
        }

        for (int i = 0; i < 100; i++) {
            Swap16bit(Data->P[j].History[i].result);
            Swap16bit(Data->P[j].History[i].spResult);
            Swap16bit(Data->P[j].History[i].Prestige);
        }

        Swap16bit(Data->P[j].PastMissionCount);

        for (int i = 0; i < 30; i++) {
            Swap16bit(Data->P[j].PastIntel[i].num);
        }

    }

    SwapEquipment();
}