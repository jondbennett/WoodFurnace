#include <Arduino.h>

#include <PID_v1.h>
#include <SaveController.h>

#include "Pins.h"
#include "Defs.h"
#include "Settings.h"
#include "MilliTimer.h"

//////////////////////////////////////////////////////
// The thermocouple
#include "TempSensor_Thermocouple.h"
CTempSensor_Thermocouple g_thermocouple;

//////////////////////////////////////////////////////
// LCD Display
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
Adafruit_RGBLCDShield g_lcd = Adafruit_RGBLCDShield();

// Special degree symbol - envision it as a 5 wide by 7 high bitmap
static unsigned char degreeSymbolData[8] = {12, 18, 18, 12, 0, 0, 0};
const char *degreeSymbol = "\1\0";

//////////////////////////////////////////////////////
// Screen Controller
#include "ScreenController.h"
#include "Screen_Normal.h"
#include "Screen_Setup_FlueTemp.h"
#include "Screen_Setup_FlueTempWait.h"
#include "Screen_Setup_Fan.h"
#include "Screen_Setup_MIdle.h"
#include "Screen_Setup_PID.h"

CScreenController g_screenController;

//////////////////////////////////////////////////////
// Temperature Controller
#include "WSPID.h"
#include "TempController.h"
CTempController g_tempController;

//////////////////////////////////////////////////////
// Fan Controller
#include "FanController.h"
CFanController g_fanController;

//////////////////////////////////////////////////////
// PWM Motor
#include "PWMMotor.h"
CPWMMotor g_forcedDraftMotor;

//////////////////////////////////////////////////////
// Settings management
CWoodStoveSettings g_settings;

//////////////////////////////////////////////////////
// Beeper for low fuel and alarm
#include "Beeper.h"
CBeeper g_beeper;

//////////////////////////////////////////////////////
// Loop Process Timing
static unsigned long s_previousMillis = 0L;

static unsigned long s_systemSeconds = 0L;
void printUptime(bool _colonSpace)
{
	Serial.print(s_systemSeconds);
	if(_colonSpace)
		Serial.print(F(": "));
}

//////////////////////////////////////////////////////
// Simulator for testing
#ifdef SIMULATION_MODE
#include "StoveSim.h"
CStoveSim g_stoveSim;
#endif

//////////////////////////////////////////////////////
// Arduino setup function (only called once at startup)
void setup()
{
	// ----------------------------------------
	// Prep serial port for debugging output
	Serial.begin(9600);
	Serial.println();
	Serial.println(F("===== Wood Stove Controller Starting ====="));

	// ----------------------------------------
	// Load settings
	g_settings.loadSettings();

	// ----------------------------------------
	// Prep the beeper
	g_beeper.setup();

	// ----------------------------------------
	// Prep the thermocouple
	g_thermocouple.init();

	// ----------------------------------------
	// Prep the forced air controller
	g_fanController.setup();

	// ----------------------------------------
	// Prep the forced draft motor
	g_forcedDraftMotor.setup();

	// ----------------------------------------
	// Prep the temperature controller
	g_tempController.setup();

	// ----------------------------------------
	// Prep the LCD
	g_lcd.begin(16, 2);
	g_lcd.setBacklight(0x07);
	g_lcd.createChar(1, degreeSymbolData);

	// ----------------------------------------
	// Prep the screens
	{
		g_screenController.setup();

		static CScreen_Normal screenNormal(SCREEN_ID_NORMAL);
		static CScreen_Setup_FlueTemp screenSetupFlueTemp(SCREEN_ID_SETUP_FLUE_TEMP);
		static CScreen_Setup_FlueTempWait screenSetupFlueTempWait(SCREEN_ID_SETUP_FLUE_TEMP_WAIT);
		static CScreen_Setup_Fan screenSetupFanTemps(SCREEN_ID_SETUP_FAN_TEMPS);
		static CScreen_Setup_MIdle screenSetupManualIdle(SCREEN_ID_SETUP_MIDLE);
		static CScreen_Setup_PID screenSetupPID(SCREEN_ID_SETUP_PID);

		g_screenController.addScreen(&screenNormal);
		g_screenController.addScreen(&screenSetupFlueTemp);
		g_screenController.addScreen(&screenSetupFlueTempWait);
		g_screenController.addScreen(&screenSetupFanTemps);
		g_screenController.addScreen(&screenSetupManualIdle);
		g_screenController.addScreen(&screenSetupPID);
	}
}

//////////////////////////////////////////////////////
// Arduino loop function called over and over forever
void loop()
{
	// ----------------------------------------
	// Set initial screen. From there it's up to
	// the screen objects
	if(s_previousMillis == 0L)
	{
		s_previousMillis = millis();
		g_screenController.setScreen(SCREEN_ID_NORMAL);

#ifdef SERIAL_PLOT
		Serial.print(F("Call, Target, Actual, PWM"));
#ifdef SIMULATION_MODE
		Serial.print(F(", Fuel"));
#endif
		Serial.println();
#endif
	}

	// ----------------------------------------
	// Fast Processing
	g_screenController.processFast();
	g_forcedDraftMotor.processFast();
	g_tempController.processFast();
	g_beeper.processFast();

	// ----------------------------------------
	// One-second processing
	unsigned long currentMillis = millis();
	if( currentMillis - s_previousMillis >= ONE_SECOND_MS )
	{
		// Time handling
		s_previousMillis = currentMillis;
		s_systemSeconds++;

		// ----------------------------------------
		// Run the stove simulator
#ifdef SIMULATION_MODE
		g_stoveSim.processOneSecond();
#endif

		// ----------------------------------------
		// Update the display
		g_screenController.processOneSecond();

		// ----------------------------------------
		// Run the fan controller
		g_fanController.processOneSecond();

		// ----------------------------------------
		// Run the temperature controller state machine
		g_tempController.processOneSecond();

#ifdef DEBUG_INO
		Serial.print(F("One-second processing took: "));
		Serial.println(millis() - currentMillis);
#endif
	}
}

//////////////////////////////////////////////////////
// Handle special cases where
void pidSettingsChanged()
{
//	g_settings.saveSettings();
	g_tempController.updateSettings();
}
