#pragma once

#include <cstdint>

//Takes care of wrapping SDL functionality

class AppWindow
{
public:

	AppWindow(const char* WindowTitle, int WindowWidth, int WindowHeight, int TextureWidth, int TextureHeight);

	~AppWindow();

	void Render(void const* Buffer, int Pitch);

	bool ProcessInput(uint8_t* Keys);

private:
	
	struct SDL_Window* Window;
	struct SDL_Renderer* Renderer;
	struct SDL_Texture* Texture;

};

