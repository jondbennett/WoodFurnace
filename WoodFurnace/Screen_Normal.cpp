////////////////////////////////////////////////////////////
// Normal Screen
////////////////////////////////////////////////////////////
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#include <PID_v1.h>

#include "Pins.h"
#include "Defs.h"
#include "MilliTimer.h"
#include "Settings.h"
#include "TempSensor_Thermocouple.h"
#include "PWMMotor.h"
#include "WSPID.h"
#include "Beeper.h"
#include "FanController.h"
#include "ScreenController.h"
#include "TempController.h"

#include "Screen_Normal.h"


extern Adafruit_RGBLCDShield g_lcd;
extern const char *degreeSymbol;

extern CTempSensor_Thermocouple g_thermocouple;
extern CWoodStoveSettings g_woodStoveSettings;
extern CPWMMotor g_forcedDraftMotor;
extern CBeeper g_beeper;
extern CFanController g_fanController;
extern CScreenController g_screenController;
extern CTempController g_tempController;
extern void pidSettingsChanged();

////////////////////////////////////////////////////////////
// Display normal temperature situation
////////////////////////////////////////////////////////////
CScreen_Normal::CScreen_Normal(int _id) : CScreen_Base(_id)
{

}

CScreen_Normal::~CScreen_Normal()
{

}

void CScreen_Normal::init()
{
#ifdef DEBUG_SCREEN_NORMAL
	printUptime();
	Serial.println(F("CScreen_Normal::init()"));
#endif
	g_lcd.clear();
	g_lcd.noCursor();

	m_lastTargetTemp = -1;
	m_lastFlueTemp = -1;
	m_lastForcedDraftPercent = -1;

	m_showingAlarm = false;

	updateStatics();
	updateDynamics();
}

void CScreen_Normal::updateStatics()
{
#ifdef DEBUG_SCREEN_NORMAL
	printUptime();
	Serial.println(F("CScreen_Normal::updateStatics()"));
#endif


	g_lcd.setCursor(0, 0);
	g_lcd.print(F("Flue:     (    )"));

	g_lcd.setCursor(0, 1);
	if(!m_showingAlarm)
	{

		g_lcd.print(F("FD:     H  F    "));
	}
	else
	{
		g_lcd.print(F("                "));
	}

}

void CScreen_Normal::updateDynamics()
{
#ifdef DEBUG_SCREEN_NORMAL
	printUptime();
	Serial.println(F("CScreen_Normal::updateDynamics()"));
#endif

	// Actual temp
	int temperature = g_thermocouple.temperature();
	if(m_lastFlueTemp != temperature)
	{
		g_lcd.setCursor(5, 0);
		g_lcd.print(F("    "));
		g_lcd.setCursor(5, 0);

		if(temperature == THERMOCOUPLE_INVALID_TEMP)
			g_lcd.print(F("---"));
		else
			g_lcd.print(temperature);

		g_lcd.print(degreeSymbol);

		m_lastFlueTemp = temperature;
	}

	// Target temp
	int targetFlueTemp = g_tempController.getTargetTemp();
	if(m_lastTargetTemp != targetFlueTemp)
	{
		g_lcd.setCursor(11, 0);
		g_lcd.print(F("    "));
		g_lcd.setCursor(11, 0);

		if(targetFlueTemp != THERMOCOUPLE_INVALID_TEMP)
			g_lcd.print(targetFlueTemp);
		else
			g_lcd.print(F("---"));
		g_lcd.print(degreeSymbol);

		m_lastTargetTemp = targetFlueTemp;
	}

	// Are we exiting alarm state?
	if((m_showingAlarm) && (g_tempController.temperatureAlarm() == CTempController::alarm_none))
	{
		// Alarm condition has cleared
		m_showingAlarm = false;
		updateStatics();
	}

	// Are we entering alarm state?
	if((!m_showingAlarm) && (g_tempController.temperatureAlarm() != CTempController::alarm_none))
	{
		// Alarm condition has cleared
		m_lastForcedDraftPercent = -1;

		m_showingAlarm = true;
		updateStatics();
	}

	// Second line depends on alarm status
	if(!m_showingAlarm)
	{
		// If we are not in alarm mode then show the usual
		// forced draft %, call for heat, fan status and TempController state

		// Forced draft %
		int forcedDraftPercent = (((double)g_forcedDraftMotor.getSpeed() / (double)PWM_MOTOR_MAX_COMMAND) * 100.);
		if(m_lastForcedDraftPercent != forcedDraftPercent)
		{
			g_lcd.setCursor(3, 1);
			g_lcd.print(F("    "));
			g_lcd.setCursor(3, 1);
			g_lcd.print(forcedDraftPercent);
			g_lcd.print(F("%"));

			m_lastForcedDraftPercent = forcedDraftPercent;
		}

		// Call for heat
		g_lcd.setCursor(9, 1);
		g_lcd.print(g_tempController.callingForHeat() ? F("*") : F("-"));

		// Fan status
		g_lcd.setCursor(12, 1);
		g_lcd.print(g_fanController.isFanOn() ? F("*") : F("-"));

		// Show temperature controller state
		g_lcd.setCursor(15, 1);
		switch(g_tempController.getState())
		{
		default:
			g_lcd.print(F("?"));
			break;

		case CTempController::state_noFire:
			g_lcd.print(F("O"));
			break;

		case CTempController::state_idle:
			g_lcd.print(F("I"));
			break;

		case CTempController::state_running:
			g_lcd.print(F("R"));
			break;

		case CTempController::state_dyingFire:
			g_lcd.print(F("D"));
			break;

		case CTempController::state_alarm:
			g_lcd.print(F("A"));
			break;

		case CTempController::state_airBoost:
			g_lcd.print(F("B"));
			break;
		}
	}
	else
	{
		// We are in alarm mode, show the type of the alarm
		g_lcd.setCursor(0, 1);
		if(g_tempController.temperatureAlarm() == CTempController::alarm_badProbe)
		{
			g_lcd.print(F("* Probe Error *"));
		}

		if(g_tempController.temperatureAlarm() == CTempController::alarm_overTemp)
		{
			g_lcd.print(F("* OVER TEMP *"));
		}
	}
}

void CScreen_Normal::buttonCheck(CButtonController &_buttons)
{
	// Press and hold select to enter setup mode
	if(_buttons.getButton(BC_BUTTON_SELECT) > SETUP_TIME_ENTER_SETUP)
		g_screenController.setScreen(SCREEN_ID_SETUP_FLUE_TEMP);

	// Press and hold left to reset the system
	if(_buttons.getButton(BC_BUTTON_LEFT) > HOLD_TIME_SYSTEM_RESET)
	{
#ifdef DEBUG_SCREEN_NORMAL
		printUptime();
		Serial.println(F("CScreen_Normal::buttonCheck() - reset to defaults"));
#endif
		g_settings.saveSettings(true);
		pidSettingsChanged();
		init();
		_buttons.maskButtonsUntilClear();
	}
}

void CScreen_Normal::processOneSecond()
{
	updateDynamics();
}
