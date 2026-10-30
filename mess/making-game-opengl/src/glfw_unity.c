#if (defined(__linux__) || defined(PLATFORM_WEB)) && (_POSIX_C_SOURCE < 199309L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L // Required for: CLOCK_MONOTONIC if compiled with c99 without gnu ext.
#endif
#if (defined(__linux__) || defined(PLATFORM_WEB)) && !defined(_GNU_SOURCE)
    #undef _GNU_SOURCE
    #define _GNU_SOURCE // Required for: ppoll if compiled with c99 without gnu ext.
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
    #define _GLFW_WIN32
#endif
#if defined(__linux__)
    #if !defined(_GLFW_WAYLAND) && !defined(_GLFW_X11)
        #error "Cannot disable Wayland and X11 at the same time"
    #endif
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    #define _GLFW_X11
#endif
#if defined(__APPLE__)
    #define _GLFW_COCOA
    #define _GLFW_USE_MENUBAR       // To create and populate the menu bar when the first window is created
    #define _GLFW_USE_RETINA        // To have windows use the full resolution of Retina displays
#endif
#if defined(__TINYC__)
    #define _WIN32_WINNT_WINXP      0x0501
#endif

// Common modules to all platforms
#include "vendors/glfw/src/init.c"
#include "vendors/glfw/src/platform.c"
#include "vendors/glfw/src/context.c"
#include "vendors/glfw/src/monitor.c"
#include "vendors/glfw/src/window.c"
#include "vendors/glfw/src/input.c"
#include "vendors/glfw/src/vulkan.c"

#if defined(_WIN32) || defined(__CYGWIN__)
    #include "vendors/glfw/src/win32_init.c"
    #include "vendors/glfw/src/win32_module.c"
    #include "vendors/glfw/src/win32_monitor.c"
    #include "vendors/glfw/src/win32_window.c"
    #include "vendors/glfw/src/win32_joystick.c"
    #include "vendors/glfw/src/win32_time.c"
    #include "vendors/glfw/src/win32_thread.c"
    #include "vendors/glfw/src/wgl_context.c"

    #include "vendors/glfw/src/egl_context.c"
    #include "vendors/glfw/src/osmesa_context.c"
#endif

#if defined(__linux__)
    #include "vendors/glfw/src/posix_module.c"
    #include "vendors/glfw/src/posix_thread.c"
    #include "vendors/glfw/src/posix_time.c"
    #include "vendors/glfw/src/posix_poll.c"
    #include "vendors/glfw/src/linux_joystick.c"
    #include "vendors/glfw/src/xkb_unicode.c"

    #include "vendors/glfw/src/egl_context.c"
    #include "vendors/glfw/src/osmesa_context.c"

    #if defined(_GLFW_WAYLAND)
        #include "vendors/glfw/src/wl_init.c"
        #include "vendors/glfw/src/wl_monitor.c"
        #include "vendors/glfw/src/wl_window.c"
    #endif
    #if defined(_GLFW_X11)
        #include "vendors/glfw/src/x11_init.c"
        #include "vendors/glfw/src/x11_monitor.c"
        #include "vendors/glfw/src/x11_window.c"
        #include "vendors/glfw/src/glx_context.c"
    #endif
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined( __NetBSD__) || defined(__DragonFly__)
    #include "vendors/glfw/src/posix_module.c"
    #include "vendors/glfw/src/posix_thread.c"
    #include "vendors/glfw/src/posix_time.c"
    #include "vendors/glfw/src/posix_poll.c"
    #include "vendors/glfw/src/xkb_unicode.c"

    #include "vendors/glfw/src/x11_init.c"
    #include "vendors/glfw/src/x11_monitor.c"
    #include "vendors/glfw/src/x11_window.c"
    #include "vendors/glfw/src/glx_context.c"

    #include "vendors/glfw/src/egl_context.c"
    #include "vendors/glfw/src/osmesa_context.c"
#endif

#if defined(__APPLE__)
    #include "vendors/glfw/src/posix_module.c"
    #include "vendors/glfw/src/posix_thread.c"
    #include "vendors/glfw/src/cocoa_init.m"
    #include "vendors/glfw/src/cocoa_joystick.m"
    #include "vendors/glfw/src/cocoa_monitor.m"
    #include "vendors/glfw/src/cocoa_window.m"
    #include "vendors/glfw/src/cocoa_time.c"
    #include "vendors/glfw/src/nsgl_context.m"

    #include "vendors/glfw/src/egl_context.c"
    #include "vendors/glfw/src/osmesa_context.c"
#endif
