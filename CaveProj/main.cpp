#include "EditorApp.h"

#include <crtdbg.h>

int main (int argc, char** argv)
{
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	EditorApp app;
	return app.Run();
}