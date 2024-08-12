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

// This file handles the main screen, and shows the Mission Review (the summary you see at the end of a mission).

#include "place.h"

#include <cassert>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>

#include <boost/format.hpp>

#include "display/image.h"
#include "display/graphics.h"
#include "display/surface.h"
#include "display/palettized_surface.h"

#include "gamedata.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "utils.h"
#include "game_main.h"
#include "mission_util.h"
#include "museum.h"
#include "port.h"
#include "replay.h"
#include "mc.h"
#include "endgame.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"
#include "filesystem.h"
#include "ioexception.h"
#include "logging.h"
#include "pbm.h"

void BCDraw(int y);
void DispHelp(char top, char bot, const char *txt);
void writePrestigeFirst(char index);


void BCDraw(int y)
{
    ShBox(23, y, 54, 20 + y); //ShBox(56,y,296,20+y);
    return;
}

int MainMenuChoice()
{
    struct {
        const char *label;
        int y;
        const char *hotkeys;
    } const menu_options[] = {
        { "NEW GAME", 9 + 27 * MAIN_NEW_GAME, "N" },
        { "OLD GAME", 9 + 27 * MAIN_OLD_GAME, "O" },
#ifdef ALLOW_PBEM
        { "PLAY BY MAIL", 9 + 27 * MAIN_PBEM_GAME, "P"},
#endif
        { "CREDITS", 9 + 27 * MAIN_CREDITS, "C" },
        { "EXIT", 9 + 27 * MAIN_EXIT, "EXQ" }
    };
    const int menu_option_count = sizeof(menu_options) / sizeof(menu_options[0]);

    int selected_option = -1;

    {
#ifdef ALLOW_PBEM
        std::shared_ptr<display::PalettizedSurface> image(Filesystem::readImage("images/main_menu.png"));
#else
        std::shared_ptr<display::PalettizedSurface> image(Filesystem::readImage("images/main_menu_4slot.png"));
#endif

        image->exportPalette();
        display::graphics.screen()->draw(image, 0, 0);
    }

    for (int i = 0; i < menu_option_count; i++) {
        ShBox(21, menu_options[i].y, 180, menu_options[i].y + 25);
        draw_heading(34, menu_options[i].y + 6, menu_options[i].label, 1, 0);
    }

    FadeIn(2, 30, 0, 0);
    WaitForMouseUp();

    while (selected_option == -1) {
        GetMouse();

        if (key) {
            // Check for hotkeys
            for (int i = 0; i < menu_option_count; i++) {
                // Iterate over each hotkey
                for (const char *hotkey = menu_options[i].hotkeys; *hotkey; hotkey++) {
                    if (*hotkey == key) {
                        // Match!
                        selected_option = i;
                        break;
                    }
                }
            }
        }

        if (mousebuttons) {
            for (int i = 0; i < menu_option_count; i++) {
                if (x >= 21 && x <= 180 && y >= menu_options[i].y && y <= menu_options[i].y + 27) {
                    selected_option = i;
                    break;
                }
            }
        }
    }

    // Draw a highlighted box
    InBox(21, menu_options[selected_option].y, 180, menu_options[selected_option].y + 25);

    // Let it sink in
    delay(50);

    // Pass back the selected option
    key = 0;
    return selected_option;
}

/**
 * Creates a menu screen with a selection of options.
 *
 * The portbut.but.%d.png images use the Port palette, specifically
 * the USA palette. It's unlikely the icons use any of the differing
 * colors, and so the palette is not exported.
 *
 * \param plr   the player index.
 * \param qty   how many menu options are available.
 * \param Name  an array of (qty) 22-character menu option labels
 *              (Name[qty][22]).
 * \param Imx   an array of (qty) numeric indices specifying menu icons.
 * \param mayEscape  true if the player may press 'ESC' to abort.
 * \return  the index of the selected menu option, [0, qty),
 *          or -1 to abort.
 * \throws runtime_error  if Filesystem cannot read the icon images.
 */
