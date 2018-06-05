/**
 *	ManiaPlanet RGB effects integration for the Wooting one analog keyboard
 *	Version 1.0, created by Dommy
 *	© Dominik Mrajca 2018
 *
 *	Uses the ManiaPlanet shared memory, code copied from the telemetry example.
 *	Requires wooting-rgb-control.dll in executable directory.
 *	May contain traces of nuts.
 *
 *	License summary:
 *	Do whatever you want, just credit me afterwards.
 */

#include <windows.h>
#include <iostream>
#include <math.h>

#include "maniaplanet_telemetry.h"
#include "WootingRGB.h"

using namespace std;

typedef unsigned int uint;

// Color data struct
struct Color {
	uint R, G, B;
	
	Color(uint r, uint g, uint b) {
		R = r;
		G = g;
		B = b;
	}

	SetBrightness(float brightness) {
		R = floor(R * brightness);
		G = floor(G * brightness);
		B = floor(B * brightness);
	}
};

const uint RENDER_FPS = 60;		// Nb of RGB updates in a second 

const Color COLOR_DEFAULT		(255, 255, 255);
const Color COLOR_BRAKING		(255, 0, 0);
const Color COLOR_INWATER		(0, 0, 255);
const Color COLOR_ENGINETURBO	(0, 255, 255);

const Color COLOR_CANYONCAR		(255, 15, 0);
const Color COLOR_STADIUMCAR	(0, 255, 31);
const Color COLOR_VALLEYCAR		(64, 255, 0);
const Color COLOR_LAGOONCAR		(0, 127, 255);

// Global
HANDLE hMapFile = NULL;
void* pBufView = NULL;
bool applicationRunning = true;

WootingRGB wootingRGB("wooting-rgb-control.dll"); // Wooting DLL

/**
 * Convert ManiaPlanet Bool to bool.
 *
 *	@param prop The Bool to convert.
 *	@return Converted bool value.
 */
bool B(NManiaPlanet::Bool prop) {
	return (bool*)&prop;
}

/**
 * Render the RGB effects on the keyboard.
 *
 *	@param telemetry The game's shared data.
 */
