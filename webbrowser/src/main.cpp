#include "browser_client.h"
#include "event_thread.h"
#include "include/cef_app.h"
#include "include/cef_render_handler.h"
#include "render_handler.h"
#include <argh.h>
#include <filesystem>
#include <iostream>
#include <system_error>
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
#if defined(OS_MAC) || defined(__APPLE__)
    // Helper processes require a properly staged .app bundle when the sandbox
    // is enabled.  The development build currently runs out of the build tree
    // without the full helper bundle layout, so disable the CEF sandbox on
    // macOS to allow the renderer and GPU subprocesses to launch.
    settings.no_sandbox = true;
#endif

    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path exe_path = fs::absolute(argv[0]);
    fs::path canonical_path = fs::canonical(exe_path, ec);
    if (!ec) {
        exe_path = canonical_path;
    }

    fs::path resources_dir;
    fs::path locales_dir;
#if defined(OS_MAC) || defined(__APPLE__)
    const fs::path exe_dir = exe_path.parent_path();
    resources_dir = exe_dir / "Resources";
    locales_dir = resources_dir / "locales";

    fs::path framework_dir = exe_dir / ".." / "Frameworks" / "Chromium Embedded Framework.framework";
    framework_dir = fs::weakly_canonical(framework_dir, ec);
    if (ec) {
        ec.clear();
        framework_dir = fs::absolute(framework_dir);
    }
    CefString(&settings.framework_dir_path) = framework_dir.string();
    CefString(&settings.main_bundle_path) = exe_dir.string();
#ifdef CEF_NO_HELPERS
    // Recent CEF binary distributions for macOS no longer ship prebuilt helper
    // applications. When the helpers are absent CEF will still try to spawn
    // them from the default bundle locations which fails at runtime. Point the
    // browser subprocess path at the main executable instead so the existing
    // process entry point handles child process roles such as the renderer and
    // GPU helpers.
    CefString(&settings.browser_subprocess_path) = exe_path.string();
#endif
#else
    resources_dir = exe_path.parent_path();
    locales_dir = resources_dir / "locales";
#endif

    CefString(&settings.resources_dir_path) = resources_dir.string();
    CefString(&settings.locales_dir_path) = locales_dir.string();
    if (!CefInitialize(args, settings, nullptr, nullptr)) {
        return -1;
    }

    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);

    CefBrowserSettings browser_settings;
    browser_settings.webgl = STATE_ENABLED;
    browser_settings.local_storage = STATE_ENABLED;
    CefRefPtr<BrowserClient> browserClient = new BrowserClient(new RenderHandler(revyv, x, y, width, height));
    CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), url, browser_settings, nullptr, nullptr);

    std::thread thread(EventThread::event_thread, browser.get(), revyv);

    CefRunMessageLoop();
    CefShutdown();

    thread.join();

    return 0;
}
