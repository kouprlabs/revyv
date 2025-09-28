#include "browser_client.h"
#include "event_thread.h"
#include "include/cef_app.h"
#include "include/cef_render_handler.h"
#include "render_handler.h"
#include <argh.h>
#include <iostream>
#include <revyv/revyv.h>
#include <thread>

void* revyv = nullptr;

int main(int argc, char* argv[])
{
    CefMainArgs args(argc, argv);
    argh::parser cmdl({ "-f", "--frame", "-u", "--url" });

    cmdl.parse(argc, argv);

    std::string url("https://www.google.com");
    if (!cmdl("url").str().empty()) {
        url = cmdl("url").str();
    }

    int x = 0, y = 0, width = 300, height = 300;
    if (!cmdl("frame").str().empty()) {
        std::stringstream frame(cmdl("frame").str());
        std::vector<int> values;
        while (frame.good()) {
            std::string value;
            getline(frame, value, ',');
            values.push_back(std::atoi(value.c_str()));
        }
        x = values[0];
        y = values[1];
        width = values[2];
        height = values[3];
    }

    int result = CefExecuteProcess(args, nullptr, nullptr);
    if (result >= 0) {
        // The child proccess terminated, we exit
        return result;
    }
    if (result == -1) {
        /* Parent proccess */
        revyv = revyv_context_create();
    }

    CefSettings settings;
    settings.windowless_rendering_enabled = true;
    if (!CefInitialize(args, settings, nullptr, nullptr)) {
        return -1;
    }

    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);

    CefBrowserSettings browser_settings;
    browser_settings.webgl = STATE_ENABLED;
    browser_settings.local_storage = STATE_ENABLED;
    browser_settings.databases = STATE_ENABLED;
    CefRefPtr<BrowserClient> browserClient = new BrowserClient(new RenderHandler(revyv, x, y, width, height));
    CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), url, browser_settings, nullptr, nullptr);

    std::thread thread(EventThread::event_thread, browser.get(), revyv);

    CefRunMessageLoop();
    CefShutdown();

    thread.join();

    return 0;
}
