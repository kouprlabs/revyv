#ifndef EVENTTHREAD_H
#define EVENTTHREAD_H

#include "include/cef_browser.h"
#include <cstdint>

class EventThread {
public:
    static void event_thread(CefBrowser* browser, void* aslCtx);

private:
    static int get_chromium_keyboard_code(int32_t scancode);

    static uint32_t get_chromium_key_modifiers(int32_t keymod);
};

#endif
