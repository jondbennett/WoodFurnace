#ifndef Settings_h
#define Settings_h

class CWoodStoveSettings
{
protected:
	void setDefaults();

public:

	// Target flue temperatures
	int m_targetIdleTemp;
	int m_targetRunTemp;

	// Over-temperature alarm
	int m_alarmFlueTemp;

	// Time to wait to hit target flue temp
	int m_flueTempWaitTime;

	// Fan controls
	int m_fanOnTemp;	// Temp at which the fan comes on
	int m_fanOffTemp;	// Temp below which the fan is turned off

	// Coefs for the flue temp PID
	float m_Kp;
	float m_Ki;
	float m_Kd;

	CWoodStoveSettings();
	virtual ~CWoodStoveSettings();

	void loadSettings();
	void saveSettings(bool _saveDefaults = false);
};

extern CWoodStoveSettings g_settings;

#endif
