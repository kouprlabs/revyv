#include "browser_client.h"
#include "event_thread.h"
#include "include/cef_app.h"
#include "include/cef_render_handler.h"
#include "render_handler.h"
#include <argh.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>
#include <revyv/revyv.h>
#include <thread>

#if defined(OS_MAC) || defined(__APPLE__)
#include <Security/Security.h>
#include <array>
#endif

#if defined(OS_MAC) || defined(__APPLE__)
namespace {

void RestrictKeychainSearchListToSystemStores()
{
    CFArrayRef original_list = nullptr;
    if (SecKeychainCopySearchList(&original_list) != errSecSuccess || !original_list)
        return;

    CFMutableArrayRef filtered = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if (!filtered) {
        CFRelease(original_list);
        return;
    }

    const std::array allowed_names = {
        CFSTR("System.keychain"),
        CFSTR("SystemRootCertificates.keychain"),
        CFSTR("SystemRootCertificates.keychain-db"),
        CFSTR("System.keychain-db"),
    };

    bool has_system_keychain = false;
    bool has_root_keychain = false;

    const CFIndex count = CFArrayGetCount(original_list);
    for (CFIndex index = 0; index < count; ++index) {
        auto keychain_ref = static_cast<SecKeychainRef>(const_cast<void*>(CFArrayGetValueAtIndex(original_list, index)));
        if (!keychain_ref)
            continue;

        CFStringRef name = nullptr;
        if (SecKeychainCopyName(keychain_ref, &name) == errSecSuccess && name) {
            for (CFStringRef allowed : allowed_names) {
                if (CFStringCompare(name, allowed, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                    CFArrayAppendValue(filtered, keychain_ref);
                    if (!has_system_keychain && CFStringCompare(allowed, CFSTR("System.keychain"), 0) == kCFCompareEqualTo)
                        has_system_keychain = true;
                    if (!has_root_keychain &&
                        (CFStringCompare(allowed, CFSTR("SystemRootCertificates.keychain"), 0) == kCFCompareEqualTo ||
                            CFStringCompare(allowed, CFSTR("SystemRootCertificates.keychain-db"), 0) == kCFCompareEqualTo)) {
                        has_root_keychain = true;
                    }
                    break;
                }
            }
        }

        if (name)
            CFRelease(name);
    }

    auto append_explicit_keychain = [&](const char* path, bool& flag) {
        if (flag)
            return;
        SecKeychainRef keychain = nullptr;
        if (SecKeychainOpen(path, &keychain) == errSecSuccess && keychain) {
            CFArrayAppendValue(filtered, keychain);
            CFRelease(keychain);
            flag = true;
        }
    };

    append_explicit_keychain("/System/Library/Keychains/System.keychain", has_system_keychain);
    append_explicit_keychain("/System/Library/Keychains/SystemRootCertificates.keychain", has_root_keychain);
    append_explicit_keychain("/System/Library/Keychains/SystemRootCertificates.keychain-db", has_root_keychain);

    if (CFArrayGetCount(filtered) > 0)
        SecKeychainSetSearchList(filtered);

    CFRelease(filtered);
    CFRelease(original_list);
}

} // namespace
#endif

class WebBrowserApp : public CefApp {
public:
    void OnBeforeCommandLineProcessing(const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) override
    {
        if (!command_line)
            return;

        auto append_switch = [&command_line](const std::string& name) {
            if (!command_line->HasSwitch(name))
                command_line->AppendSwitch(name);
        };

        auto append_switch_with_value = [&command_line](const std::string& name, const std::string& value) {
            std::string current = command_line->GetSwitchValue(name);
            auto contains_value = [](const std::string& list, const std::string& token) {
                size_t begin = 0;
                while (begin <= list.length()) {
                    const size_t end = list.find(',', begin);
                    const std::string_view entry(list.data() + begin,
                        (end == std::string::npos ? list.length() : end) - begin);
                    if (entry == token)
                        return true;
                    if (end == std::string::npos)
                        break;
                    begin = end + 1;
                }
                return false;
            };

            if (current.empty()) {
                command_line->AppendSwitchWithValue(name, value);
            } else if (!contains_value(current, value)) {
                current.append(",");
                current.append(value);
                command_line->AppendSwitchWithValue(name, current);
            }
        };

        append_switch("ignore-certificate-errors");
        append_switch("allow-insecure-localhost");
        append_switch("allow-running-insecure-content");
        append_switch("test-type");
        append_switch("use-chrome-root-store");

#if defined(OS_MAC) || defined(__APPLE__)
        // Recent macOS releases ship a trust store that can return malformed
        // certificate metadata for some well known sites ("GeneralNames is a
        // sequence of 0 elements").  Instruct Chromium to use a lightweight
        // in-memory keychain implementation instead of the system keychain so
        // those bogus entries never reach the network stack.
        append_switch("use-mock-keychain");
#endif

        constexpr char kDisabledSecurityFeatures[] =
            "CertificateTransparencyComponentUpdater,ExpectCTReporting";
        append_switch_with_value("disable-features", kDisabledSecurityFeatures);
    }

    IMPLEMENT_REFCOUNTING(WebBrowserApp);
};

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

    CefRefPtr<WebBrowserApp> app(new WebBrowserApp());

    int result = CefExecuteProcess(args, app.get(), nullptr);
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
    // macOS 15 currently triggers bogus certificate parsing failures for some
    // CEF requests (for example "GeneralNames is a sequence of 0 elements").
    // Certificate errors are handled by BrowserClient::OnCertificateError so we
    // can keep loading pages until Apple resolves the trust store regression.
    RestrictKeychainSearchListToSystemStores();
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

    fs::path cache_root = exe_path.parent_path() / "cef_cache";
    fs::create_directories(cache_root, ec);
    if (ec) {
        ec.clear();
    }
    CefString(&settings.root_cache_path) = cache_root.string();
    if (!CefInitialize(args, settings, app.get(), nullptr)) {
        return -1;
    }

    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);

    CefBrowserSettings browser_settings;
    browser_settings.webgl = STATE_ENABLED;
    browser_settings.local_storage = STATE_ENABLED;
    CefRefPtr<BrowserClient> browserClient = new BrowserClient(new RenderHandler(revyv, x, y, width, height));
    CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), url, browser_settings, nullptr, nullptr);

    std::thread thread(EventThread::event_thread, browser, revyv);

    CefRunMessageLoop();
    CefShutdown();

    thread.join();

    return 0;
}
