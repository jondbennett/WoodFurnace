////////////////////////////////////////////////////////////
// Fan controller
////////////////////////////////////////////////////////////
#ifndef FanController_h
#define FanController_h

////////////////////////////////////////////////////////////
// Control the forced air fan based on a
// reference temperature (Flue temp?)
////////////////////////////////////////////////////////////
class CFanController
{
protected:
	bool m_fanOn;

	bool m_forcedOn;

	void setFanOn(bool _on);

public:
	CFanController();
	virtual ~CFanController();

	void setup();
	void processOneSecond();

	bool isFanOn()
	{
		return m_fanOn;
	}

	void forceFanOn(bool _force)
	{
		m_forcedOn = _force;
	}
};

#endif
