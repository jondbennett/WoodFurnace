////////////////////////////////////////////////////////////
// Set flue temp wait time
////////////////////////////////////////////////////////////
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#include "Pins.h"
#include "Defs.h"
#include "MilliTimer.h"

#include "ScreenController.h"
#include "Screen_Setup_FlueTempWait.h"
#include "Settings.h"

extern Adafruit_RGBLCDShield g_lcd;

extern CWoodStoveSettings g_woodStoveSettings;
extern CScreenController g_screenController;

////////////////////////////////////////////////////////////
// Display normal temperature situation
////////////////////////////////////////////////////////////
CScreen_Setup_FlueTempWait::CScreen_Setup_FlueTempWait(int _id) : CScreen_Base(_id)
{

}

CScreen_Setup_FlueTempWait::~CScreen_Setup_FlueTempWait()
{

}

void CScreen_Setup_FlueTempWait::init()
{
#ifdef DEBUG_SCREEN_SETUP_FLUE_TEMP
	printUptime();
	Serial.println(F("CScreen_Setup_FlueTempWait::init()"));
#endif
	g_lcd.clear();
	g_lcd.cursor();
	g_lcd.noBlink();

	updateStatics();
	updateDynamics();
}

void CScreen_Setup_FlueTempWait::updateStatics()
{
#ifdef DEBUG_SCREEN_SETUP_FLUE_TEMP
	printUptime();
	Serial.println(F("CScreen_Setup_FlueTempWait::updateStatics()"));
#endif

	g_lcd.setCursor(0, 0);
	g_lcd.print(F("Setup:Flue Wait"));
	g_lcd.setCursor(0, 1);
	g_lcd.print(F("Seconds:"));
}

void CScreen_Setup_FlueTempWait::updateDynamics()
{
#ifdef DEBUG_SCREEN_SETUP_FLUE_TEMP
	printUptime();
	Serial.println(F("CScreen_Setup_FlueTempWait::updateDynamics()"));
#endif

	// Print the spaces because it could be a two or three digit value
	g_lcd.setCursor(8, 1);
	g_lcd.print(F("   "));
	g_lcd.setCursor(8, 1);
	g_lcd.print(g_settings.m_flueTempWaitTime);

	if(g_settings.m_flueTempWaitTime < 100)
		g_lcd.setCursor(9, 1);
	else
		g_lcd.setCursor(10, 1);
}

void CScreen_Setup_FlueTempWait::buttonCheck(CButtonController &_buttons)
{
	// Don't let it get away and change too fast, but
	// let the operator press buttons as fast as he wants
	if(!_buttons.anyButtonsPressed())
		m_buttonTimer.reset();

	if(m_buttonTimer.getState() == CMilliTimer::running)
		return;

	// Are we done here?
	if(_buttons.getButton(BC_BUTTON_SELECT) > 0)
	{
		g_settings.saveSettings();
		g_screenController.setScreen(SCREEN_ID_SETUP_FAN_TEMPS);
		return;
	}

	// Increase the wait time?
	unsigned long buttonTime;
	bool updateDisplay = false;
	buttonTime = _buttons.getButton(BC_BUTTON_UP);
	if(buttonTime > 0)
	{
		if(buttonTime < SETUP_TIME_BUMP)
			g_settings.m_flueTempWaitTime++;
		else
			g_settings.m_flueTempWaitTime += 10;

		m_buttonTimer.start(SETUP_TIME_BUMP);
		updateDisplay = true;
	}

	// Decrease the wait time?
	buttonTime = _buttons.getButton(BC_BUTTON_DOWN);
	if(buttonTime > 0)
	{
		if(buttonTime < SETUP_TIME_BUMP)
			g_settings.m_flueTempWaitTime--;
		else
			g_settings.m_flueTempWaitTime -= 10;

		m_buttonTimer.start(SETUP_TIME_BUMP);
		updateDisplay = true;
	}

	// Keep them within bounds
	g_settings.m_flueTempWaitTime = constrain(g_settings.m_flueTempWaitTime, MIN_FLUE_TEMP_WAIT_TIME, MAX_FLUE_TEMP_WAIT_TIME);

	if(updateDisplay)
		updateDynamics();
}

void CScreen_Setup_FlueTempWait::processOneSecond()
{
//	updateDynamics();
}
