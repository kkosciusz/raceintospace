/*
    Copyright (C) 2005 Michael K. McCarty & Fritz Bronner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
// Interplay's BUZZ ALDRIN's RACE into SPACE
//
// Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA
// Copyright 1991 by Strategic Visions, Inc.
// Designed by Fritz Bronner
// Programmed by Michael K McCarty
//
// Support files for news.cpp

// This file seems to perform support tasks for news.cpp:
// wrapup from last turn's missions, etc.

#include "news_sup.h"

#include "Buzz_inc.h"
#include "game_main.h"
#include "hardware.h"
#include "options.h"
#include "pace.h"

/**
 *
 * \param type 1:postive -1:negative search
 */
int Steal(int plr, int prog, int type)
{
    int i = 0, j = 0, k = 0, save[28], lo = 0, hi = 28;

    // Iterate among enum without requiring elements to have specific
    // values.
    int hwMap[4] = { PROBE_HARDWARE, ROCKET_HARDWARE, MANNED_HARDWARE,
                     MISC_HARDWARE
                   };

    memset(save, 0x00, sizeof save);

    for (int hwType = 0; hwType < 4; hwType++) {
        for (int i = 0; i < 7; i++) {
            int eqIndex = i + 7 * hwType;
            Equipment &equip = HardwareProgram(plr, hwMap[hwType], i);
            Equipment &rival =
                HardwareProgram(other(plr), hwMap[hwType], i);

            if (equip.Num >= 0 && rival.Num >= 0) {
                if (type == 1) {
                    save[eqIndex] = rival.Safety - equip.Safety;
                } else {
                    save[eqIndex] = equip.Safety - rival.Safety;
                }

                if (type == -1 && save[eqIndex] < 0 &&
                    (equip.Safety + save[eqIndex]) < equip.Base) {
                    save[eqIndex] = 0;
                }
            }
        }
    }

    save[25] = save[26] = save[27] = 0;
    save[12] = save[13] = 0;
    save[3] = save[4] = save[5] = save[6] = 0;

    if (type == 1) {
        for (i = lo; i < hi; i++) {
            if (save[i] <= 0) {
                save[i] = 0;    // Positive Only
            }
        }
    } else {
        for (i = lo; i < hi; i++) {
            if (save[i] >= 0) {
                save[i] = 0;    // Negatives Only
            } else {
                save[i] *= -1;
            }
        }
    }

    for (i = lo; i < hi; i++) {
        if (save[i] > 0) {
            j++;    // Check if event is good.
        }
    }

    if (j == 0) {
        return 0;
    }

    j = brandom(hi - lo) + lo;

    while ((k < 100) && (save[j] <= 0)) { // finds candidate
        j = brandom(hi - lo) + lo;
        k++;
    }

    if (k == 100) {
        return 0;
    }

    Equipment &chosen = HardwareProgram(plr, hwMap[j / 7], j % 7);
    chosen.Safety += (save[j] * type);
    strcpy(&Name[0], &chosen.Name[0]);

    return save[j];
}

/** ???
 *
 * \param type 1:postive -1:negative search
 * \param per Amount of modification in percent
 */
int NMod(int plr, int prog, int type, int per)
{
    int i = 0, j = 0, save[28], lo = 0, hi = 28;
    Equipment *Eptr[28];

    lo = (prog > 0) ? (prog - 1) * 7 : 0;
    hi = (prog > 0) ? lo + 7 : 28;

    if (prog == 1) {
        hi = lo + 3;
    }

    /* drvee: this loop was going to 25, not 28 */
    for (i = 0; i < (int)ARRAY_LENGTH(Eptr); i++) {
        /** \bug Mismatch between data.h(250) and this code here */
        Eptr[i] = &Data->P[plr].Probe[i];
        save[i] = ((Eptr[i]->Safety + per * type) <= (Eptr[i]->MaxSafety) && Eptr[i]->Num >= 0) ? Eptr[i]->Safety + per * type : 0;

        if (Eptr[i]->Num < 0) {
            save[i] = 0;
        }
    }

    for (i = 0; i < (int)ARRAY_LENGTH(save); i++) {
        if (save[i] < 0) {
            save[i] = 0;
        }
    }

    save[11] = save[25] = save[26] = save[27] = save[12] = save[13] = save[3] = save[4] = save[5] = save[6] = 0;

    for (i = lo; i < hi; i++) {
        if (save[i] > 0) {
            j++;    // Check if event is good.
        }
    }

    if (j == 0) {
        return 0;
    }

    j = hi - 1;

    while (save[j] == 0) {
        j--;
    }

    // Increment value and return program name
    Eptr[j]->Safety = save[j];
    strcpy(&Name[0], &Eptr[j]->Name[0]);

    if (Eptr[j]->Safety > Eptr[j]->MaxSafety) {
        Eptr[j]->Safety = Eptr[j]->MaxSafety;
    }

    return save[j];
}


