// Chip8-Emu.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include "AppWindow.h"
#include "Emulator.h"

int main(int argc, char* args[])
{

    //If no argument was passed when starting the program, then stop execution immediately
    if (argc < 2)
    {
        std::cout << "Specify a path to the rom you want to play as argument!";
        return -1;
    }


    //Creates the actual Chip-8 Emulator/Interpreter
    Emulator* Chip8 = new Emulator;

    //Loads the ROM specified as argument
    Chip8->LoadRom(args[1]);

    const int CycleDelay = 3;
    int Pitch = sizeof(Chip8->ScreenPixels[0]) * 64;

    //Creates the actual window 
    AppWindow* Window = new AppWindow(Chip8->GetRomName().c_str(), 64 * 10, 32 * 10, 64, 32);

    auto LastCycleTime = std::chrono::high_resolution_clock::now();
    bool Quit = false;
    
    //Main loop
    while (!Quit)
    {
        Quit = Window->ProcessInput(Chip8->Keymap);

        auto CurrentTime = std::chrono::high_resolution_clock::now();

        float DeltaTime = std::chrono::duration<float, std::chrono::milliseconds::period>(CurrentTime - LastCycleTime).count();

        if (DeltaTime > CycleDelay)
        {
            LastCycleTime = CurrentTime;

            Chip8->ExecuteCycle();

            Window->Render(Chip8->ScreenPixels, Pitch);
        }
    }

    delete Chip8;
    delete Window;

    return 0;
}