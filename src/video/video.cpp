/*
 * video.cpp
 *
 * Copyright (C) 2001 Matt Ownby
 *
 * This file is part of DAPHNE, a laserdisc arcade game emulator
 *
 * DAPHNE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * DAPHNE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// video.cpp
// Part of the DAPHNE emulator
// This code started by Matt Ownby, May 2000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>      // for some error messages
#include <SDL_syswm.h> // rdg2010
#include "video.h"
#include "palette.h"
#include "../io/conout.h"
#include "../io/error.h"
#include "../io/mpo_fileio.h"
#include "../io/mpo_mem.h"
#include "../game/game.h"
#include "../ldp-out/ldp.h"

using namespace std;

unsigned int g_vid_width = 640, g_vid_height = 480; // default video width and
                                                    // video height
#ifdef DEBUG
const Uint16 cg_normalwidths[]  = {320, 640, 800, 1024, 1280, 1280, 1600};
const Uint16 cg_normalheights[] = {240, 480, 600, 768, 960, 1024, 1200};
#else
const Uint16 cg_normalwidths[]  = {640, 800, 1024, 1280, 1280, 1600};
const Uint16 cg_normalheights[] = {480, 600, 768, 960, 1024, 1200};
#endif // DEBUG

// the dimensions that we draw (may differ from g_vid_width/height if aspect
// ratio is enforced)
unsigned int g_draw_width = 640, g_draw_height = 480;

FC_Font *g_font = NULL;
SDL_Surface *g_led_bmps[LED_RANGE] = {0};
SDL_Surface *g_other_bmps[B_EMPTY] = {0};
SDL_Window  *g_window              = NULL;
SDL_Renderer *g_renderer           = NULL;
SDL_Surface *g_screen              = NULL; // our primary display
SDL_Surface *g_screen_blitter      = NULL; // the surface we blit to (we don't blit
                                      // directly to g_screen because opengl
                                      // doesn't like that)
bool g_fullscreen          = false; // whether we should initialize video in fullscreen
                                    // mode or not
int g_scalefactor = 100; // by RDG2010 -- scales the image to this percentage
                         // value (for CRT TVs with overscan problems).
int sboverlay_characterset = 1;

// whether we will try to force a 4:3 aspect ratio regardless of window size
// (this is probably a good idea as a default option)
bool g_bForceAspectRatio = true;

// the # of degrees to rotate counter-clockwise in opengl mode
float g_fRotateDegrees = 0.0;

////////////////////////////////////////////////////////////////////////////////////////////////////

// initializes the window in which we will draw our BMP's
// returns true if successful, false if failure
bool init_display()
{

    bool result                  = false; // whether video initialization is successful or not
    Uint32 sdl_flags             = 0;

    sdl_flags = SDL_WINDOW_SHOWN;

    char s[250] = {0};
    Uint32 x    = 0; // temporary index

    // if we were able to initialize the video properly
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) >= 0) {

        if (g_fullscreen) sdl_flags |= SDL_WINDOW_FULLSCREEN;

        g_draw_width  = g_vid_width;
        g_draw_height = g_vid_height;

        // if we're supposed to enforce the aspect ratio ...
        if (g_bForceAspectRatio) {
            double dCurAspectRatio            = (double)g_vid_width / g_vid_height;
            const double dTARGET_ASPECT_RATIO = 4.0 / 3.0;

            // if current aspect ratio is less than 1.3333
            if (dCurAspectRatio < dTARGET_ASPECT_RATIO) {
                g_draw_height = (g_draw_width * 3) / 4;
            }
            // else if current aspect ratio is greater than 1.3333
            else if (dCurAspectRatio > dTARGET_ASPECT_RATIO) {
                g_draw_width = (g_draw_height * 4) / 3;
            }
            // else the aspect ratio is already correct, so don't change
            // anything
        }

        // if we're supposed to scale the image...
        if (g_scalefactor < 100) {
            g_draw_width  = g_draw_width * g_scalefactor / 100;
            g_draw_height = g_draw_height * g_scalefactor / 100;
        }

            // by RDG2010
            // Step 2. Create a borderless SDL window.
            // If doing fullscreen window, make the window bordeless (no title
            // bar).
            // This is achieved by adding the SDL_NOFRAME flag.

        g_window = SDL_CreateWindow("DAPHNE: First Ever Multiple Arcade Laserdisc Emulator =]",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    g_vid_width,
                                    g_vid_height,
                                    sdl_flags);
        if (!g_window) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize window: %s", SDL_GetError());
        } else {
            g_renderer = SDL_CreateRenderer(g_window, -1, 0);
	    if (!g_renderer) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize renderer: %s", SDL_GetError());
	    } else {
                g_font = FC_CreateFont();
                FC_LoadFont(g_font, g_renderer, "fonts/default.ttf", 18, FC_MakeColor(0,0,0,255), TTF_STYLE_NORMAL);

                // create a 32-bit surface
                g_screen_blitter =
                    SDL_CreateRGBSurface(SDL_SWSURFACE, g_vid_width, g_vid_height, 32,
                                         0xff, 0xFF00, 0xFF0000, 0xFF000000);
        
                //if (g_screen && g_screen_blitter) {
                //    sprintf(s, "Set %dx%d at %d bpp with flags: %x", g_screen->w,
                //            g_screen->h, g_screen->format->BitsPerPixel, g_screen->flags);
                //    printline(s);
        
                    // NOTE: SDL Console was initialized here.
                    result                = true;
                //}
            }
        }
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL: %s", SDL_GetError());
    }

    return (result);
}

// shuts down video display
void shutdown_display()
{
    printline("Shutting down video display...");

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void vid_blit(SDL_Surface *srf, int x, int y)
{
    if (g_ldp->is_blitting_allowed()) {
            SDL_Rect dest;
            dest.x = (short)x;
            dest.y = (short)y;
            dest.w = (unsigned short)srf->w;
            dest.h = (unsigned short)srf->h;
            SDL_BlitSurface(srf, NULL, g_screen, &dest);
    }
    // else blitting isn't allowed, so just ignore
}

// redraws the proper display (Scoreboard, etc) on the screen, after first
// clearing the screen
// call this every time you want the display to return to normal
void display_repaint()
{
    SDL_RenderClear(g_renderer);
    SDL_RenderPresent(g_renderer);
    g_game->video_force_blit();
}

// loads all the .bmp's used by DAPHNE
// returns true if they were all successfully loaded, or a false if they weren't
bool load_bmps()
{

    bool result = true; // assume success unless we hear otherwise
    int index   = 0;
    char filename[81];

    for (; index < LED_RANGE; index++) {
        sprintf(filename, "pics/led%d.bmp", index);

        g_led_bmps[index] = load_one_bmp(filename);

        // If the bit map did not successfully load
        if (g_led_bmps[index] == 0) {
            result = false;
        }
    }

    g_other_bmps[B_DL_PLAYER1]    = load_one_bmp("pics/player1.bmp");
    g_other_bmps[B_DL_PLAYER2]    = load_one_bmp("pics/player2.bmp");
    g_other_bmps[B_DL_LIVES]      = load_one_bmp("pics/lives.bmp");
    g_other_bmps[B_DL_CREDITS]    = load_one_bmp("pics/credits.bmp");
    g_other_bmps[B_DAPHNE_SAVEME] = load_one_bmp("pics/saveme.bmp");
    g_other_bmps[B_GAMENOWOOK]    = load_one_bmp("pics/gamenowook.bmp");

    if (sboverlay_characterset != 2)
        g_other_bmps[B_OVERLAY_LEDS] = load_one_bmp("pics/overlayleds1.bmp");
    else
        g_other_bmps[B_OVERLAY_LEDS] = load_one_bmp("pics/overlayleds2.bmp");

    g_other_bmps[B_OVERLAY_LDP1450] = load_one_bmp("pics/ldp1450font.bmp");

    // check to make sure they all loaded
    for (index = 0; index < B_EMPTY; index++) {
        if (g_other_bmps[index] == NULL) {
            result = false;
        }
    }

    return (result);
}

SDL_Surface *load_one_bmp(const char *filename)
{
    SDL_Surface *result = SDL_LoadBMP(filename);

    if (!result) {
        string err = "Could not load bitmap : ";
        err = err + filename;
        printerror(err.c_str());
    }

    return (result);
}

// Draw's one of our LED's to the screen
// value contains the bitmap to draw (0-9 is valid)
// x and y contain the coordinates on the screen
// This function is called from scoreboard.cpp
// 1 is returned on success, 0 on failure
bool draw_led(int value, int x, int y)
{
    vid_blit(g_led_bmps[value], x, y);
    return true;
}

// Draw overlay digits to the screen
void draw_overlay_leds(unsigned int values[], int num_digits, int start_x,
                       int y, SDL_Surface *overlay)
{
    SDL_Rect src, dest;

    dest.x = start_x;
    dest.y = y;
    dest.w = OVERLAY_LED_WIDTH;
    dest.h = OVERLAY_LED_HEIGHT;

    src.y = 0;
    src.w = OVERLAY_LED_WIDTH;
    src.h = OVERLAY_LED_HEIGHT;

    /* Draw the digit(s) */
    for (int i = 0; i < num_digits; i++) {
        src.x = values[i] * OVERLAY_LED_WIDTH;
        SDL_BlitSurface(g_other_bmps[B_OVERLAY_LEDS], &src, overlay, &dest);
        dest.x += OVERLAY_LED_WIDTH;
    }

    dest.x = start_x;
    dest.w = num_digits * OVERLAY_LED_WIDTH;
    //SDL_UpdateRects(overlay, 1, &dest); FIXME
}

