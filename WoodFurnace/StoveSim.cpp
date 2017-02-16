////////////////////////////////////////////////////////////
// Naive Wood stove simulator
////////////////////////////////////////////////////////////
#include <Arduino.h>

#include "Defs.h"
#include "TempSensor_Thermocouple.h"

#include "StoveSim.h"

double mapd(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//#define TEST_DEAD_PROBE_ALARM
//#define TEST_OVER_TEMP_ALARM
//#define TEST_FUEL_ALARM

#ifndef TEST_FUEL_ALARM
#define INITIAL_FUEL_LOAD			(40. * 16.)	// In oz.
#else
#define INITIAL_FUEL_LOAD			(3. * 16.)	// In oz.
#endif

#define AMBIENT_TEMP				(65)		// Cold basement
#define COOLING_RATE				(2.0 / 60.)	// I just made it up (evaluates to degrees per minute per degree)

#define DEGS_PER_OZ_PER_SEC			(360.)		// Has no basis in science, I just made this one up too
#define FUEL_OZ_PER_SEC_AT_FULL_PWM	(0.05)		// Another value pulled from the nether regions


////////////////////////////////////////////////////////////
// For testing the wood stove controller
////////////////////////////////////////////////////////////
CStoveSim::CStoveSim()
{
	// Prep for operation
	m_fuelLoad = INITIAL_FUEL_LOAD;

#ifdef TEST_FUEL_ALARM
	m_fuelLoad *= 0.50;
#endif

	m_idleTemp = AMBIENT_TEMP;
	m_curTemp = m_idleTemp;

	for(int _ = 0; _ < STOVE_SIM_DELAY_TIME; ++_)
		m_temps[_] = m_curTemp;

	m_forcedDraftSpeed = 0;
}

CStoveSim::~CStoveSim()
{

}

void CStoveSim::processOneSecond()
{
#ifdef DEBUG_SIMULATOR
	Serial.print("CStoveSim::processOneSecond() - current temp: ");
	Serial.println(m_curTemp);
#endif

	// Nothing happens if there
	// is no fire
	if(m_curTemp == AMBIENT_TEMP)
		return;

	// ----------------------------------------------------------
	// Calculate cooling losses
	double coolingLoss = (m_curTemp - AMBIENT_TEMP) * COOLING_RATE;
#ifdef DEBUG_SIMULATOR
	Serial.print("CStoveSim::processOneSecond - cooling loss: ");
	Serial.println(-coolingLoss);
#endif
	m_curTemp -= coolingLoss;

	// ----------------------------------------------------------
	// Calculate fuel consumption (base idle + forced draft)
	double baseFuelConsumption = ((m_idleTemp - AMBIENT_TEMP) * COOLING_RATE)/(DEGS_PER_OZ_PER_SEC);

	double forcedDraftFuelConsumption = mapd(m_forcedDraftSpeed, 0., 255., 0., FUEL_OZ_PER_SEC_AT_FULL_PWM);

	double cycleFuelConsumption = baseFuelConsumption + forcedDraftFuelConsumption;
	if(m_fuelLoad > cycleFuelConsumption)
	{
		m_fuelLoad -= cycleFuelConsumption;
	}
	else
	{
		cycleFuelConsumption = m_fuelLoad;
		m_fuelLoad = 0.;
	}

#ifdef DEBUG_SIMULATOR
	Serial.print("CStoveSim::processOneSecond - fuel consumed: ");
	Serial.println(cycleFuelConsumption, 4);
#endif
	// ----------------------------------------------------------
	// Calculate temperature gain
	double tempGain = DEGS_PER_OZ_PER_SEC * cycleFuelConsumption;
	double fuelUsedFactor = 1. - (m_fuelLoad / INITIAL_FUEL_LOAD);
	double tailScale;

	// This section is used to emulate the loss of energy in
	// the charcoal. Once we have used up most of the fuel, what
	// remains becomes less energetic. This allows the out-of-fuel
	// condition to taper rather than just drop.
	if(fuelUsedFactor < 0.75)
		tailScale = 1.;
	else
		tailScale = mapd(fuelUsedFactor, 0.75, 1., 1., 0.);
	tempGain *= tailScale;

#ifdef DEBUG_SIMULATOR
	Serial.print("CStoveSim::processOneSecond - temp Gain: ");
	Serial.println(tempGain);
#endif
	m_curTemp += tempGain;

#ifdef TEST_OVER_TEMP_ALARM
#warning Testing over-temp alarm
	{
		static bool overTemp = false;
		if((!overTemp) && (m_curTemp > 360.))
		{
			overTemp = true;
			m_curTemp = 600.;
		}
	}
#endif

	// ----------------------------------------------------------
	// Put the new current temp in the array
	for(int _ = (STOVE_SIM_DELAY_TIME -1 ); _ > 0; --_)
		m_temps[_] = m_temps[_ - 1];
	m_temps[0] = m_curTemp;
}

void CStoveSim::setForcedDraftBlower(int _speed)
{
	m_forcedDraftSpeed = _speed;
}

int CStoveSim::getPWM()
{
	return m_forcedDraftSpeed;
}

int CStoveSim::getTemperature()
{
#ifdef TEST_DEAD_PROBE_ALARM
	return THERMOCOUPLE_INVALID_TEMP;
#endif

	return (int)m_temps[STOVE_SIM_DELAY_TIME - 1];
}

double CStoveSim::getFuelLoad()
{
	return m_fuelLoad;
}

void CStoveSim::bumpToMinForcedDraftTemp()
{
#ifdef DEBUG_SIMULATOR
	Serial.println("CStoveSim::bumpToMinForcedDraftTemp - bumping");
#endif

	// Refuel the stove?
	if(m_fuelLoad < 10.)
	{
		m_fuelLoad = INITIAL_FUEL_LOAD;
		m_idleTemp = AMBIENT_TEMP;
	}

	// Start a  fire
	if(m_idleTemp == AMBIENT_TEMP)
	{
		m_idleTemp = MIN_FORCED_DRAFT_TEMP;
		if(m_curTemp < m_idleTemp)
			m_curTemp = m_idleTemp;
	}
}
