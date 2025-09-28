#include "include/cef_app.h"
#include "include/wrapper/cef_library_loader.h"

int main(int argc, char* argv[])
{
    CefMainArgs main_args(argc, argv);
#if defined(__APPLE__)
    CefScopedLibraryLoader library_loader;
    if (!library_loader.LoadInHelper()) {
        return -1;
    }
#endif
    return CefExecuteProcess(main_args, nullptr, nullptr);
}