// Draw LDP1450 overlay characters to the screen (added by Brad O.)
void draw_singleline_LDP1450(char *LDP1450_String, int start_x, int y, SDL_Surface *overlay)
{
    SDL_Rect src, dest;

    int i     = 0;
    int value = 0;
    int LDP1450_strlen;
    //	char s[81]="";

    dest.x = start_x;
    dest.y = y;
    dest.w = OVERLAY_LDP1450_WIDTH;
    dest.h = OVERLAY_LDP1450_HEIGHT;

    src.y = 0;
    src.w = OVERLAY_LDP1450_WIDTH;
    src.h = OVERLAY_LDP1450_WIDTH;

    LDP1450_strlen = strlen(LDP1450_String);

    if (!LDP1450_strlen) // if a blank line is sent, we must blank out the
                         // entire line
    {
        strcpy(LDP1450_String, "           ");
        LDP1450_strlen = strlen(LDP1450_String);
    } else {
        if (LDP1450_strlen <= 11) // pad end of string with spaces (in case
                                  // previous line was not cleared)
        {
            for (i = LDP1450_strlen; i <= 11; i++) LDP1450_String[i] = 32;
            LDP1450_strlen = strlen(LDP1450_String);
        }
    }

    for (i = 0; i < LDP1450_strlen; i++) {
        value = LDP1450_String[i];

        if (value >= 0x26 && value <= 0x39) // numbers and symbols
            value -= 0x25;
        else if (value >= 0x41 && value <= 0x5a) // alpha
            value -= 0x2a;
        else if (value == 0x13) // special LDP-1450 character (inversed space)
            value = 0x32;
        else
            value = 0x31; // if not a number, symbol, or alpha, recognize as a
                          // space

        src.x = value * OVERLAY_LDP1450_WIDTH;
        SDL_BlitSurface(g_other_bmps[B_OVERLAY_LDP1450], &src, overlay, &dest);

        dest.x += OVERLAY_LDP1450_CHARACTER_SPACING;
    }
    dest.x = start_x;
    dest.w = LDP1450_strlen * OVERLAY_LDP1450_CHARACTER_SPACING;

    // MPO : calling UpdateRects probably isn't necessary and may be harmful
    // SDL_UpdateRects(overlay, 1, &dest);
}