int DamMod(int plr, int prog, int dam, int cost)
{
    int i = 0, j = 0, lo = 0, hi = 28;
    int save[28];
    Equipment *Eptr[28];

    memset(save, 0x00, sizeof save);
    lo = (prog > 0) ? (prog - 1) * 7 : 0;
    hi = (prog > 0) ? lo + 7 : 28;

    for (i = 0; i < 25; i++) {
        Eptr[i] = &Data->P[plr].Probe[i];
        save[i] = ((Eptr[i]->Safety > Eptr[i]->Base) && Eptr[i]->Num >= 0) ? Eptr[i]->Safety : 0;
    }

    for (i = 0; i < 25; i++) {
        if (save[i] < dam) {
            save[i] = 0;
        }
    }

    save[25] = save[26] = save[27] = save[12] = save[13] = save[3] = save[4] = save[5] = save[6] = save[11] = 0;
    save[19] = save[20] = 0; //remove LM's

    for (i = lo; i < hi; i++) {
        if (save[i] > 0) {
            j++;    // Check if event is good.
        }
    }

    if (j == 0) {
        return 0;
    }

    j = hi - 1;

    while (save[j] == 0) {
        j--;
    }

    strcpy(&Name[0], &Eptr[j]->Name[0]);

    if (options.cheat_no_damage == 0) {
        Eptr[j]->Damage += dam;    //Damaged Equipment Cheat
    }

    if (options.cheat_no_damage == 0) {
        Eptr[j]->DCost += cost;
    }

    return save[j];
}


int RDMods(int plr, int prog, int type, int val)
{
    int i = 0, j = 0, save[28], lo = 0, hi = 28;
    Equipment *Eptr[28];

    memset(save, 0x00, sizeof save);
    lo = (prog > 0) ? (prog - 1) * 7 : 0;
    hi = (prog > 0) ? lo + 7 : 28;

    for (i = 0; i < 25; i++) {
        Eptr[i] = &Data->P[plr].Probe[i];
        save[i] = ((Eptr[i]->Safety > Eptr[i]->Base) && Eptr[i]->Num >= 0) ? Eptr[i]->Safety : 0;
    }

    save[11] = save[25] = save[26] = save[27] = save[12] = save[13] = save[3] = save[4] = save[5] = save[6] = 0;

    for (i = 0; i < 28; i++) {
        if (save[i] < 0) {
            save[i] = 0;
        }
    }

    for (i = lo; i < hi; i++) {
        if (save[i] > 0) {
            j++;    // Check if event is good.
        }
    }

    if (j == 0) {
        return 0;
    }

    j = hi - 1;

    while (save[j] == 0) {
        j--;
    }

    strcpy(&Name[0], &Eptr[j]->Name[0]);
    Eptr[j]->RDCost += type * val;
    return save[j];
}


