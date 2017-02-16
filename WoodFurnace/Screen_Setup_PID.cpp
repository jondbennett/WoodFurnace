////////////////////////////////////////////////////////////
// Blank Screen
////////////////////////////////////////////////////////////
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#include "Pins.h"
#include "Defs.h"
#include "MilliTimer.h"

#include "TempSensor_Thermocouple.h"
#include "ScreenController.h"
#include "Screen_Setup_PID.h"
#include "Settings.h"
#include "PWMMotor.h"

extern Adafruit_RGBLCDShield g_lcd;
extern const char *degreeSymbol;

extern CWoodStoveSettings g_woodStoveSettings;
extern CScreenController g_screenController;
extern void pidSettingsChanged();
extern CPWMMotor g_forcedDraftMotor;

extern const char *degreeSymbol;
extern CTempSensor_Thermocouple g_thermocouple;
////////////////////////////////////////////////////////////
// Display PID control values
////////////////////////////////////////////////////////////
CScreen_Setup_PID::CScreen_Setup_PID(int _id) : CScreen_Base(_id)
{

}

CScreen_Setup_PID::~CScreen_Setup_PID()
{

}

void CScreen_Setup_PID::init()
{
#ifdef DEBUG_SCREEN_SETUP_PID
	printUptime();
	Serial.println(F("CScreen_Setup_PID::init()"));
#endif
	m_field = field_PID_Kp;

	g_lcd.clear();
	g_lcd.cursor();
	g_lcd.noBlink();

	updateStatics();
	updateDynamics();
}

void CScreen_Setup_PID::updateStatics()
{
#ifdef DEBUG_SCREEN_SETUP_PID
	printUptime();
	Serial.println(F("CScreen_Setup_PID::updateStatics()"));
#endif


	g_lcd.setCursor(0, 0);
	g_lcd.print(F("Setup:Flue PID"));
}

void CScreen_Setup_PID::updateDynamics()
{
#ifdef DEBUG_SCREEN_SETUP_PID
	printUptime();
	Serial.println(F("CScreen_Setup_PID::updateDynamics()"));
#endif

	// Now display the P/I/D value depending on field selection
	g_lcd.setCursor(2, 1);
	g_lcd.print("     ");
	switch(m_field)
	{
	default:
	case field_PID_Kp:
		g_lcd.setCursor(0, 1);
		g_lcd.print(F("P:"));
		g_lcd.print(g_settings.m_Kp, 2);
		break;

	case field_PID_Ki:
		g_lcd.setCursor(0, 1);
		g_lcd.print(F("I:"));
		g_lcd.print(g_settings.m_Ki, 2);
		break;

	case field_PID_Kd:
		g_lcd.setCursor(0, 1);
		g_lcd.print(F("D:"));
		g_lcd.print(g_settings.m_Kd, 2);
		break;
	};

	g_lcd.setCursor(5, 1);
}

void CScreen_Setup_PID::buttonCheck(CButtonController &_buttons)
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
		g_screenController.setScreen(SCREEN_ID_NORMAL);
		return;
	}

	// Switch fields
	if(_buttons.getButton(BC_BUTTON_LEFT) > 0)
	{
#ifdef DEBUG_SCREEN_SETUP_PID
		printUptime();
		Serial.println(F("CScreen_Setup_PID::buttonCheck - switching fields"));
#endif
		g_settings.saveSettings();

		if(m_field == field_PID_Kp)
			m_field = field_PID_Kd;
		else if(m_field == field_PID_Ki)
			m_field = field_PID_Kp;
		else if(m_field == field_PID_Kd)
			m_field = field_PID_Ki;

		updateDynamics();
		_buttons.maskButtonsUntilClear();
	}

	if(_buttons.getButton(BC_BUTTON_RIGHT) > 0)
	{
#ifdef DEBUG_SCREEN_SETUP_PID
		printUptime();
		Serial.println(F("CScreen_Setup_PID::buttonCheck - switching fields"));
#endif
		g_settings.saveSettings();

		if(m_field == field_PID_Kp)
			m_field = field_PID_Ki;
		else if(m_field == field_PID_Ki)
			m_field = field_PID_Kd;
		else if(m_field == field_PID_Kd)
			m_field = field_PID_Kp;

		updateDynamics();
		_buttons.maskButtonsUntilClear();
	}

	// Figure which value are we going to change
	float *targetValue = 0;
	if(m_field == field_PID_Kp)
		targetValue = &g_settings.m_Kp;

	if(m_field == field_PID_Ki)
		targetValue = &g_settings.m_Ki;

	if(m_field == field_PID_Kd)
		targetValue = &g_settings.m_Kd;

	if(!targetValue)
	{
#ifdef DEBUG_SCREEN_SETUP_PID
		printUptime();
		Serial.println(F("CScreen_Setup_PID::buttonCheck - *** ERROR: Bad Field Selection ***"));
#endif
		return;
	}

	// Increase value?
	bool updateDisplay = false;
	unsigned long buttonTime;
	buttonTime = _buttons.getButton(BC_BUTTON_UP);
	if(buttonTime > 0)
	{
		if(buttonTime < SETUP_TIME_BUMP)
			(*targetValue) += 0.01;
		else if(buttonTime < (SETUP_TIME_BUMP * 8))
			(*targetValue) += 0.1;
		else
			(*targetValue) += 1.0;

		m_buttonTimer.start(SETUP_TIME_BUMP);
		updateDisplay = true;
	}

	// Decrease?
	buttonTime = _buttons.getButton(BC_BUTTON_DOWN);
	if(buttonTime > 0)
	{
		if(buttonTime < SETUP_TIME_BUMP)
			(*targetValue) -= 0.01;
		else if(buttonTime < (SETUP_TIME_BUMP * 8))
			(*targetValue) -= 0.1;
		else
			(*targetValue) -= 1.0;

		m_buttonTimer.start(SETUP_TIME_BUMP);
		updateDisplay = true;
	}

	if(g_settings.m_Kp < 0.) g_settings.m_Kp = 0.;
	if(g_settings.m_Ki < 0.) g_settings.m_Ki = 0.;
	if(g_settings.m_Kd < 0.) g_settings.m_Kd = 0.;

	if(updateDisplay)
	{
		updateDynamics();

		// Update the temp controller
		pidSettingsChanged();
	}
}

void CScreen_Setup_PID::updatePTInfo()
{
	// Display PWM output
	g_lcd.setCursor(8, 1);
	g_lcd.print(F("   "));
	g_lcd.setCursor(8, 1);
	g_lcd.print(g_forcedDraftMotor.getSpeed());

	// Display actual temperature
	g_lcd.setCursor(12, 1);
	g_lcd.print(F("    "));
	g_lcd.setCursor(12, 1);

	int temperature = g_thermocouple.temperature();
	if(temperature == THERMOCOUPLE_INVALID_TEMP)
		g_lcd.print(F("---"));
	else
		g_lcd.print(temperature);

	g_lcd.print(degreeSymbol);
	g_lcd.setCursor(5, 1);
}

void CScreen_Setup_PID::processOneSecond()
{
	updatePTInfo();
}
