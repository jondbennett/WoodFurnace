////////////////////////////////////////////////////////////
// Beeper controller
////////////////////////////////////////////////////////////

#include <Arduino.h>

#include "Defs.h"
#include "Pins.h"
#include "MilliTimer.h"

#include "Beeper.h"

////////////////////////////////////////////////////////////
// Uses a relay to turn a loud beeper on and off for specified
// periods of time. Can be muted for a period before returning
// to normal operation.
////////////////////////////////////////////////////////////
CBeeper::CBeeper()
{
	m_state = state_off;
	m_onTime = m_offTime = 0;
}

CBeeper::~CBeeper()
{
}

void CBeeper::setup()
{
	pinMode(PIN_BEEPER_RELAY, OUTPUT);
	digitalWrite(PIN_BEEPER_RELAY, RELAY_OFF);
}

void CBeeper::processFast()
{
	// Don't cycle if we are muted
	if(m_muteTimer.getState() == CMilliTimer::running)
		return;

	// If the mute timer has expired then return to
	// normal operation
	if(m_muteTimer.getState() == CMilliTimer::expired)
		m_muteTimer.reset();

	switch(m_state)
	{
	case state_off:
		break;

	case state_start:
#ifdef DEBUG_BEEPER
		Serial.println(F("CBeeper::processOneSecond() - beeper on"));
#endif
		digitalWrite(PIN_BEEPER_RELAY, RELAY_ON);
		m_timer.start(m_onTime * ONE_SECOND_MS);
		m_state = state_onTime;
		break;

	case state_onTime:
		if(m_timer.getState() == CMilliTimer::expired)
		{
#ifdef DEBUG_BEEPER
			Serial.println(F("CBeeper::processOneSecond() - beeper off"));
#endif
			digitalWrite(PIN_BEEPER_RELAY, RELAY_OFF);
			m_timer.start(m_offTime * ONE_SECOND_MS);
			m_state = state_offTime;
		}
		break;

	case state_offTime:
		if(m_timer.getState() == CMilliTimer::expired)
			m_state = state_start;
		break;
	}
}

void CBeeper::beep(unsigned long _onTime, unsigned long _offTime)
{
	// Don't keep re-starting or re-stopping
	if((m_onTime == _onTime ) && (m_offTime == _offTime))
		return;

	// Starting or stopping
	if(_onTime > 0 && _offTime > 0)
	{
		// Starting
		m_onTime = _onTime;
		m_offTime = _offTime;

#ifdef DEBUG_BEEPER
		Serial.println(F("CBeeper::beep() - starting beeper"));
#endif
		// Start the incessant beeping
		if(m_state == state_off)
			m_state = state_start;
	}
	else
	{
#ifdef DEBUG_BEEPER
		Serial.println(F("CBeeper::beep() - stopping beeper"));
#endif
		// Please stop... please!
		m_state = state_off;
		m_onTime = m_offTime = 0;
		digitalWrite(PIN_BEEPER_RELAY, RELAY_OFF);
		m_timer.reset();
	}
}

void CBeeper::mute(unsigned long _time)
{
	// If the timer is not set then go ahead and
	// start it. We use the timer's state to keep from
	// re-starting the mute timer every time the button
	// has been pressed.
	if( (m_state != state_off) &&
		(m_muteTimer.getState() == CMilliTimer::notSet) )
	{
#ifdef DEBUG_BEEPER
		Serial.println(F("CBeeper::mute() - muting beeper"));
#endif
		m_muteTimer.start(_time);
		digitalWrite(PIN_BEEPER_RELAY, RELAY_OFF);
	}
}