int BChoice(int plr, int qty, char *Name, char *Imx, bool mayEscape)
{
    int j;
    int starty = 100;
    char filename[128];
    //FadeOut(2,pal,10,0,0);
    display::LegacySurface local(30, 19);

    starty -= (qty * 23 / 2);

    /* hard-coded magic numbers, yuck */
    for (int i = 0; i < qty; i++) {
        BCDraw(starty + 23 * i);
        draw_heading(60, starty + 4 + 23 * i, &Name[i * 22], 1, 0);

        snprintf(filename, sizeof(filename), "images/portbut.but.%d.png",
                 (int) Imx[i]);
        std::shared_ptr<display::PalettizedSurface> icon(
            Filesystem::readImage(filename));
        display::graphics.screen()->draw(icon, 24, starty + 1 + 23 * i);
    }

    // FadeIn(2,pal,10,0,0);
    key = 0;
    WaitForMouseUp();
    j = -1;

    while (j == -1) {
        av_block();
        GetMse(plr, 0);

        if (mayEscape && (key == K_ESCAPE || key == K_ENTER)) {
            break;
        }

        for (int i = 0; i < qty; i++)  { // keyboard stuff
            if ((char)key == Name[i * 22]) {

                InBox(23, starty + 23 * i, 54, starty + 20 + 23 * i);

                delay(50);
                j = i + 1;
                key = 0;
            }
        }

        if (mousebuttons != 0) {
            for (int i = 0; i < qty; i++) {
                if ((x >= 23 && x <= 54 && y >= starty + 23 * i && y <= starty + 20 + 23 * i) ||
                    (x > 56 && y > starty + 23 * i && x < 296 && y < starty + 20 + 23 * i)) {

                    InBox(23, starty + 23 * i, 54, starty + 20 + 23 * i);

                    delay(50);
                    j = i + 1;
                }
            }
        }

    }

    return j;
}

/**
 * Draw a mission patch to the screen.
 *
 * There are only ten patches per side for each hardware program.
 * Going over 10 will overflow into the opposing side's patches.
 * There are 110 patches, indexed from 0, with 100-109 apparently
 * representing the Zond capsule.
 *
 * Reserves the [32, 63] screen palette space for the patch colors.
 * Patches are 32-color palettes with one transparent.
 *
 * \param plr  different patches for each program (0 for USA, 1 for USSR).
 * \param x    the upper-left x-coordinate of the image destination.
 * \param y    the upper-left y-coordinate of the image destination.
 * \param prog  The hardware program per EquipMannedIndex.
 * \param poff  The poff-th patch associated with the program.
 * \throw runtime_error  if Filesystem cannot load the image.
 */
void PatchMe(char plr, int x, int y, char prog, char poff)
{
    int patchNum = (50 * plr) + 10 * prog + poff;

    assert(patchNum >= 0 && patchNum < 110);

    char filename[128];
    snprintf(filename, sizeof(filename), "images/patches.but.%d.png",
             (int) patchNum);

    std::shared_ptr<display::PalettizedSurface> patch(
        Filesystem::readImage(filename));
    patch->exportPalette(32, 32 + (32 - 1));  // [32, 64) color palette
    display::graphics.screen()->draw(patch, x, y);
}

/**
 * Draws an image of the astronaut/cosmonaut face in the appropriate
 * helmet at the given screen coordinates.
 *
 * Exports the face/helmet palette to the screen palette space [64, 96).
 *
 * Astronaut/Cosmonaut portraits are 18x15 pixels.
 * The helmet is 80x50 pixels.
 *
 * \param plr   0 for the USA suit, 1 for the USSR suit.
 * \param x     the upper-left x-coordinate of the image destination.
 * \param y     the upper-left y-coordinate of the image destination.
 * \param face  the face image, 0-84.
 * \throws runtime_error  if Filesystem is unable to read face image.
 */
void AstFaces(char plr, int x, int y, char face)
{
    assert(face >= 0 && face <= 96);
    char filename[128];
    snprintf(filename, sizeof(filename), "images/faces.but.%d.png",
             (int)face);

    std::shared_ptr<display::PalettizedSurface> icon(
        Filesystem::readImage(filename));
    icon->exportPalette(64, 64 + 31);  // Palette space [64, 96)

    snprintf(filename, sizeof(filename), "images/faces.but.%d.png",
             (int) plr + 85);
    std::shared_ptr<display::PalettizedSurface> helmet(
        Filesystem::readImage(filename));

    int fx, fy;  // Position of face within the helmet

    // The USA & USSR helmet viewports are at different spots.
    if (plr == 0) {
        fx = 32;
        fy = 17;
    } else {
        fx = 33;
        fy = 21;
    }

    display::graphics.screen()->draw(icon, x + fx, y + fy);
    display::graphics.screen()->draw(helmet, x, y);
}


/**
 * Display small icon of a planet and/or a capsule/probe.
 *
 * TODO: Change this function's name.
 *
 * An icon of a planet/moon is displayed if the planet argument is
 * set. Planets are:
 *   1: Mercury     2: Venus       3: Mars
 *   4: Jupiter     5: Saturn      6: The Moon
 *   7: ? Earth
 *
 * An icon of a hardware program may be displayed. For a non-Earth
 * planet, an interplanetary probe will display if specified. Otherwise,
 * the specified program is always displayed.
 *   1: Mercury/Vostok   2: Gemini/Voskhod   3: Apollo/Soyuz
 *   4: XMS-2/Lapot      5: Jupiter/LK-700   6: Explorer/Sputnik
 *   7: Ranger/Cosmos    8: Surveyor/Luna
 *
 * Loads a 64-color MHIST palette into the screen's color space at
 * [64, 128). This used to be configurable via the argument coff,
 * but not at the moment.
 *
 * \param plr   0 for USA, 1 for USSR.
 * \param x     the x-coordinate of the destination.
 * \param y     the y-coordinate of the destination.
 * \param prog  0-4 correspond to the capsules, 5-7 are the probes.
 * \param planet  0 to show hardware, 1-7 for a planet.
 * \param coff  [DEPRECATED] Where to place the color palette.
 * \throws runtime_error  if Filesystem is unable to load the image.
 */
