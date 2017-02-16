////////////////////////////////////////////////////////////
// PWM Motor Controller
////////////////////////////////////////////////////////////
#include <Arduino.h>

#include "Pins.h"
#include "Defs.h"
#include "MilliTimer.h"

#include "PWMMotor.h"

#ifdef SIMULATION_MODE
#include "StoveSim.h"
extern CStoveSim g_stoveSim;
#endif
////////////////////////////////////////////////////////////
// More Description and notes
////////////////////////////////////////////////////////////
CPWMMotor::CPWMMotor()
{
}

CPWMMotor::~CPWMMotor()
{
}

void CPWMMotor::setup()
{
#ifdef PIN_FORCED_DRAFT_GND
	pinMode(PIN_FORCED_DRAFT_GND, OUTPUT);
	digitalWrite(PIN_FORCED_DRAFT_GND, 0);
#endif

	m_lastCommand = 0;
	writePWM(0);
}

void CPWMMotor::setSpeed(int _speed)
{
	// Sanity
	if(_speed < 0 || _speed > PWM_MOTOR_MAX_COMMAND)
		return;

	// Don't go below the minimum speed at which the motor will run.
	if((_speed > 0) && (_speed < PWM_MOTOR_MIN_COMMAND))
		_speed = PWM_MOTOR_MIN_COMMAND;

	// If nothing is changing then we're done
	if(_speed == m_lastCommand)
		return;

	// Something is changing

#ifdef DEBUG_PWM_MOTOR
	Serial.print(F("CPWMMotor::setSpeed(): "));
	Serial.println(_speed);
#endif

	// If the command is 0 then stop the motor
	if(_speed == 0)
	{
		m_lastCommand = 0;
		writePWM(0);
		return;
	}

	// If the motor is not running then start it up,
	// otherwise just change the speed
	if(m_lastCommand == 0)
	{
		// The motor is not running, so do the startup procedure
#ifdef DEBUG_PWM_MOTOR
		Serial.println(F("CPWMMotor::setSpeed() - starting motor."));
#endif
		m_lastCommand = _speed;
		writePWM(PWM_MOTOR_MAX_COMMAND);
		m_startupTimer.start(PWM_MOTOR_STARTUP_TIME);
	}
	else
	{
#ifdef DEBUG_PWM_MOTOR
		Serial.println(F("CPWMMotor::setSpeed() - changing motor speed."));
#endif
		// The motor is running so just change the speed
		m_lastCommand = _speed;

		// OK, this is a boundary condition. If we are commanded to
		// change the motor speed while the startup timer is still running,
		// we don't want to write the PWM value yet because the motor
		// may fail to start. So, we only set the
		// PWM value if the timer is not set. If the timer is either
		// running or expired then the PWM value will be written
		// later, in the processFast() method below.
		if(m_startupTimer.getState() == CMilliTimer::notSet)
			writePWM(m_lastCommand);
	}
}

void CPWMMotor::processFast()
{
	// If the timer is in expired state the it had to have been started.
	// That only happens when we are starting the motor, so when it expires
	// just write the correct speed. Resetting the timer changes its state
	// to CMilliTimer::state_notSet, so this will only happen once per
	// motor start.
	if(m_startupTimer.getState() == CMilliTimer::expired)
	{
#ifdef DEBUG_PWM_MOTOR
		Serial.println(F("CPWMMotor::setSpeed() - startup complete, setting speed."));
#endif
		writePWM(m_lastCommand);
		m_startupTimer.reset();
	}
}

void CPWMMotor::writePWM(int _speed)
{
#ifdef DEBUG_PWM_MOTOR
	Serial.print(F("CPWMMotor::writePWM(): "));
	Serial.println(_speed);
#endif

#ifdef SIMULATION_MODE
	g_stoveSim.setForcedDraftBlower(_speed);
#endif
	analogWrite(PIN_FORCED_DRAFT_PWM, _speed);
}


