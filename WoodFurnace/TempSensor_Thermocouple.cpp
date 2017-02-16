////////////////////////////////////////////////////
// Thermocouple temperature sensor based on MAX6675
////////////////////////////////////////////////////
#include <Arduino.h>
#include <max6675.h>

#include "Pins.h"
#include "Defs.h"

#ifdef SIMULATION_MODE
#include "StoveSim.h"
extern CStoveSim g_stoveSim;
#endif
#include "TempSensor_Thermocouple.h"

// =================================================
// 'structors
// =================================================
CTempSensor_Thermocouple::CTempSensor_Thermocouple()
{
	// Init instance variables
	m_thermocoupleInterface = 0;

	m_nextReadTime = 0;

	m_temp = THERMOCOUPLE_INVALID_TEMP;
}

CTempSensor_Thermocouple::~CTempSensor_Thermocouple()
{
	if(m_thermocoupleInterface != 0)
	{
		MAX6675 *thermocouple = (MAX6675 *)m_thermocoupleInterface;
		delete thermocouple;
		m_thermocoupleInterface = 0;
	}
}

// =================================================
// Prepare for operation
// =================================================
void CTempSensor_Thermocouple::init()
{
#ifdef DEBUG_TEMPSENSOR
		Serial.println(F("CTempSensor_Thermocouple::init()"));
#endif

	// Set Gnd pin for thermocouple interface
#ifdef PIN_THERMOCOUPLE_GND
	pinMode(PIN_THERMOCOUPLE_GND, OUTPUT);
	digitalWrite(PIN_THERMOCOUPLE_GND, LOW);
#endif

	// Set VCC pin for thermocouple interface
#ifdef PIN_THERMOCOUPLE_VCC
	pinMode(PIN_THERMOCOUPLE_VCC, OUTPUT);
	digitalWrite(PIN_THERMOCOUPLE_VCC, HIGH);
#endif

	// CS is an output, active low
	pinMode(PIN_THERMOCOUPLE_CS, OUTPUT);
	digitalWrite(PIN_THERMOCOUPLE_CS, HIGH);

	// SCK is an output, active high
	pinMode(PIN_THERMOCOUPLE_SCK, OUTPUT);
	digitalWrite(PIN_THERMOCOUPLE_SCK, LOW);

	// SO is an input / output, but we use it as input only
	pinMode(PIN_THERMOCOUPLE_SO, INPUT);

	// Let the chip stabilize
	delay(500);		// This seems needed in order for the first reading to be correct

	// Create the thermocouple interface
	if(m_thermocoupleInterface == 0)
	{
		// Create the thermocouple interface
		m_thermocoupleInterface = (void *)new MAX6675(PIN_THERMOCOUPLE_SCK, PIN_THERMOCOUPLE_CS, PIN_THERMOCOUPLE_SO);
	}
}

// =================================================
// Operate
// =================================================
void CTempSensor_Thermocouple::updateTemp()
{
	// If the interface has been created then read the temperature
	if(m_thermocoupleInterface)
	{
		MAX6675 *thermocouple = (MAX6675 *)m_thermocoupleInterface;

		// Check for open thermocouple
		if(isnan(thermocouple->readCelsius()))
		{
#ifdef DEBUG_TEMPSENSOR
		printUptime();
		Serial.println(F("CTempSensor_Thermocouple::updateTemp() - readCelsius returned NaN"));
#endif
			m_temp = THERMOCOUPLE_INVALID_TEMP;
			return;
		}

		// Smooth them out and store the results in actual
		double newTemp = thermocouple->readFahrenheit();
		if(m_temp == THERMOCOUPLE_INVALID_TEMP)
			m_temp = newTemp;

		m_temp = (newTemp * THERMOCOUPLE_EMA_ALPHA) +
				(m_temp * (1. - THERMOCOUPLE_EMA_ALPHA));

#ifdef DEBUG_TEMPSENSOR
		printUptime();
		Serial.print(F("CTempSensor_Thermocouple::updateTemp() - temp updated: "));
		Serial.println(m_temp);
#endif

	}
	else
	{
#ifdef DEBUG_TEMPSENSOR
		printUptime();
		Serial.println(F("CTempSensor_Thermocouple::updateTemp() - m_thermocoupleInterface == 0"));
#endif
		m_temp = THERMOCOUPLE_INVALID_TEMP;
	}
}

int CTempSensor_Thermocouple::temperature()
{
#ifdef SIMULATION_MODE
	return g_stoveSim.getTemperature();
#endif

	// Is it time to update the reading?
	if((m_temp == THERMOCOUPLE_INVALID_TEMP) ||
	   (millis() > m_nextReadTime))
	{
		// Yes, so read the hardware
		updateTemp();

		// Remember the next update time
		m_nextReadTime = millis() + ONE_SECOND_MS;
	}

	// Return latest temp reading
	return m_temp;
}