void SmHardMe(char plr, int x, int y, char prog, char planet,
              unsigned char coff)
{
    coff = 64; //
    int patch = (planet > 0) ? planet - 1 : (7 + plr * 8 + prog);

    assert(patch >= 0 && patch <= 22);

    char filename[128];
    snprintf(filename, sizeof(filename), "images/mhist.but.%d.png", patch);
    std::shared_ptr<display::PalettizedSurface> image(
        Filesystem::readImage(filename));

    image->exportPalette(coff, coff + 63);
    display::graphics.screen()->draw(image, x, y);

    // Planets may show an interplanetary probe as well.
    if (planet > 0 && prog == 6) {
        SmHardMe(plr, x + planet * 2, y + 5, prog, 0, coff);
    }

    // Earth & the Moon can have a wider variety of spacecraft.
    if (planet == 7 || planet == 6) {
        SmHardMe(plr, x + planet * 2, y + 5, prog, 0, coff);
    }

    return;
}

void BigHardMe(char plr, int x, int y, char hw, char unit, char sh)
{
    if (sh == 0) {
        int index = (plr * 32) + (hw * 8) + unit;

        std::string filename((boost::format("images/rdfull.but.%1%.png") % index).str());
        std::shared_ptr<display::PalettizedSurface> image(Filesystem::readImage(filename));

        image->exportPalette(32, 2 + 33);
        display::graphics.screen()->draw(image, x, y);
    } else {
        char name[5];

        if (plr == 0) {
            name[0] = 'U';
            name[1] = 'S';
        } else {
            name[0] = 'S';
            name[1] = 'V';
        }

        switch (hw) {
        case PROBE_HARDWARE:
            name[2] = 'P';
            break;

        case ROCKET_HARDWARE:
            name[2] = 'R';
            break;

        case MANNED_HARDWARE:
            name[2] = 'C';
            break;

        case MISC_HARDWARE:
            name[2] = 'M';
            break;
        }

        name[3] = 0x30 + unit;      // poor man's itoa()
        name[4] = '\x00';           // terminator

        std::string filename((boost::format("images/liftoff.abz.%1%.png") % name).str());
        std::shared_ptr<display::PalettizedSurface> image(Filesystem::readImage(filename));

        image->exportPalette(32, 2 + 63);
        display::graphics.screen()->draw(image, x, y);
    }
}

void
DispHelp(char top, char bot, const char *txt)
{
    int i = 0;
    int pl = 0;

    while (i++ < top) {
        if (txt[i * 42] == (char) 0xcc) {
            display::graphics.setForegroundColor(txt[i * 42 + 1]);
        }
    }

    i = top;
    fill_rectangle(38, 49, 260, 127, 3);

    while (i <= bot && pl < 11) {
        if (txt[i * 42] == (char) 0xCC) {
            display::graphics.setForegroundColor(txt[i * 42 + 1]);
            draw_string(45, 55 + 7 * pl, &txt[i * 42 + 2]);
        } else {
            draw_string(45, 55 + 7 * pl, &txt[i * 42]);
        }

        pl++;
        i++;
    }

    return;
}

/**
 * Launch a Help dialogue popup.
 *
 * Sets the global AL_CALL to signify a Help popup is open, and
 * another should not be allowed to open.
 *
 * \param FName  the help entry identifier (Ex: "i043")
 * \return  -1 for No, 1 for Yes, 0 for Continue.
 * \throws IOException  if unable to open the help file.
 * \throws runtime_error  if help entry seems invalid.
 */
