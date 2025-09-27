#include "browser_client.h"
#include "include/cef_app.h"

BrowserClient::BrowserClient(RenderHandler* renderHandler)
    : _render_handler(renderHandler)
{
}

CefRefPtr<CefRenderHandler> BrowserClient::GetRenderHandler()
{
    return _render_handler;
}

void BrowserClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    ++_browser_count;
    CefLifeSpanHandler::OnAfterCreated(browser);
}

void BrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    if (--_browser_count == 0) {
        CefQuitMessageLoop();
    }
    CefLifeSpanHandler::OnBeforeClose(browser);
}

bool BrowserClient::OnCertificateError(CefRefPtr<CefBrowser> browser,
    cef_errorcode_t cert_error,
    const CefString& request_url,
    CefRefPtr<CefSSLInfo> ssl_info,
    CefRefPtr<CefCallback> callback)
{
    if (callback.get()) {
        callback->Continue(true);
        return true;
    }

    return false;
}
