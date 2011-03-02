#pragma once
#ifndef _EDITORAPP_H_
#define _EDITORAPP_H_

#include "Application.h"
#include "Editor.h"

class EditorApp : public Application
{
public:
	EditorApp();
	void LoadGraphics();
	void UnloadGraphics();
	void Render();
	void Update(float dt);
	bool HandleMessage(MSG msg);

private:
	Editor _editor;
};

#endif