int Help(const char *FName)
{
    const int PAGE_SIZE = 11;
    int i, j, line, top = 0, bot = 0, plc = 0;
    char *NTxt, mode;
    int fsize;

    mode = 0; /* XXX check uninitialized */
    NTxt = NULL; /* XXX check uninitialized */

    if (!FName || strncmp(&FName[1], "000", 3) == 0) {
        return 0;
    }
   
    for (i = 0; i < Assets->help.size(); i++) {
        if(!xstrncasecmp(Assets->help.at(i).Code.c_str(), FName, 4)) {
            break;
        }
    }

    if (i == Assets->help.size()) {
        CERROR3(baris, "Could not find help entry %s", FName);
        return 0;
    }

    AL_CALL = 1;
    const auto& description = Assets->help.at(i).description;
    std::string str = std::accumulate(description.begin(), description.end(), std::string{},
        [](const std::string& l, const std::string& r) {
            return l + r + "\r\n";
        });
    const char *Help = str.c_str();

    // Process entry
    i = 0;
    j = 0;
    line = 0;
    fsize = 1;

    while (Help[i]) {
        if (Help[i] == 0x3B) while (Help[i++] != 0x0a);  // Remove Comments
        else if (Help[i] == 0x25) {  // Percent for line qty
            i++;
            fsize = 10 * (Help[i] - 0x30) + (Help[i + 1] - 0x30) + 1;
            bot = fsize;

            NTxt = (char *)xmalloc((unsigned int)(42 * fsize));
            memset(NTxt, 0x00, (unsigned int)(42 * fsize));
            j = line * 42;   // should be 0
            mode = Help[i + 3] - 0x30;
            i += 6;
        } else if (Help[i] == 0x2e) {  // Period
            i++;
            assert(NTxt != NULL);
            NTxt[j++] = (char) 0xcc;
            NTxt[j++] = (Help[i] - 0x30) * 100 + (Help[i + 1] - 0x30) * 10 + (Help[i + 2] - 0x30);
            i += 5;
        } else {   // Text of Line
            assert(NTxt != NULL);

            while (Help[i] != 0x0d) {
                NTxt[j++] = Help[i++];
            }

            NTxt[j] = 0x00;
            i += 2;
            line++;
            j = line * 42;
        }
    }

    key = 0;
    display::LegacySurface local(250, 128);
    local.palette().copy_from(display::graphics.legacyScreen()->palette());
    local.draw(*display::graphics.screen(), 34, 32, 250, 128);

    ShBox(34, 32, 283, 159);
    InBox(37, 35, 279, 45);
    InBox(37, 48, 261, 128);
    InBox(264, 48, 279, 128);
    fill_rectangle(265, 49, 278, 127, 0);
    ShBox(266, 50, 277, 87);
    draw_up_arrow(268, 56);
    ShBox(266, 89, 277, 126);
    draw_down_arrow(268, 95);
    fill_rectangle(38, 36, 278, 44, 7);

    if (mode == 0) {
        IOBox(83, 131, 241, 156);
        draw_heading(119, 137, "CONTINUE", 1, 0);
    } else {
        IOBox(83, 131, 156, 156);
        IOBox(168, 131, 241, 156);
        draw_heading(103, 137, "YES", 1, 0);
        draw_heading(192, 137, "NO", 1, 0);
    }

    // Display Title
    display::graphics.setForegroundColor(NTxt[1]);
    fsize = strlen(&NTxt[2]);
    draw_string(157 - fsize * 3, 42, &NTxt[2]);
    top = plc = 1;
    DispHelp(plc, bot - 1, &NTxt[0]);

    if (plc + PAGE_SIZE < bot) {
        draw_down_arrow_highlight(268, 95);
    }

    av_sync();

    WaitForMouseUp();
    i = 2;

    while (i == 2) {
        GetMouse();

        if (mode == 0 && ((x > 85 && y >= 133 && x <= 239 && y <= 154 && mousebuttons > 0) || (key == 'C' || key == K_ENTER || key == K_ESCAPE))) {
            InBox(85, 133, 239, 154);
            i = 0;
            WaitForMouseUp();
            key = 0;
            // Continue Response
        }

        if (mode == 1 && ((x >= 85 && y >= 133 && x <= 154 && y <= 154 && mousebuttons > 0) || key == 'Y')) {
            InBox(85, 133, 154, 154);
            i = 1;
            WaitForMouseUp();
            key = 0;
            // Yes Response
        }

        if (mode == 1 && ((x > 170 && y >= 133 && x <= 239 && y <= 154 && mousebuttons > 0) || key == 'N')) {
            InBox(170, 133, 239, 154);
            i = -1;
            WaitForMouseUp();
            key = 0;
            // No Response
        }

        if (plc > top && ((x >= 266 && y > 50 && x <= 277 && y <= 87 && mousebuttons > 0) || key == UP_ARROW)) {
            InBox(266, 50, 277, 87);
            // WaitForMouseUp();

            if (plc + PAGE_SIZE == bot) {
                draw_down_arrow_highlight(268, 95);
            }

            plc--;
            DispHelp(plc, bot, &NTxt[0]);
            OutBox(266, 50, 277, 87);
            key = 0;

            if (plc <= top) {
                draw_up_arrow(268, 56);
            } else {
                draw_up_arrow_highlight(268, 56);
            }
        }   // Up

        if (plc > top && (key == K_PGUP || key == K_HOME)) {
            // PageUp / Home
            InBox(266, 50, 277, 87);

            if (plc + PAGE_SIZE == bot) {
                draw_down_arrow_highlight(268, 95);
            }

            plc = (key == K_HOME) ? top : MAX(plc - PAGE_SIZE, top);
            plc = MAX(plc - PAGE_SIZE, top);
            DispHelp(plc, bot, &NTxt[0]);
            OutBox(266, 50, 277, 87);
            key = 0;

            if (plc <= top) {
                draw_up_arrow(268, 56);
            } else {
                draw_up_arrow_highlight(268, 56);
            }
        }

        if ((plc + PAGE_SIZE) < bot && ((x >= 266 && y > 89 && x <= 277 && y <= 126 && mousebuttons > 0) || key == DN_ARROW)) {
            InBox(266, 89, 277, 126);
            // WaitForMouseUp();

            if (plc == top) {
                draw_up_arrow_highlight(268, 56);
            }

            plc++;
            DispHelp(plc, bot, &NTxt[0]);
            OutBox(266, 89, 277, 126);
            key = 0;

            if ((plc + PAGE_SIZE) >= bot) {
                draw_down_arrow(268, 95);
            } else {
                draw_down_arrow_highlight(268, 95);
            }
        }   // Down

        if ((plc + PAGE_SIZE) < bot && (key == K_PGDN || key == K_END)) {
            // Page Down / End
            InBox(266, 89, 277, 126);
            // WaitForMouseUp();

            if (plc == top) {
                draw_up_arrow_highlight(268, 56);
            }

            plc = (key == K_END) ? bot - PAGE_SIZE
                  : MIN(plc + PAGE_SIZE, bot - PAGE_SIZE);
            DispHelp(plc, bot, &NTxt[0]);
            OutBox(266, 89, 277, 126);
            key = 0;

            if ((plc + PAGE_SIZE) >= bot) {
                draw_down_arrow(268, 95);
            } else {
                draw_down_arrow_highlight(268, 95);
            }
        }
    }

    local.copyTo(display::graphics.legacyScreen(), 34, 32);
    free(NTxt);

    AL_CALL = 0;
    return i;
}

