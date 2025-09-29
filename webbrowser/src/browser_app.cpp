#include "browser_app.h"

#include "include/base/cef_build.h"

void BrowserApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
#if defined(OS_MACOSX)
    if (process_type.empty()) {
        command_line->AppendSwitch("use-mock-keychain");
    }
#endif
}
