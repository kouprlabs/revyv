#ifndef BROWSER_APP_H
#define BROWSER_APP_H

#include "include/cef_app.h"

class BrowserApp : public CefApp {
public:
    BrowserApp() = default;

    void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

private:
    IMPLEMENT_REFCOUNTING(BrowserApp);
};

#endif