void writePrestigeFirst(char index)   ///index==plr
{
    char w = 0, i, draw = 0;

    for (i = 0; i < 28; i++) {
        //Prestige First
        if (w < 6 && Data->Prestige[i].Place == index && !(Data->PD[index][i] & 1)) {
            if (draw == 0) {
                ShBox(6, 170, 314, 197);
                fill_rectangle(10, 173, 310, 194, 7);
                InBox(9, 172, 311, 195);
                ShBox(216, 156, 314, 172);
                fill_rectangle(220, 160, 310, 168, 9);
                InBox(219, 159, 311, 169);
                fill_rectangle(216, 171, 216, 171, 3);
                fill_rectangle(312, 172, 313, 172, 3);
                display::graphics.setForegroundColor(11);
                draw_string(224, 166, "PRESTIGE FIRSTS");
                draw = 1;
            }

            display::graphics.setForegroundColor(11);
            draw_string(w > 2 ? 170 : 14,
                        w > 2 ? 179 + (w - 3) * 7 : 179 + w * 7,
                        &PF[i][0]);
            ++w;
            Data->PD[index][i] |= 1;

            // NOTE: This method, inside a loop as it is, depends upon
            // a strict ordering of PrestigeValues such that Duration_F
            // is lower than Duration_E, etc.
            switch (i) {
            case Prestige_Duration_F:
                if (Data->Prestige[Prestige_Duration_E].Place == index &&
                    Data->PD[index][Prestige_Duration_E] == 0) {
                    draw_string(0, 0, ", E");
                    Data->PD[index][Prestige_Duration_E] |= 1;
                }

            case Prestige_Duration_E:
                if (Data->Prestige[Prestige_Duration_D].Place == index &&
                    Data->PD[index][Prestige_Duration_D] == 0) {
                    draw_string(0, 0, ", D");
                    Data->PD[index][Prestige_Duration_D] |= 1;
                }

            case Prestige_Duration_D:
                if (Data->Prestige[Prestige_Duration_C].Place == index &&
                    Data->PD[index][Prestige_Duration_C] == 0) {
                    draw_string(0, 0, ", C");
                    Data->PD[index][Prestige_Duration_C] |= 1;
                }

            case Prestige_Duration_C:
                if (Data->Prestige[Prestige_Duration_B].Place == index &&
                    Data->PD[index][Prestige_Duration_B] == 0) {
                    draw_string(0, 0, ", B");
                    Data->PD[index][Prestige_Duration_B] |= 1;
                }

            case Prestige_Duration_B:
                i = Prestige_Duration_B;

            default:
                break;
            }
        }
    }

    for (i = 0; i < 28; i++) {
        // Prestige Seconds
        if (w < 6 && Data->Prestige[i].mPlace == index &&
            Data->PD[index][i] == 0) {
            if (draw == 0) {
                ShBox(6, 170, 314, 197);
                fill_rectangle(10, 173, 310, 194, 7);
                InBox(9, 172, 311, 195);
                ShBox(216, 156, 314, 172);
                fill_rectangle(220, 160, 310, 168, 9);
                InBox(219, 159, 311, 169);
                fill_rectangle(216, 171, 216, 171, 3);
                fill_rectangle(312, 172, 313, 172, 3);
                display::graphics.setForegroundColor(11);
                draw_string(224, 166, "PRESTIGE FIRSTS");
                draw = 1;
            }

            display::graphics.setForegroundColor(11);
            draw_string(w > 2 ? 170 : 14,
                        w > 2 ? 179 + (w - 3) * 7 : 179 + w * 7,
                        &PF[i][0]);
            ++w;
            Data->PD[index][i] |= 1;

            switch (i) {
            case Prestige_Duration_F:
                if (Data->Prestige[Prestige_Duration_E].mPlace == index &&
                    Data->PD[index][Prestige_Duration_E] == 0) {
                    draw_string(0, 0, ", E");
                    Data->PD[index][Prestige_Duration_E] |= 1;
                }

            case Prestige_Duration_E:
                if (Data->Prestige[Prestige_Duration_D].mPlace == index &&
                    Data->PD[index][Prestige_Duration_D] == 0) {
                    draw_string(0, 0, ", D");
                    Data->PD[index][Prestige_Duration_D] |= 1;
                }

            case Prestige_Duration_D:
                if (Data->Prestige[Prestige_Duration_C].mPlace == index &&
                    Data->PD[index][Prestige_Duration_C] == 0) {
                    draw_string(0, 0, ", C");
                    Data->PD[index][Prestige_Duration_C] |= 1;
                }

            case Prestige_Duration_C:
                if (Data->Prestige[Prestige_Duration_B].mPlace == index &&
                    Data->PD[index][Prestige_Duration_B] == 0) {
                    draw_string(0, 0, ", B");
                    Data->PD[index][Prestige_Duration_B] |= 1;
                }

            case Prestige_Duration_B:
                i = Prestige_Duration_B;

            default:
                break;
            }

            draw_string(0, 0, " (2ND)");
        }
    }
}