//  used to draw non LED stuff like scoreboard text
//  'which' corresponds to enumerated values
bool draw_othergfx(int which, int x, int y, bool bSendToScreenBlitter)
{
    // NOTE : this is drawn to g_screen_blitter, not to g_screen,
    //  to be more friendly to our opengl implementation!
    SDL_Surface *srf = g_other_bmps[which];
    SDL_Rect dest;
    dest.x = (short)x;
    dest.y = (short)y;
    dest.w = (unsigned short)srf->w;
    dest.h = (unsigned short)srf->h;

    // if we should blit this to the screen blitter for later use ...
    if (bSendToScreenBlitter) {
        SDL_BlitSurface(srf, NULL, g_screen_blitter, &dest);
    }
    // else blit it now
    else {
        vid_blit(g_other_bmps[which], x, y);
    }
    return true;
}

// de-allocates all of the .bmps that we have allocated
void free_bmps()
{

    int nuke_index = 0;

    // get rid of all the LED's
    for (; nuke_index < LED_RANGE; nuke_index++) {
        free_one_bmp(g_led_bmps[nuke_index]);
    }
    for (nuke_index = 0; nuke_index < B_EMPTY; nuke_index++) {
        // check to make sure it exists before we try to free
        if (g_other_bmps[nuke_index]) {
            free_one_bmp(g_other_bmps[nuke_index]);
        }
    }
}

