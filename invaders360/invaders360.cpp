#include "stdafx.h"
#include <stdio.h>
#include "cpu.h"
#include "gameScreenRenderer.h"
#include "audio.h"
#include "loader.h"

//-------------------------------------------------------------------------------------
// Time             Since fAppTime is a float, we need to keep the quadword app time 
//                  as a LARGE_INTEGER so that we don't lose precision after running
//                  for a long time.
//-------------------------------------------------------------------------------------
struct TimeInfo
{    
    LARGE_INTEGER qwTime;    
    LARGE_INTEGER qwAppTime;   

    float fAppTime;    
    float fElapsedTime;    

    float fSecsPerTick;    
};

TimeInfo g_Time;

void InitTime()
{    
    // Get the frequency of the timer
    LARGE_INTEGER qwTicksPerSec;
    QueryPerformanceFrequency( &qwTicksPerSec );
    g_Time.fSecsPerTick = 1.0f / (float)qwTicksPerSec.QuadPart;

    // Save the start time
    QueryPerformanceCounter( &g_Time.qwTime );
    
    // Zero out the elapsed and total time
    g_Time.qwAppTime.QuadPart = 0;
    g_Time.fAppTime = 0.0f; 
    g_Time.fElapsedTime = 0.0f;    
}

void UpdateTime()
{
    LARGE_INTEGER qwNewTime;
    LARGE_INTEGER qwDeltaTime;
    
    QueryPerformanceCounter( &qwNewTime );    
    qwDeltaTime.QuadPart = qwNewTime.QuadPart - g_Time.qwTime.QuadPart;

    g_Time.qwAppTime.QuadPart += qwDeltaTime.QuadPart;    
    g_Time.qwTime.QuadPart     = qwNewTime.QuadPart;
    
    g_Time.fElapsedTime      = g_Time.fSecsPerTick * ((FLOAT)(qwDeltaTime.QuadPart));
    g_Time.fAppTime          = g_Time.fSecsPerTick * ((FLOAT)(g_Time.qwAppTime.QuadPart));    
}

void __cdecl main()
{

    InitTime();

    gameScreenRenderer renderer;
    renderer.init();

    bus bus;
    cpu cpu(bus);
    audio audio;
    audio.init();

	FILE* f = fopen("game:\\invaders.rom", "rb");
	if (!f) return;

	
    size_t n = fread(bus.getMemoryPtr(0), 1, 0x4000, f);
	fclose(f);

    std::vector<SoundData> sounds = loader::LoadSoundPak("game:\\sounds.pak");
    audio.play(sounds[8]);

	const int CYCLES_HALF_FRAME = (2 * 1000 * 1000) / 60 / 2;
    for (;;)
    {
        UpdateTime();
	
        cpu.run(CYCLES_HALF_FRAME);
        cpu.interrupt(0xCF);
        cpu.run(CYCLES_HALF_FRAME);
        cpu.interrupt(0xD7);

        renderer.render(bus.getMemoryPtr(0x2400));
		XINPUT_STATE state;

		if (XInputGetState(0, &state) == ERROR_SUCCESS)
		{
			WORD buttons = state.Gamepad.wButtons;
            uint8_t mask = 0;
			if (buttons & XINPUT_GAMEPAD_Y) mask |= 1;
			if (buttons & XINPUT_GAMEPAD_B) mask |= 1 << 2;
			if (buttons & XINPUT_GAMEPAD_A) mask |= 1 << 4;
			if (buttons & XINPUT_GAMEPAD_DPAD_LEFT) mask |= 1 << 5;
			if (buttons & XINPUT_GAMEPAD_DPAD_RIGHT) mask |= 1 << 6;

            bus.setInputMask(mask);
		}
    }
}

