////////////////////////////////////////////////////////////
// Forced air fan controller
////////////////////////////////////////////////////////////
#include <Arduino.h>

#include "Pins.h"
#include "Defs.h"
#include "Settings.h"
#include "TempSensor_Thermocouple.h"

#include "FanController.h"

extern CTempSensor_Thermocouple g_thermocouple;
extern CWoodStoveSettings g_woodStoveSettings;

////////////////////////////////////////////////////////////
// Control the forced air fan based on a
// reference temperature (Flue temp?)
////////////////////////////////////////////////////////////
CFanController::CFanController()
{
	m_forcedOn = false;
}

CFanController::~CFanController()
{

}


void CFanController::setup()
{
	// ----------------------------------------
	// Prep the fan relay
	pinMode(PIN_FAN_RELAY, OUTPUT);
	digitalWrite(PIN_FAN_RELAY, RELAY_OFF);

	// ----------------------------------------
	// Prep the fan request input
	pinMode(PIN_CALL_FOR_FAN, INPUT_PULLUP);

	m_fanOn = false;
}

void CFanController::processOneSecond()
{
	// ----------------------------------------
	// Manual control of the fan
	if(digitalRead(PIN_CALL_FOR_FAN) == 0)
	{
#ifdef DEBUG_FAN_CONTROLLER
			Serial.println(F("CFanController::processOneSecond - manual fan ON."));
#endif
		setFanOn(true);
		return;
	}

	// If the fan is forced on then
	// just do it.
	if(m_forcedOn)
	{
		setFanOn(true);
		return;
	}

	// ----------------------------------------
	// Control the fan
	int currentTemp = g_thermocouple.temperature();
	if(currentTemp != THERMOCOUPLE_INVALID_TEMP)
	{

		if(currentTemp >= g_settings.m_fanOnTemp)
		{
			// Fan on?
#ifdef DEBUG_FAN_CONTROLLER
			Serial.println(F("CFanController::processOneSecond - automatic fan ON."));
#endif
			setFanOn(true);
		}
		else if(currentTemp <= g_settings.m_fanOffTemp)
		{
			// Fan off?
			setFanOn(false);
#ifdef DEBUG_FAN_CONTROLLER
			Serial.println(F("CFanController::processOneSecond - automatic fan OFF."));
#endif
		}
	}
}

void CFanController::setFanOn(bool _on)
{
	// Only write the relay if the on/off state
	// is changing
	if(m_fanOn != _on)
	{
		m_fanOn = _on;
		digitalWrite(PIN_FAN_RELAY, m_fanOn ? RELAY_ON : RELAY_OFF);
	}
}
