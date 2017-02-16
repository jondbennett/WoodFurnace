////////////////////////////////////////////////////////////
// Set target flue temp
////////////////////////////////////////////////////////////
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#include "Pins.h"
#include "Defs.h"
#include "MilliTimer.h"

#include "ScreenController.h"
#include "Screen_Setup_FlueTemp.h"
#include "Settings.h"

extern Adafruit_RGBLCDShield g_lcd;
extern const char *degreeSymbol;

extern CWoodStoveSettings g_woodStoveSettings;
extern CScreenController g_screenController;
extern void pidSettingsChanged();

////////////////////////////////////////////////////////////
// Display normal temperature situation
////////////////////////////////////////////////////////////
CScreen_Setup_FlueTemp::CScreen_Setup_FlueTemp(int _id) : CScreen_Base(_id)
{

}

CScreen_Setup_FlueTemp::~CScreen_Setup_FlueTemp()
{

}

void CScreen_Setup_FlueTemp::init()
{
#ifdef DEBUG_SCREEN_SETUP_FLUE_TEMP
	printUptime();
	Serial.println(F("CScreen_Setup_FlueTemp::init()"));
#endif

	m_field = field_idleTemp;

	g_lcd.clear();
	g_lcd.cursor();
	g_lcd.noBlink();

	updateStatics();
	updateDynamics();
}

void CScreen_Setup_FlueTemp::updateStatics()
{
#ifdef DEBUG_SCREEN_SETUP_FLUE_TEMP
	printUptime();
	Serial.println(F("CScreen_Setup_FlueTemp::updateStatics()"));
#endif

	g_lcd.setCursor(0, 0);
	g_lcd.print(F("Setup:Flue Temp"));
	g_lcd.setCursor(0, 1);

	if(m_field == field_idleTemp)
		g_lcd.print(F("Idle:"));
	if(m_field == field_runTemp)
		g_lcd.print(F("Run:"));
	if(m_field == field_alarmTemp)
		g_lcd.print(F("Alarm:"));
}

void CScreen_Setup_FlueTemp::updateDynamics()
{
#ifdef DEBUG_SCREEN_SETUP_FLUE_TEMP
	printUptime();
	Serial.println(F("CScreen_Setup_FlueTemp::updateDynamics()"));
#endif

	if(m_field == field_idleTemp)
	{
		g_lcd.setCursor(5, 1);
		g_lcd.print(F("     "));

		g_lcd.setCursor(5, 1);
		g_lcd.print(g_settings.m_targetIdleTemp);
		g_lcd.print(degreeSymbol);
		g_lcd.setCursor(7, 1);
	}

	if(m_field == field_runTemp)
	{
		g_lcd.setCursor(4, 1);
		g_lcd.print(F("     "));

		g_lcd.setCursor(4, 1);
		g_lcd.print(g_settings.m_targetRunTemp);
		g_lcd.print(degreeSymbol);
		g_lcd.setCursor(6, 1);
	}


	if(m_field == field_alarmTemp)
	{
		g_lcd.setCursor(6, 1);
		g_lcd.print(F("     "));

		g_lcd.setCursor(6, 1);
		g_lcd.print(g_settings.m_alarmFlueTemp);
		g_lcd.print(degreeSymbol);
		g_lcd.setCursor(8, 1);
	}
}

void CScreen_Setup_FlueTemp::buttonCheck(CButtonController &_buttons)
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
		g_screenController.setScreen(SCREEN_ID_SETUP_FLUE_TEMP_WAIT);
		return;
	}


	if(_buttons.getButton(BC_BUTTON_RIGHT) > 0)
	{
#ifdef DEBUG_SCREEN_SETUP_FLUE_TEMP
		printUptime();
		Serial.println(F("CScreen_Setup_FlueTemp::buttonCheck - switching fields"));
#endif
		if(m_field == field_idleTemp)
			m_field = field_runTemp;
		else if(m_field == field_runTemp)
			m_field = field_alarmTemp;
		else if(m_field == field_alarmTemp)
			m_field = field_idleTemp;

		updateStatics();
		updateDynamics();
		_buttons.maskButtonsUntilClear();
	}

	if(_buttons.getButton(BC_BUTTON_LEFT) > 0)
	{
#ifdef DEBUG_SCREEN_SETUP_FLUE_TEMP
		printUptime();
		Serial.println(F("CScreen_Setup_FlueTemp::buttonCheck - switching fields"));
#endif
		if(m_field == field_idleTemp)
			m_field = field_alarmTemp;
		else if(m_field == field_alarmTemp)
			m_field = field_runTemp;
		else if(m_field == field_runTemp)
			m_field = field_idleTemp;

		updateStatics();
		updateDynamics();
		_buttons.maskButtonsUntilClear();
	}

	// Increase the target temp?
	unsigned long buttonTime;
	bool updateDisplay = false;
	int *targetValue = 0;

	if(m_field == field_idleTemp)
		targetValue = &g_settings.m_targetIdleTemp;
	else if (m_field == field_runTemp)
		targetValue = &g_settings.m_targetRunTemp;
	else if (m_field == field_alarmTemp)
		targetValue = &g_settings.m_alarmFlueTemp;

	if(!targetValue)
	{
#ifdef DEBUG_SCREEN_SETUP_FLUE_TEMP
		printUptime();
		Serial.println(F("CScreen_Setup_FlueTemp::buttonCheck - *** ERROR: Bad Field Selection ***"));
#endif
		return;
	}

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

	// Decrease the target temp?
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
	g_settings.m_targetIdleTemp = constrain(g_settings.m_targetIdleTemp, MIN_FLUE_TEMP_IDLE, MAX_FLUE_TEMP_IDLE);
	g_settings.m_targetRunTemp = constrain(g_settings.m_targetRunTemp, MIN_FLUE_TEMP_RUN, MAX_FLUE_TEMP_RUN);
	g_settings.m_alarmFlueTemp = constrain(g_settings.m_alarmFlueTemp, MIN_FLUE_TEMP_ALARM, MAX_FLUE_TEMP_ALARM);

	if(updateDisplay)
	{
		updateDynamics();
		pidSettingsChanged();
	}
}

void CScreen_Setup_FlueTemp::processOneSecond()
{
//	updateDynamics();
}
