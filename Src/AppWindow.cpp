#include "AppWindow.h"
#include <SDL.h>
#include <iostream>

AppWindow::AppWindow(const char* WindowTitle, int WindowWidth, int WindowHeight, int TextureWidth, int TextureHeight)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL error\n";
	}

	Window = SDL_CreateWindow(WindowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowWidth, WindowHeight, SDL_WINDOW_SHOWN);

	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);

	Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, TextureWidth, TextureHeight);
}

AppWindow::~AppWindow()
{
	SDL_DestroyTexture(Texture);
	SDL_DestroyRenderer(Renderer);
	SDL_DestroyWindow(Window);

	SDL_Quit();
}

void AppWindow::Render(void const* Buffer, int Pitch)
{
	SDL_UpdateTexture(Texture, nullptr, Buffer, Pitch);
	SDL_RenderClear(Renderer);
	SDL_RenderCopy(Renderer, Texture, nullptr, nullptr);
	SDL_RenderPresent(Renderer);
}

bool AppWindow::ProcessInput(uint8_t* Keys)
{
	bool Quit = false;

	SDL_Event Event;

	while (SDL_PollEvent(&Event))
	{
		switch (Event.type)
		{
			case SDL_QUIT:
			{
				Quit = true;
				break;
			}

			case SDL_KEYDOWN:
			{
				switch (Event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
					{
						Quit = true;
						break;
					}
					case SDLK_x:
					{
						Keys[0] = 1;
						break;
					}
					case SDLK_1:
					{
						Keys[1] = 1;
						break;
					}
					case SDLK_2:
					{
						Keys[2] = 1;
						break;
					}
					case SDLK_3:
					{
						Keys[3] = 1;
						break;
					}
					case SDLK_q:
					{
						Keys[4] = 1;
						break;
					}
					case SDLK_w:
					{
						Keys[5] = 1;
						break;
					}
					case SDLK_e:
					{
						Keys[6] = 1;
						break;
					}
					case SDLK_a:
					{
						Keys[7] = 1;
						break;
					}
					case SDLK_s:
					{
						Keys[8] = 1;
						break;
					}
					case SDLK_d:
					{
						Keys[9] = 1;
						break;
					}
					case SDLK_z:
					{
						Keys[0xA] = 1;
						break;
					}
					case SDLK_c:
					{
						Keys[0xB] = 1;
						break;
					}
					case SDLK_4:
					{
						Keys[0xC] = 1;
						break;
					}
					case SDLK_r:
					{
						Keys[0xD] = 1;
						break;
					}
					case SDLK_f:
					{
						Keys[0xE] = 1;
						break;
					}
					case SDLK_v:
					{
						Keys[0xF] = 1;
						break;
					}
				}
				break;
			}

			case SDL_KEYUP:
			{
				switch (Event.key.keysym.sym)
				{
					case SDLK_x:
					{
						Keys[0] = 0;
						break;
					}
					case SDLK_1:
					{
						Keys[1] = 0;
						break;
					}
					case SDLK_2:
					{
						Keys[2] = 0;
						break;
					}
					case SDLK_3:
					{
						Keys[3] = 0;
						break;
					}
					case SDLK_q:
					{
						Keys[4] = 0;
						break;
					}
					case SDLK_w:
					{
						Keys[5] = 0;
						break;
					}
					case SDLK_e:
					{
						Keys[6] = 0;
						break;
					}
					case SDLK_a:
					{
						Keys[7] = 0;
						break;
					}
					case SDLK_s:
					{
						Keys[8] = 0;
						break;
					}
					case SDLK_d:
					{
						Keys[9] = 0;
						break;
					}
					case SDLK_z:
					{
						Keys[0xA] = 0;
						break;
					}
					case SDLK_c:
					{
						Keys[0xB] = 0;
						break;
					}
					case SDLK_4:
					{
						Keys[0xC] = 0;
						break;
					}
					case SDLK_r:
					{
						Keys[0xD] = 0;
						break;
					}
					case SDLK_f:
					{
						Keys[0xE] = 0;
						break;
					}
					case SDLK_v:
					{
						Keys[0xF] = 0;
						break;
					}
				}
				break;
			}
		}
	}

	return Quit;
}

