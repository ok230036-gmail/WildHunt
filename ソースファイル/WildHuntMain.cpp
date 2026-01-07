#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             //  Exclude rarely-used stuff from Windows headers.
#endif

#include <Windows.h>

#include "WildHuntScene.h"
#include "MyAppRunner.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{

    MyGameEngine* engine = new MyGameEngine(960, 540, L"WildHunt");
    engine->SetSceneController(new WildHuntScene());

    int res = MyAppRunner::Run(engine, hInstance, nCmdShow);

    delete(engine);

    return res;
}