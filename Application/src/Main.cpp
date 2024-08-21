#include "Application.hpp"

// Creates and starts the application
int main()
{
    Application app{};
    app.run();

    return 0;
}

#ifdef MTD_APP_WINDOW_ONLY
    #include <Windows.h>

    // Entry point for window only applications (no terminal), on Windows
    int WINAPI WinMain
    (
        _In_ HINSTANCE hInstance,
        _In_opt_ HINSTANCE hPrevInstance,
        _In_ LPSTR lpCmdLine,
        _In_ int nCmdShow
    )
    {
        return main();
    }
#endif
