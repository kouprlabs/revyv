#ifndef BROWSERCLIENT_H
#define BROWSERCLIENT_H

#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_request_handler.h"
#include "include/cef_resource_request_handler.h"
#include "render_handler.h"
#include <atomic>

class BrowserClient : public CefClient,
                      public CefLifeSpanHandler,
                      public CefRequestHandler,
                      public CefResourceRequestHandler {
public:
    explicit BrowserClient(RenderHandler* renderHandler);

    CefRefPtr<CefRenderHandler> GetRenderHandler() override;

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

    CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        bool is_navigation,
        bool is_download,
        const CefString& request_initiator,
        bool& disable_default_handling) override
    {
        return this;
    }

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    bool OnCertificateError(CefRefPtr<CefBrowser> browser,
        cef_errorcode_t cert_error,
        const CefString& request_url,
        CefRefPtr<CefSSLInfo> ssl_info,
        CefRefPtr<CefCallback> callback) override;

    CefResourceRequestHandler::ReturnValue OnBeforeResourceLoad(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        CefRefPtr<CefCallback> callback) override;

private:
    CefRefPtr<CefRenderHandler> _render_handler;
    std::atomic<int> _browser_count { 0 };

    IMPLEMENT_REFCOUNTING(BrowserClient);
};

#endif
