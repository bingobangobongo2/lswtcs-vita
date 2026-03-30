/*
 * netdbg.h — Network debug logging for Vita so-loader ports
 *
 * Header-only wrapper around debugnet. Include in logger.c, call
 * netdbg_init() from main.c before soloader_init_all().
 *
 * Enable via CMake: -DNETDBG_IP="192.168.1.100"
 * Listen on PC:     socat udp-recv:18194 stdout
 */

#ifndef NETDBG_H
#define NETDBG_H

#ifdef NETDBG_ENABLED

#include <debugnet.h>

/* debugnet defines bare NONE/INFO/ERROR/DEBUG macros — undef to avoid
 * collisions with soloader logger (which uses LT_DEBUG etc.) */
#undef NONE
#undef INFO
#undef ERROR
#undef DEBUG

#ifndef NETDBG_PORT
#define NETDBG_PORT 18194
#endif

/* Shared across all translation units — declared extern, defined in the
   compilation unit that calls netdbg_init() (use NETDBG_DEFINE in exactly
   one .c file before including this header, or link with -D). */
#ifdef NETDBG_DEFINE
int _netdbg_active = 0;
#else
extern int _netdbg_active;
#endif

static inline void netdbg_init(void) {
    int ret = debugNetInit(NETDBG_IP, NETDBG_PORT, 3 /* DEBUG */);
    if (ret >= 0) _netdbg_active = 1;
}

static inline void netdbg_send(const char *text) {
    if (_netdbg_active) debugNetUDPSend(text);
}

static inline void netdbg_finish(void) {
    if (_netdbg_active) { debugNetFinish(); _netdbg_active = 0; }
}

#else /* NETDBG_ENABLED not defined — all calls compile to nothing */

static inline void netdbg_init(void) {}
static inline void netdbg_send(const char *text) { (void)text; }
static inline void netdbg_finish(void) {}

#endif /* NETDBG_ENABLED */

#endif /* NETDBG_H */
