////////////////////////////////////////////////////////////
// Manual Idle
////////////////////////////////////////////////////////////
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <PID_v1.h>

#include "Pins.h"
#include "Defs.h"

#include "MilliTimer.h"
#include "WSPID.h"
#include "TempController.h"
#include "TempSensor_Thermocouple.h"

#include "ScreenController.h"
#include "Screen_Setup_MIdle.h"


extern Adafruit_RGBLCDShield g_lcd;
extern CScreenController g_screenController;
extern CTempController g_tempController;
extern CTempSensor_Thermocouple g_thermocouple;

////////////////////////////////////////////////////////////
// Setup forced draft fixed (non-PID) in idle
////////////////////////////////////////////////////////////
CScreen_Setup_MIdle::CScreen_Setup_MIdle(int _id) : CScreen_Base(_id)
{

}

CScreen_Setup_MIdle::~CScreen_Setup_MIdle()
{

}

void CScreen_Setup_MIdle::init()
{
#ifdef DEBUG_SCREEN_MIDLE
	Serial.println(F("CScreen_Setup_MIdle::init()"));
#endif
	updateStatics();
	updateDynamics();
	updateValue();
}

void CScreen_Setup_MIdle::updateStatics()
{
#ifdef DEBUG_SCREEN_MIDLE
	Serial.println(F("CScreen_Setup_MIdle::updateStatics()"));
#endif

	g_lcd.clear();
	g_lcd.setCursor(0, 0);
	g_lcd.print(F("Setup:Idle"));
	g_lcd.setCursor(0,1);
	g_lcd.print(F("Manual:"));
}

void CScreen_Setup_MIdle::updateDynamics()
{
#ifdef DEBUG_SCREEN_MIDLE
	Serial.println(F("CScreen_Setup_MIdle::updateDynamics()"));
#endif
	g_lcd.setCursor(13, 1);
	g_lcd.print(F("   "));
	g_lcd.setCursor(13, 1);
	g_lcd.print(g_thermocouple.temperature());

	// Now put the cursor on the value
	int idleSpeedOverride = g_tempController.getIdleSpeedOverride();
	if(idleSpeedOverride < 10)
		g_lcd.setCursor(7,1);
	else if(idleSpeedOverride < 100)
		g_lcd.setCursor(8,1);
	else
		g_lcd.setCursor(9,1);
}

void CScreen_Setup_MIdle::buttonCheck(CButtonController &_buttons)
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
		g_screenController.setScreen(SCREEN_ID_SETUP_PID);
		return;
	}

	// Increase?
	int idleSpeedOverride = g_tempController.getIdleSpeedOverride();

	unsigned long buttonTime;
	bool updateDisplay = false;
	buttonTime = _buttons.getButton(BC_BUTTON_UP);
	if(buttonTime > 0)
	{
		if(buttonTime < SETUP_TIME_BUMP)
			idleSpeedOverride++;
		else
			idleSpeedOverride += 10;

		m_buttonTimer.start(SETUP_TIME_BUMP);
		updateDisplay = true;
	}

	// Decrease?
	buttonTime = _buttons.getButton(BC_BUTTON_DOWN);
	if(buttonTime > 0)
	{
		if(buttonTime < SETUP_TIME_BUMP)
			idleSpeedOverride--;
		else
			idleSpeedOverride -= 10;

		m_buttonTimer.start(SETUP_TIME_BUMP);
		updateDisplay = true;
	}

	// Keep them within bounds
	idleSpeedOverride = constrain(idleSpeedOverride, 0, 100);

	g_tempController.setIdleSpeedOverride(idleSpeedOverride);

	if(updateDisplay)
		updateValue();
}

void CScreen_Setup_MIdle::updateValue()
{
	int idleSpeedOverride = g_tempController.getIdleSpeedOverride();
	g_lcd.setCursor(7,1);
	g_lcd.print(F("   "));
	g_lcd.setCursor(7,1);
	g_lcd.print(idleSpeedOverride);

	if(idleSpeedOverride < 10)
		g_lcd.setCursor(7,1);
	else if(idleSpeedOverride < 100)
		g_lcd.setCursor(8,1);
	else
		g_lcd.setCursor(9,1);
}

void CScreen_Setup_MIdle::processOneSecond()
{
	updateDynamics();
}