void renderEffects(NManiaPlanet::STelemetry telemetry) {
	if (!applicationRunning || telemetry.Game.State == 1 || telemetry.Race.State != 1) return;

	Color vehicleColor = COLOR_DEFAULT;
	
	// Keyboard color depends on the currently driven vehicle
	if (strstr(telemetry.Game.PlayerModel, "CanyonCar"))	vehicleColor = COLOR_CANYONCAR;
	if (strstr(telemetry.Game.PlayerModel, "StadiumCar"))	vehicleColor = COLOR_STADIUMCAR;
	if (strstr(telemetry.Game.PlayerModel, "ValleyCar"))	vehicleColor = COLOR_VALLEYCAR;
	if (strstr(telemetry.Game.PlayerModel, "LagoonCar"))	vehicleColor = COLOR_LAGOONCAR;
	
	// Engine RPM meter on FN keys
	float engineMaxRPM = 10000.;
	if (strstr(telemetry.Game.PlayerModel, "StadiumCar")) engineMaxRPM = 11000.;
	
	for (int i = 0; i < 12; i++) {
		float ratioRPM = telemetry.Vehicle.EngineRPM / engineMaxRPM * 12;
		float keyRatio = i * 1.;
		float diff = ratioRPM - keyRatio;
		
		Color color(0, 0, 0);
		
		if (diff > 0) {
			color = vehicleColor;
			if (diff <= 1) color.SetBrightness(diff);
		}
		
		wootingRGB.wooting_rgb_array_set_single(0, 2 + i, color.R, color.G, color.B);
	}
	
	// Current engine gear display on number keys (0 -> reverse -> backspace)
	int gearKeys[] = { 13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
	
	for (int i = 0; i < 13; i++) {
		Color color(0, 0, 0);
		
		if (i == telemetry.Vehicle.EngineCurGear) color = vehicleColor;

		wootingRGB.wooting_rgb_array_set_single(1, gearKeys[i], color.R, color.G, color.B);
	}
	
	// Show wheel contact indicators
	int wheelKeysY[] = { 1, 1, 2, 2 };
	int wheelKeysX[] = { 15, 16, 16, 15 };
	
	for (int i = 0; i < 4; i++) {
		Color color = vehicleColor;

		// White during drifts
		if (telemetry.Vehicle.WheelsIsSliping[i]) color = COLOR_DEFAULT;
		
		// Damper length as key brightness
		float wheelDamperLength = (telemetry.Vehicle.WheelsDamperRangeMax - telemetry.Vehicle.WheelsDamperLength[i]) / (telemetry.Vehicle.WheelsDamperRangeMax - telemetry.Vehicle.WheelsDamperRangeMin);
		if (wheelDamperLength < 0) wheelDamperLength = 0.; 
		if (wheelDamperLength > 1) wheelDamperLength = 1.;
		color.SetBrightness(wheelDamperLength);
		
		wootingRGB.wooting_rgb_array_set_single(wheelKeysY[i], wheelKeysX[i], color.R, color.G, color.B);
	}

	// Escape key: headlights
	Color headlightsColor(0, 0, 0);
	if (telemetry.Vehicle.IsLightsOn) headlightsColor = COLOR_DEFAULT;
	wootingRGB.wooting_rgb_array_set_single(0, 0, headlightsColor.R, headlightsColor.G, headlightsColor.B);
	
	// Gas pedal
	Color gasColor = vehicleColor;
	gasColor.SetBrightness(telemetry.Vehicle.InputGasPedal);
	wootingRGB.wooting_rgb_array_set_single(4, 15, gasColor.R, gasColor.G, gasColor.B);
	
	// Steer left
	Color leftColor(0, 0, 0);
	if (telemetry.Vehicle.InputSteering < 0) {
		leftColor = vehicleColor;
		leftColor.SetBrightness(-telemetry.Vehicle.InputSteering);
	}
	wootingRGB.wooting_rgb_array_set_single(5, 14, leftColor.R, leftColor.G, leftColor.B);
	
	// Steer right
	Color rightColor(0, 0, 0);
	if (telemetry.Vehicle.InputSteering > 0) {
		rightColor = vehicleColor;
		rightColor.SetBrightness(telemetry.Vehicle.InputSteering);
	}
	wootingRGB.wooting_rgb_array_set_single(5, 16, rightColor.R, rightColor.G, rightColor.B);
	
	// Brake
	Color brakeColor(0, 0, 0);
	if (telemetry.Vehicle.InputIsBraking) brakeColor = vehicleColor;
	wootingRGB.wooting_rgb_array_set_single(5, 15, brakeColor.R, brakeColor.G, brakeColor.B);
	
	// Keys matrix
	Color matrixColor(0, 0, 0);
	
	// In water
	if (telemetry.Vehicle.IsInWater) matrixColor = COLOR_INWATER;
	
	for (int x = 0; x < 14; x++)
	for (int y = 2; y < 6; y++) {
		wootingRGB.wooting_rgb_array_set_single(y, x, matrixColor.R, matrixColor.G, matrixColor.B);
	}
	
	// Turbo boost
	if (telemetry.Vehicle.EngineTurboRatio > 0) {
		for (int y = 5; y >= 2; y--) {
			float turboRatio = telemetry.Vehicle.EngineTurboRatio * 8;
			float keyRatio = (5 - y) * 1.;
			float diff = turboRatio - keyRatio;
	
			Color turboColor(0, 0, 0);
			
			if (diff > 0) {
				turboColor = COLOR_ENGINETURBO;
				if (diff <= 1) turboColor.SetBrightness(diff);
			}
	
			for (int x = 0; x < 14; x++) {
				wootingRGB.wooting_rgb_array_set_single(y, x, turboColor.R, turboColor.G, turboColor.B);
			}
		}
}
	
	// Print on keyboard
	wootingRGB.wooting_rgb_array_update_keyboard();
}

/**
 * Reset Wooting RGB.
 *
 * @return True,k if the keyboard was reset properly.
 */
bool resetWooting() {
	printf("Resetting Wooting RGB...\n");
	
	Sleep(250); // Prevents reset problems from occuring
	
	bool restored = wootingRGB.wooting_rgb_reset();
	
	if (restored) {
		printf("Wooting RGB restored!\n");
	} else {
		printf("Failed to restore Wooting RGB!\n");
	}
	
	return restored;
}

/**
 * Cleanup before closing the app
 */
void shutdown() {
	applicationRunning = false;
	
	printf("Stopping app...\n");
	
	if (pBufView) UnmapViewOfFile(pBufView);
	if (hMapFile) CloseHandle(hMapFile);
	
	resetWooting();
}

/**
 * Console closing handler.
 */
void SigBreak_Handler(int signal) {
	shutdown();
}

/**
 * Main.
 */
int main(int argc, char *argv[]) {
	// Close the app if keyboard is not detected
	if (!wootingRGB.wooting_rgb_kbd_connected()) {
		printf("Wooting one not detected, stopping the app...\n");
		return 1;
	}
	
	printf("Wooting one detected\n");
	
	signal(SIGBREAK, &SigBreak_Handler);
	
	const volatile NManiaPlanet::STelemetry* shared = NULL;
	NManiaPlanet::STelemetry telemetry;
	
	uint sleepDuration = 1000 / RENDER_FPS;
	bool isInRace = false;
	bool prevIsInRace = isInRace;
	
	// Infinite loop at desired FPS
	while (applicationRunning) {
		Sleep(sleepDuration);
		
		// Immediately stop if keyboard got disconnected
		if (!wootingRGB.wooting_rgb_kbd_connected()) {
			printf("Keyboard disconnected! Stopping the app...\n");
			shutdown();
			break;
		}
		
		isInRace = shared != NULL;
		
		// From Nadeo telemetry example: mapping file
		if (shared == NULL) {
			if (!hMapFile) {
				hMapFile = OpenFileMapping(
					FILE_MAP_ALL_ACCESS,		// read/write access
					FALSE,						// do not inherit the name
					"ManiaPlanet_Telemetry");	// name of mapping object
			}
			
			if (hMapFile) {
				if (!pBufView) {
					pBufView = (void*) MapViewOfFile(
						hMapFile,				// handle to map object
						FILE_MAP_ALL_ACCESS,	// read/write permission
						0,
						0,
						4096);
				}
			}
			
			shared = (const NManiaPlanet::STelemetry*)pBufView;
		}
		
		// Copy the contents to telemetry object
		else {
			while (true) {
				uint Before = shared->UpdateNumber;
				memcpy(&telemetry, (const NManiaPlanet::STelemetry*)shared, sizeof(telemetry));
				uint After = shared->UpdateNumber;
				if (Before == After) break;
				
				// Reading while the game is changing the values, retry...
				else continue;
			}
			
			// Check if the player is in a race
			isInRace = strcmp(telemetry.Game.PlayerModel, "Unassigned") != 0 && telemetry.Game.State != 1 && telemetry.Race.State == 1;
			
			if (isInRace) renderEffects(telemetry);
		}
		
		if (prevIsInRace != isInRace) {
			prevIsInRace = isInRace;
			
			// Reset the keyboard
			if (!isInRace) {
				printf("Race end\n");
				resetWooting();
			}
			
			// Init effects by applying black color to all keys
			else {
				printf("Race start\n");
				
				for (int x = 0; x < 21; x++)
				for (int y = 0; y < 6; y++) {
					wootingRGB.wooting_rgb_array_set_single(y, x, 0, 0, 0);
				}
				
				wootingRGB.wooting_rgb_array_update_keyboard();
			}
		}
	}
	
	shutdown();

	return 0;
}