/**
 *
 * \param plr   TODO
 * \param index TODO
 * \param mode  0 for replay, 1 for after-mission summary.
 */
void Draw_Mis_Stats(int plr, int index, int mode)
{
    if (mode == 0) {
        InBox(245, 5, 314, 17);
    }

    ShBox(6, 30, 209, 168);  // Left Side
    fill_rectangle(10, 34, 205, 44, 9);
    InBox(9, 33, 206, 45);
    fill_rectangle(10, 49, 205, 119, 7);
    InBox(9, 48, 206, 120);
    fill_rectangle(10, 124, 205, 164, 7);
    InBox(9, 123, 206, 165);
    ShBox(62, 117, 154, 127);
    display::graphics.setForegroundColor(1);
    draw_string(79, 124, "FLIGHT CREW");
    display::graphics.setForegroundColor(11);
    draw_string(58, 41, "MISSION INFORMATION");
    draw_string(12, 104, "MISSION DURATION: ");

    char duration = 'A' + Data->P[plr].History[index].Duration - 1;

    if (duration >= 'A' && duration <= 'F') {
        draw_character(duration);
    } else {
        draw_string(0, 0, "NONE");
    }

    draw_string(12, 112, "PRESTIGE EARNED: ");

    if ((MAIL != 0 && MAIL != 3 && Option == -1) || mode == 0) {
        draw_number(0, 0, Data->P[plr].History[index].Prestige);
    } else {
        draw_string(0, 0, "PENDING");
    }

    int mcode = Data->P[plr].History[index].MissionCode;
    const struct mStr plan = GetMissionPlan(mcode);

    display::graphics.setForegroundColor(1);
    draw_string(12, 56, "MISSION NAME: ");
    draw_string(0, 0, (char *)Data->P[plr].History[index].MissionName);
    draw_string(12, 64, "MISSION TYPE:");
    display::graphics.setForegroundColor(11);
    draw_string(15, 72, plan.Abbr);

    display::graphics.setForegroundColor(1);
    draw_string(12, 80, "RESULT: ");

    if (Data->P[plr].History[index].Duration != 0) {

        if (Data->P[plr].History[index].spResult == 4197) {
            draw_string(0, 0, "PRIMARY CREW DEAD");
        } else if (Data->P[plr].History[index].spResult == 4198) {
            draw_string(0, 0, "SECONDARY CREW DEAD");
        } else if (Data->P[plr].History[index].spResult == 4199) {
            draw_string(0, 0, "BOTH CREWS DEAD");
        } else draw_string(0, 0, ((Data->P[plr].History[index].spResult < 500) || (Data->P[plr].History[index].spResult >= 5000)) ? "SUCCESS" :
                               (Data->P[plr].History[index].spResult < 1999) ? "PARTIAL FAILURE" :
                               (Data->P[plr].History[index].spResult == 1999) ? "FAILURE" :
                               (Data->P[plr].History[index].spResult < 3000) ? "MISSION INJURY" :
                               (Data->P[plr].History[index].spResult < 4000) ? "MISSION DEATH" :
                               "ALL DEAD");
    } else if (Data->P[plr].History[index].Event == 0) {
        draw_string(0, 0, (Data->P[plr].History[index].spResult == 1) ? "SUCCESS" : "FAILURE");
    } else if (Data->P[plr].History[index].Event > 0) {
        draw_string(0, 0, "ARRIVE IN ");
        draw_number(0, 0, Data->P[plr].History[index].Event);
        draw_string(0, 0, " SEASON");

        if (Data->P[plr].History[index].Event >= 2) {
            draw_string(0, 0, "S");
        }
    }

    fill_rectangle(210, 30, 211, 168, 0);  // Clear any leftover stuff
    ShBox(211, 30, 313, 168);  // Right Side
    fill_rectangle(215, 34, 309, 44, 9);
    InBox(214, 33, 310, 45);
    display::graphics.setForegroundColor(11);
    draw_string(225, 41, "MISSION REPLAY");
    InBox(214, 55, 310, 116);

    AbzFrame(plr, 215, 56, 94, 60, index);

    IOBox(214, 134, 310, 148);
    display::graphics.setForegroundColor(9);
    draw_string(224, 143, "R");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "EPLAY MISSION");

    if (mode == 0) {
        IOBox(214, 151, 310, 165);
        display::graphics.setForegroundColor(1);
        draw_string(252, 160, "EXIT");
    }

    // Crew Stuff
    display::graphics.setForegroundColor(11);

    if (Data->P[plr].History[index].Man[PAD_A][0] == -1 &&
        Data->P[plr].History[index].Man[PAD_B][0] == -1) {
        draw_string(13, 137, "UNMANNED MISSION");
    } else {
        int let = 0;

        // First Part -- Men
        if (Data->P[plr].History[index].Man[PAD_A][0] != -1) {
            for (int j = 0; j < 4; j++) {
                int k = Data->P[plr].History[index].Man[PAD_A][j];

                if (Data->P[plr].Pool[k].Sex == 1) {
                    display::graphics.setForegroundColor(18);    // Display women in blue, not yellow
                }

                if (k != -1) {
                    draw_string(13, 137 + j * 7, Data->P[plr].Pool[k].Name);
                }

                display::graphics.setForegroundColor(11);
            }

            let = 1;    // Men on Part 1
        }

        // Second Part -- Men
        if (Data->P[plr].History[index].Man[PAD_B][0] != -1) {
            for (int j = 0; j < 4; j++) {
                int k = Data->P[plr].History[index].Man[PAD_B][j];

                if (Data->P[plr].Pool[k].Sex == 1) {
                    // Display women in blue, not yellow
                    display::graphics.setForegroundColor(18);
                }

                if (k != -1) {
                    draw_string(13 + let * 100, 137 + j * 7,
                                Data->P[plr].Pool[k].Name);
                }

                display::graphics.setForegroundColor(11);
            }
        }

    }

    writePrestigeFirst(plr);

    if (mode == 1) {
        FadeIn(2, 10, 0, 0);
    }

    WaitForMouseUp();

    while (1) {
        GetMouse();

        if (mode == 0 && ((x >= 216 && y >= 153 && x <= 308 && y <= 163 && mousebuttons == 1) || (key == K_ENTER || key == 'E'))) {
            InBox(216, 153, 308, 153);
            OutBox(245, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            key = 0;
            break;
        } else if (mode == 1 && ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons == 1) || key == K_ENTER)) {
            InBox(245, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            OutBox(245, 5, 314, 17);
            delay(10);

            if (!AI[plr]) {
                music_stop();
            }

            FadeOut(2, 10, 0, 0);
            key = 0;
            break;
        } else if ((x >= 216 && y >= 136 && x <= 308 && y <= 146 && mousebuttons == 1) || (key == 'R')) {
            InBox(216, 136, 308, 146);

            if (mode == 0) {
                InBox(216, 153, 309, 163);
            }

            display::graphics.setForegroundColor(11);
            draw_string(225, 125, "PLAYING...");

            WaitForMouseUp();
            display::graphics.setForegroundColor(1);

            if (x == 0 && y == 0) {
                // Create temp image file
                FILE *tin = sOpen("REPL.TMP", "wb", FT_SAVE);

                {
                    display::AutoPal p(display::graphics.legacyScreen());
                    fwrite(p.pal, 768, 1, tin);
                }
                fwrite(display::graphics.legacyScreen()->pixels(), 64000, 1, tin);
                fclose(tin);
                FadeOut(2, 10, 0, 0);
                display::graphics.screen()->clear();
                FadeIn(2, 10, 0, 0);

                if (Data->P[plr].History[index].MissionCode == Mission_MarsFlyby ||
                    Data->P[plr].History[index].MissionCode == Mission_JupiterFlyby ||
                    Data->P[plr].History[index].MissionCode == Mission_SaturnFlyby) {
                    if (Data->P[plr].History[index].Event == 0 &&
                        Data->P[plr].History[index].spResult == 1)
                        switch (Data->P[plr].History[index].MissionCode) {
                        case Mission_MarsFlyby:
                            Replay(plr, index, 0, 0, 320, 200, (plr == 0) ? "WUM1" : "WSM1");
                            break;

                        case Mission_JupiterFlyby:
                            Replay(plr, index, 0, 0, 320, 200, (plr == 0) ? "WUJ1" : "WSJ1");
                            break;

                        case Mission_SaturnFlyby:
                            Replay(plr, index, 0, 0, 320, 200, (plr == 0) ? "WUS1" : "WSS1");
                            break;

                        default:
                            break;
                        };
                } else {
                    Replay(plr, index, 0, 0, 320, 200, "OOOO");
                }

                FadeOut(2, 10, 0, 0);
                // reload temp image file
                tin = sOpen("REPL.TMP", "rb", FT_SAVE);
                {
                    display::AutoPal p(display::graphics.legacyScreen());
                    fread(p.pal, 768, 1, tin);
                }
                fread(display::graphics.legacyScreen()->pixels(), 64000, 1, tin);
                fclose(tin);
                FadeIn(2, 10, 0, 0);
                key = 0;
                remove_savedat("REPL.TMP");

            } else {
                //Specs: Planetary Mission Kludge
                if (Data->P[plr].History[index].MissionCode == Mission_MarsFlyby ||
                    Data->P[plr].History[index].MissionCode == Mission_JupiterFlyby ||
                    Data->P[plr].History[index].MissionCode == Mission_SaturnFlyby) {
                    if (Data->P[plr].History[index].Event == 0 &&
                        Data->P[plr].History[index].spResult == 1) {
                        switch (Data->P[plr].History[index].MissionCode) {
                        case Mission_MarsFlyby:
                            Replay(plr, index, 215, 56, 94, 60, (plr == 0) ? "WUM1" : "WSM1");
                            break;

                        case Mission_JupiterFlyby:
                            Replay(plr, index, 215, 56, 94, 60, (plr == 0) ? "WUJ1" : "WSJ1");
                            break;

                        case Mission_SaturnFlyby:
                            Replay(plr, index, 215, 56, 94, 60, (plr == 0) ? "WUS1" : "WSS1");
                            break;
                        }
                    } else {
                        Replay(plr, index, 215, 56, 94, 60, "OOOO");
                    }
                } else {
                    Replay(plr, index, 215, 56, 94, 60, "OOOO");
                }

                AbzFrame(plr, 215, 56, 94, 60, index);
            }

            OutBox(216, 136, 308, 146);

            if (mode == 0) {
                OutBox(216, 153, 309, 163);
            }

            fill_rectangle(212, 119, 312, 127, 3);
            key = 0;

        }  // if
    }  // while

    display::graphics.videoRect().w = 0;
    display::graphics.videoRect().h = 0;
}


/**
 * Prompts if the specified mission should be scrubbed.
 *
 * Chooses the correct Scrub Mission prompt from the help files
 * and launches it for human players.
 *
 * \param plr  The player index (0 for USA, 1 for USSR)
 * \param pad  The mission to scrub (pad 0, 1, or 2)
 * \return  false to cancel, true to continue
 */
bool ScrubMissionQuery(const char plr, const int pad)
{
    if (AI[plr]) {
        return true;
    }

    // Different Help prompts are used under different circumstances
    // "i110" - first pad of joint mission
    // "i111" - non-joint mission
    // "i112" - second pad of joint mission
    if (Data->P[plr].Mission[pad].Joint == 0) {
        return Help("i111") >= 0;
    } else if (Data->P[plr].Mission[pad].part == 0) {
        return Help("i110") >= 0;
    } else {
        return Help("i112") >= 0;
    }
}

// EOF
