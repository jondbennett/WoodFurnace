////////////////////////////////////////////////////////////
// Beeper controller
////////////////////////////////////////////////////////////
#ifndef Beeper_h
#define Beeper_h

////////////////////////////////////////////////////////////
// Toggles a relay that should run a beeper or alarm
////////////////////////////////////////////////////////////
class CBeeper
{
protected:

	typedef enum
	{
		state_off,
		state_start,
		state_onTime,
		state_offTime,
	} CBeeper_stateE;

	CBeeper_stateE m_state;
	CMilliTimer m_timer;

	CMilliTimer m_muteTimer;

	unsigned long m_onTime;
	unsigned long m_offTime;

public:
	CBeeper();
	virtual ~CBeeper();

	void setup();
	void processFast();

	void beep(unsigned long _onTime, unsigned long _offTime);
	void mute(unsigned long _time);
	void off()
	{
		beep(0, 0);
	}
};

#endif
