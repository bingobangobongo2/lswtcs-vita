/*
 * Copyright (C) 2023 Volodymyr Atamanenko
 * Copyright (C) 2024 Jan Smialkowski
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

/**
 * @file  patch.c
 * @brief Patching some of the .so internal functions or bridging them to native
 *        for better compatibility.
 */

#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <kubridge.h>
#include <so_util/so_util.h>

#include "reimpl/keycodes.h"
#include "reimpl/native_window.h"
#include "utils/gamepad.h"
#include "utils/logger.h"
#include "utils/settings.h"
#include "utils/utils.h"

extern so_module so_mod;

extern uint8_t g_rawRX;
extern uint8_t g_rawRY;

// Captured nupad_s pointer for controls_thread continuous overwrite
volatile void *g_nupad_ptr = NULL;

so_hook NuPadRead_hook;

static int diag_counter = 0;
static int diag_lines = 0;

int NuPadRead_soloader(void *nupad) {
    uint8_t *p = (uint8_t *)nupad;

    // Snapshot BEFORE original runs (previous frame's residual)
    uint8_t prev_a0 = p[0xA0], prev_a1 = p[0xA1];

    int ret = SO_CONTINUE(int, NuPadRead_hook, nupad);

    // Snapshot AFTER original pipeline
    uint8_t orig_a0 = p[0xA0], orig_a1 = p[0xA1];

    g_nupad_ptr = nupad;

    // Overwrite right stick bytes only (0xA0=RS_X, 0xA1=RS_Y)
    p[0xA0] = g_rawRX;
    p[0xA1] = g_rawRY;

    // Log once/sec for 2 min — wiggle stick in all 4 directions
    if (diag_lines < 120 && ++diag_counter >= 60) {
        diag_counter = 0;
        diag_lines++;
        SceUID fd = sceIoOpen("ux0:/data/lswtcs/stick_log.txt",
                               SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0666);
        if (fd >= 0) {
            char buf[200];
            int len = sceClibSnprintf(buf, sizeof(buf),
                "raw=%3d,%3d orig=%3d,%3d prev=%3d,%3d\n",
                g_rawRX, g_rawRY,
                orig_a0, orig_a1,
                prev_a0, prev_a1);
            sceIoWrite(fd, buf, len);
            sceIoClose(fd);
        }
    }

    return ret;
}

so_hook _Z11AndroidMainPv_hook;

/// AndroidMain(void*)
void *_Z11AndroidMainPv_soloader(void *param_1) {
    return SO_CONTINUE(void*, _Z11AndroidMainPv_hook, param_1);
}

so_hook _Z17renderThread_mainPv_hook;

/// renderThread_main(void*)
void *_Z17renderThread_mainPv_soloader(void *param_1) {
    return SO_CONTINUE(void*, _Z17renderThread_mainPv_hook, param_1);
}

so_hook _ZN15NuInputDevicePS21GetGamePadButtonIndexEiPi_hook;

/// NuInputDevicePS::GetGamePadButtonIndex(int, int*)
unsigned int _ZN15NuInputDevicePS21GetGamePadButtonIndexEiPi_soloader(int android_code, int *is_success) {
    *is_success = 1;

    switch (android_code) {
        case AKEYCODE_HOME:
        case AKEYCODE_BUTTON_START:
            return GAMEPAD_START;
        case AKEYCODE_DPAD_UP:
        case AKEYCODE_DPAD_DOWN:
        case AKEYCODE_DPAD_LEFT:
        case AKEYCODE_DPAD_RIGHT:
            return 0;
        case AKEYCODE_BUTTON_A:
            return GAMEPAD_JUMP;
        case AKEYCODE_BUTTON_B:
            return GAMEPAD_SPECIAL;
        case AKEYCODE_BUTTON_X:
            return GAMEPAD_ACTION;
        case AKEYCODE_BUTTON_Y:
            return GAMEPAD_TAG;
        case AKEYCODE_BUTTON_L1:
            return GAMEPAD_L1;
        case AKEYCODE_BUTTON_R1:
            return GAMEPAD_R1;
        case AKEYCODE_BUTTON_L2:
            return GAMEPAD_L2;
        case AKEYCODE_BUTTON_R2:
            return GAMEPAD_R2;
        case AKEYCODE_BUTTON_THUMBL:
            return GAMEPAD_L3;
        case AKEYCODE_BUTTON_THUMBR:
            return GAMEPAD_R3;
        default:
            *is_success = 0;
            return 0;
    }
}

so_hook _ZN8NuThread10ThreadMainEPv_hook;

/// NuThread::ThreadMain(void*)
void *_ZN8NuThread10ThreadMainEPv_soloader(void *param_1) {
    return SO_CONTINUE(void*, _ZN8NuThread10ThreadMainEPv_hook, param_1);
}

#ifdef DEBUG
so_hook NuFileOpen_hook;

int NuFileOpen_soloader(char *param_1, int param_2) {
    int ret = SO_CONTINUE(int, NuFileOpen_hook, param_1, param_2);

    if (ret)
        l_debug("NuFileOpen(%s, %d): %d", param_1, param_2, ret);
    else
        l_warn("NuFileOpen(%s, %d): %d", param_1, param_2, ret);

    return ret;
}
#endif

void so_patch(void) {
    // Right stick fix: hook NuPadRead to overwrite right stick bytes
    // after the original's broken float→byte conversion
    NuPadRead_hook = hook_addr((uintptr_t) so_symbol(&so_mod, "NuPadRead"),
                               (uintptr_t) &NuPadRead_soloader);

    _Z11AndroidMainPv_hook = hook_addr((uintptr_t) so_symbol(&so_mod, "_Z11AndroidMainPv"),
                                       (uintptr_t) &_Z11AndroidMainPv_soloader);

    _Z17renderThread_mainPv_hook = hook_addr((uintptr_t) so_symbol(&so_mod, "_Z17renderThread_mainPv"),
                                             (uintptr_t) &_Z17renderThread_mainPv_soloader);

    if (setting_patchControls == true) {
        _ZN15NuInputDevicePS21GetGamePadButtonIndexEiPi_hook = hook_addr(
            (uintptr_t) so_symbol(&so_mod, "_ZN15NuInputDevicePS21GetGamePadButtonIndexEiPi"),
            (uintptr_t) &_ZN15NuInputDevicePS21GetGamePadButtonIndexEiPi_soloader);
    }

    _ZN8NuThread10ThreadMainEPv_hook = hook_addr((uintptr_t) so_symbol(&so_mod, "_ZN8NuThread10ThreadMainEPv"),
                                                 (uintptr_t) &_ZN8NuThread10ThreadMainEPv_soloader);

#ifdef DEBUG
    NuFileOpen_hook = hook_addr((uintptr_t) so_symbol(&so_mod, "NuFileOpen"), (uintptr_t) &NuFileOpen_soloader);
#endif
}