void free_one_bmp(SDL_Surface *candidate) { SDL_FreeSurface(candidate); }

//////////////////////////////////////////////////////////////////////////////////////////

SDL_Renderer *get_screen() { return g_renderer; }

SDL_Surface *get_screen_blitter() { return g_screen_blitter; }

bool get_fullscreen() { return g_fullscreen; }

// sets our g_fullscreen bool (determines whether will be in fullscreen mode or
// not)
void set_fullscreen(bool value) { g_fullscreen = value; }

int get_scalefactor() { return g_scalefactor; }
void set_scalefactor(int value)
{
    if (value > 100 || value < 50) // Validating in case user inputs crazy
                                   // values.
    {
        printline("Invalid scale value. Ignoring -scalefactor parameter.");
        g_scalefactor = 100;

    } else {
        g_scalefactor = value;
    }
}

void set_rotate_degrees(float fDegrees) { g_fRotateDegrees = fDegrees; }

void set_sboverlay_characterset(int value) { sboverlay_characterset = value; }

// returns video width
Uint16 get_video_width() { return g_vid_width; }

// sets g_vid_width
void set_video_width(Uint16 width)
{
    // Let the user specify whatever width s/he wants (and suffer the
    // consequences)
    // We need to support arbitrary resolution to accomodate stuff like screen
    // rotation
    g_vid_width = width;
}

// returns video height
Uint16 get_video_height() { return g_vid_height; }

// sets g_vid_height
void set_video_height(Uint16 height)
{
    // Let the user specify whatever height s/he wants (and suffer the
    // consequences)
    // We need to support arbitrary resolution to accomodate stuff like screen
    // rotation
    g_vid_height = height;
}

// converts YUV to RGB
// Use this only when you don't care about speed =]
// NOTE : it is important for y, u, and v to be signed
void yuv2rgb(SDL_Color *result, int y, int u, int v)
{
    // NOTE : Visual C++ 7.0 apparently has a bug
    // in its floating point optimizations because this function
    // will return incorrect results when compiled as a Release
    // Possible workaround: use integer math instead of float? or don't use VC++
    // 7? hehe

    int b = (int)(1.164 * (y - 16) + 2.018 * (u - 128));
    int g = (int)(1.164 * (y - 16) - 0.813 * (v - 128) - 0.391 * (u - 128));
    int r = (int)(1.164 * (y - 16) + 1.596 * (v - 128));

    // clamp values (not sure if this is necessary, but we aren't worried about
    // speed)
    if (b > 255) b = 255;
    if (b < 0) b = 0;
    if (g > 255) g = 255;
    if (g < 0) g = 0;
    if (r > 255) r = 255;
    if (r < 0) r = 0;

    result->r = (unsigned char)r;
    result->g = (unsigned char)g;
    result->b = (unsigned char)b;
}

FC_Font *get_font()
{
    return g_font;
}

void draw_string(const char *t, int col, int row, SDL_Renderer *renderer)
{
    SDL_Rect dest;

    dest.x = (short)((col * 6));
    dest.y = (short)((row * 13));
    dest.w = (unsigned short)(6 * strlen(t)); // width of rectangle area to draw
                                              // (width of font * length of
                                              // string)
    dest.h = 13;                              // height of area (height of font)

    //SDL_FillRect(overlay, &dest, 0); // erase anything at our destination before
    //                                 // we print new text
    FC_Draw(g_font, renderer, dest.x, dest.y, t);
    //SDL_UpdateRects(overlay, 1, &dest); FIXME
}

// toggles fullscreen mode
void vid_toggle_fullscreen()
{
    // Commented out because it creates major problems with YUV overlays and
    // causing segfaults..
    // The real way to toggle fullscreen is to kill overlay, kill surface, make
    // surface, make overlay.
    /*
    g_screen = SDL_SetVideoMode(g_screen->w,
        g_screen->h,
        g_screen->format->BitsPerPixel,
        g_screen->flags ^ SDL_FULLSCREEN);
        */
}

void set_force_aspect_ratio(bool bEnabled) { g_bForceAspectRatio = bEnabled; }

bool get_force_aspect_ratio() { return g_bForceAspectRatio; }
