#ifndef BROWSERCLIENT_H
#define BROWSERCLIENT_H

#include "include/cef_client.h"
#include "render_handler.h"
#include "include/cef_life_span_handler.h"
#include <atomic>

class BrowserClient : public CefClient, public CefLifeSpanHandler {
public:
    explicit BrowserClient(RenderHandler* renderHandler);

    CefRefPtr<CefRenderHandler> GetRenderHandler() override;

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

private:
    CefRefPtr<CefRenderHandler> _render_handler;
    std::atomic<int> _browser_count { 0 };

    IMPLEMENT_REFCOUNTING(BrowserClient);
};

#endif