int SaveMods(char plr, char prog)
{
    int i = 0, j = 0, save[28], lo = 0, hi = 28;

    for (i = 0; i < 28; i++) {
        save[i] = 0;
    }

    switch (prog) {
    case 0:
        lo = 0;
        hi = 21;
        break;   // check ALL    programs

    case 1:
        lo = 0;
        hi = 7;
        break;    // check PROBE  programs

    case 2:
        lo = 7;
        hi = 14;
        break;   // check ROCKET programs

    case 3:
        lo = 14;
        hi = 18;
        break;  // check MANNED programs

    case 4:
        lo = 18;
        hi = 21;
        break;  // check LEM   programs
    }

    for (i = 0; i < 7; i++) {
        if (Data->P[plr].Probe[i].Num >= 0) {
            save[i] = 1;
        }

        if (Data->P[plr].Rocket[i].Num >= 0) {
            save[i + 7] = 1;
        }

        if (Data->P[plr].Manned[i].Num >= 0) {
            save[i + 14] = 1;
        }

        if (Data->P[plr].Misc[i].Num >= 0) {
            save[i + 21] = 1;
        }
    }

    save[11] = save[25] = save[26] = save[27] = save[12] = save[13] = save[3] = save[4] = save[5] = save[6] = 0;

    for (i = lo; i < hi; i++) {
        if (save[i] > 0) {
            j++;    // Check if event is good.
        }
    }

    if (j == 0) {
        return 0;
    }

    j = hi - 1;

    while (save[j] == 0) {
        j--;
    }

    // Increment value and return program name
    if (j >= 0 && j < 7) {
        Data->P[plr].Probe[j].SaveCard = 1;
        strcpy(&Name[0], &Data->P[plr].Probe[j].Name[0]);
    }

    if (j >= 7 && j < 14) {
        Data->P[plr].Rocket[j - 7].SaveCard = 1;
        strcpy(&Name[0], &Data->P[plr].Rocket[j - 7].Name[0]);
    }

    if (j >= 14 && j < 21) {
        Data->P[plr].Manned[j - 14].SaveCard = 1;
        strcpy(&Name[0], &Data->P[plr].Manned[j - 14].Name[0]);
    }

    return save[j];
}


/** Transfer new nauts
 */
void NewNauts(int plr) {
    
    /* The original bonus astronauts & cosmonauts were:
    REEVES, CHAMBERLAIN, YEAGER and STIPPOV, SCHLICKBERND, FARGOV -Leon */
    
    // US Astronauts
    char UsName[3][14] = { "MANKE", "POWELL", "YEAGER"};
    int UsStatsCap[3] = {1, 2, 3};
    int UsStatsLM[3] = {2, 0, 0};
    int UsStatsEVA[3] = {2, 0 ,1};
    int UsStatsDock[3] = {1, 1, 1};
    int UsStatsEndur[3] = {3, 0, 2};
	
    // USSR Cosmonauts
    char SovName[3][14] = { "ILYUSHIN", "KRAMARENKO", "DOLGOV"};
    int SovStatsCap[3] = {3, 2, 2};
    int SovStatsLM[3] = {0, 1, 3};
    int SovStatsEVA[3] = {0, 2 ,0};
    int SovStatsDock[3] = {3, 0, 0};
    int SovStatsEndur[3] = {3, 3, 1};	
	
    for (int i = 0; i < 3; i++) {
        strcpy(&Data->P[plr].Pool[Data->P[plr].AstroCount].Name[0], plr ? SovName[i]: UsName[i]);
        Data->P[plr].Pool[Data->P[plr].AstroCount].Cap = plr ? SovStatsCap[i] : UsStatsCap[i];
        Data->P[plr].Pool[Data->P[plr].AstroCount].LM = plr ? SovStatsLM[i] : UsStatsLM[i];
        Data->P[plr].Pool[Data->P[plr].AstroCount].EVA = plr ? SovStatsEVA[i] : UsStatsEVA[i];
        Data->P[plr].Pool[Data->P[plr].AstroCount].Docking = plr ? SovStatsDock[i] : UsStatsDock[i];
        Data->P[plr].Pool[Data->P[plr].AstroCount].Endurance = plr ? SovStatsEndur[i] : UsStatsEndur[i];
        Data->P[plr].Pool[Data->P[plr].AstroCount].Status = AST_ST_TRAIN_BASIC_1;
        Data->P[plr].Pool[Data->P[plr].AstroCount].Face = brandom(10) + 1;
        Data->P[plr].Pool[Data->P[plr].AstroCount].Service = 1;
        Data->P[plr].Pool[Data->P[plr].AstroCount].Compat = brandom(10) + 1;
        Data->P[plr].Pool[Data->P[plr].AstroCount].CR = brandom(2) + 1;
        Data->P[plr].Pool[Data->P[plr].AstroCount].CL = brandom(2) + 1;
        Data->P[plr].Pool[Data->P[plr].AstroCount].Group = 9;
        Data->P[plr].Pool[Data->P[plr].AstroCount].Mood = 85 + 5 * brandom(4);
        Data->P[plr].AstroCount++;
    }
}

// EOF
