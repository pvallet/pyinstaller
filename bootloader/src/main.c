/*
 * ****************************************************************************
 * Copyright (c) 2013-2023, PyInstaller Development Team.
 *
 * Distributed under the terms of the GNU General Public License (version 2
 * or later) with exception for distributing the bootloader.
 *
 * The full license is in the file COPYING.txt, distributed with this software.
 *
 * SPDX-License-Identifier: (GPL-2.0-or-later WITH Bootloader-exception)
 * ****************************************************************************
 */

/* This file has a total of three different entry points, with one of them chosen
 * using preprocessor defines.
 *
 * wWinMain: For Windows with console=False
 * wmain: For Windows with console=True
 * main: For OS X and Linux
 */

#ifdef _WIN32
    #include <windows.h>
    #include <wchar.h>

    /* Prevent the MS CRT from expanding wildcards in command-line arguments. */
    int _CRT_glob = 0;

    // BEGIN pvallet-modification
    /* On most laptops, two graphics processors are available: one dedicated, and an integrated chipset.
     * It allows to run desktop apps on the chipset, to prevent using too much battery.
     * Games and other graphics-intensive apps are running on the dedicated card.
     * This parameter allows us to request running on the dedicated card.
     * It maximizes the chances for having a valid OpenGL profile and get better performance.
     * 
     * This only works when part of the executable, or statically linked to it, which is why we need to 
     * manually modify pyinstaller's bootloader, and we can't rely on exposing those variables from a dynamically 
     * loaded python module. To make that module work on your laptop in a development environment,
     * you should open the nvidia config panel and select the appropriate graphics processor for Python.exe
     */
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
    // END pvallet-modification
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include "pyi_main.h"
#include "pyi_global.h"
#include "pyi_win32_utils.h"

#ifdef __FreeBSD__
    #include <floatingpoint.h>
#endif

#if defined(_WIN32)
    #define MS_WINDOWS
#endif

#if defined(_WIN32)

    #if defined(WINDOWED)

/* Entry point for Windows when console=False */

int WINAPI
wWinMain(
    HINSTANCE hInstance,      /* handle to current instance */
    HINSTANCE hPrevInstance,  /* handle to previous instance */
    LPWSTR lpCmdLine,         /* pointer to command line */
    int nCmdShow              /* show state of window */
    )
{
    /* store wargv in argv as UTF-8 - decode later when used. */
    char ** u8argv = pyi_win32_argv_to_utf8(__argc, __wargv);

    return pyi_main(__argc, u8argv);
}

    #else /* defined(WINDOWED) */

/* Entry point for Windows when console=True */

int
wmain(int argc, wchar_t* argv[])
{
    char ** u8argv = pyi_win32_argv_to_utf8(__argc, __wargv);

    return pyi_main(argc, u8argv);
}

    #endif /* defined(WINDOWED) */

#else  /* defined(_WIN32) */

/* Based on main() from Modules/python.c
 *
 *  Entry point for Linux/OS X
 */

int
main(int argc, char **argv)
{
    int res;

    #ifdef __FreeBSD__
    fp_except_t m;

    /* 754 requires that FP exceptions run in "no stop" mode by default,
     * and until C vendors implement C99's ways to control FP exceptions,
     * Python requires non-stop mode.  Alas, some platforms enable FP
     * exceptions by default.  Here we disable them.
     */
    m = fpgetmask();
    fpsetmask(m & ~FP_X_OFL);
    #endif

    res = pyi_main(argc, argv);
    return res;
}

#endif  /* defined(WIN32) */
