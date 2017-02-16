////////////////////////////////////////////////////////////
// Fan Screen
////////////////////////////////////////////////////////////
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#include "Pins.h"
#include "Defs.h"
#include "MilliTimer.h"

#include "ScreenController.h"
#include "Screen_Setup_Fan.h"
#include "Settings.h"

extern Adafruit_RGBLCDShield g_lcd;
extern const char *degreeSymbol;

extern CWoodStoveSettings g_woodStoveSettings;
extern CScreenController g_screenController;

////////////////////////////////////////////////////////////
// Setup the circulation fan controller
////////////////////////////////////////////////////////////
CScreen_Setup_Fan::CScreen_Setup_Fan(int _id) : CScreen_Base(_id)
{

}

CScreen_Setup_Fan::~CScreen_Setup_Fan()
{

}

void CScreen_Setup_Fan::init()
{
#ifdef DEBUG_SCREEN_SETUP_FAN
	printUptime();
	Serial.println(F("CScreen_Setup_Fan::init()"));
#endif
	g_lcd.clear();
	g_lcd.cursor();
	g_lcd.noBlink();

	m_field = field_fan_on_temp;
	updateStatics();
	updateDynamics();
}

void CScreen_Setup_Fan::updateStatics()
{
#ifdef DEBUG_SCREEN_SETUP_FAN
	printUptime();
	Serial.println(F("CScreen_Setup_Fan::updateStatics()"));
#endif

	g_lcd.setCursor(0, 0);
	g_lcd.print(F("Setup:Fan"));

	g_lcd.setCursor(0, 1);
	g_lcd.print(F("On:"));

	g_lcd.setCursor(8, 1);
	g_lcd.print(F("Off:"));
}

void CScreen_Setup_Fan::updateDynamics()
{
#ifdef DEBUG_SCREEN_SETUP_FAN
	printUptime();
	Serial.println(F("CScreen_Setup_Fan::updateDynamics()"));
#endif
	// Fan on temp
	g_lcd.setCursor(3, 1);
	g_lcd.print(g_settings.m_fanOnTemp);
	g_lcd.print(degreeSymbol);

	g_lcd.setCursor(12, 1);
	g_lcd.print(g_settings.m_fanOffTemp);
	g_lcd.print(degreeSymbol);

	if(m_field == field_fan_on_temp)
		g_lcd.setCursor(5, 1);

	if(m_field == field_fan_off_temp)
		g_lcd.setCursor(14, 1);
}

void CScreen_Setup_Fan::buttonCheck(CButtonController &_buttons)
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
		g_screenController.setScreen(SCREEN_ID_SETUP_MIDLE);
		return;
	}

	// Switch fields
	if( (_buttons.getButton(BC_BUTTON_LEFT) > 0) ||
		(_buttons.getButton(BC_BUTTON_RIGHT) > 0) )
	{
#ifdef DEBUG_SCREEN_SETUP_FAN
		printUptime();
		Serial.println(F("CScreen_Setup_Fan::buttonCheck - switching fields"));
#endif

		if(m_field == field_fan_on_temp)
			m_field = field_fan_off_temp;
		else
			m_field = field_fan_on_temp;

		updateDynamics();
		_buttons.maskButtonsUntilClear();
	}

	// Figure which value are we going to change
	int *targetValue;

	if(m_field == field_fan_on_temp)
		targetValue = &g_settings.m_fanOnTemp;
	else
		targetValue = &g_settings.m_fanOffTemp;

	// Increase value?
	unsigned long buttonTime;
	bool updateDisplay = false;
	buttonTime = _buttons.getButton(BC_BUTTON_UP);
	if(buttonTime > 0)
	{
		if(buttonTime < SETUP_TIME_BUMP)
			(*targetValue)++;
		else
			(*targetValue) += 10;

		m_buttonTimer.start(SETUP_TIME_BUMP);
		updateDisplay = true;
	}

	// Decrease?
	buttonTime = _buttons.getButton(BC_BUTTON_DOWN);
	if(buttonTime > 0)
	{
		if(buttonTime < SETUP_TIME_BUMP)
			(*targetValue)--;
		else
			(*targetValue) -= 10;

		m_buttonTimer.start(SETUP_TIME_BUMP);
		updateDisplay = true;
	}

	// Keep them within bounds
	g_settings.m_fanOnTemp = constrain(g_settings.m_fanOnTemp, MIN_FAN_ON_TEMP, MAX_FAN_ON_TEMP);
	g_settings.m_fanOffTemp = constrain(g_settings.m_fanOffTemp, MIN_FAN_OFF_TEMP, MAX_FAN_OFF_TEMP);

	// Check for minimum hysteresis
	if((g_settings.m_fanOnTemp - g_settings.m_fanOffTemp) < MIN_FAN_HYSTERESIS)
		g_settings.m_fanOnTemp = g_settings.m_fanOffTemp + MIN_FAN_HYSTERESIS;

	if(updateDisplay)
		updateDynamics();
}

void CScreen_Setup_Fan::processOneSecond()
{
//	updateDynamics();
}
