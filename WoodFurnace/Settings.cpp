#include <Arduino.h>
#include <SaveController.h>

#include "Defs.h"
#include "Settings.h"

//////////////////////////////////////////////////////
// Save Controller
static CSaveController s_saveController('W', 'o', 'o', 'd');

CWoodStoveSettings::CWoodStoveSettings()
{
	setDefaults();
}

void CWoodStoveSettings::setDefaults()
{
	// Target flue temperatures

	m_targetIdleTemp = DEF_FLUE_TEMP_IDLE;
	m_targetRunTemp = DEF_FLUE_TEMP_RUN;

	// Alarm temp
	m_alarmFlueTemp = DEF_FLUE_TEMP_ALARM;

	// Flue temp wait time
	m_flueTempWaitTime = DEF_FLUE_TEMP_WAIT_TIME;

	// Fan controls
	m_fanOnTemp = DEF_FAN_ON_TEMP;		// Temp above which the fan is turned on
	m_fanOffTemp = DEF_FAN_OFF_TEMP;	// Temp below which the fan is turned off

	// Coefs for the flue temp PID
	m_Kp = DEF_KP;
	m_Ki = DEF_KI;
	m_Kd = DEF_KD;
}

CWoodStoveSettings::~CWoodStoveSettings()
{
}

void CWoodStoveSettings::loadSettings()
{
	// Make sure we have the correct data version
	if(s_saveController.getDataVersion() != WOODSTOVE_DATA_VERSION)
	{
#ifdef DEBUG_SETTINGS
		printUptime();
		Serial.print(F("CWoodStoveSettings::loadSettings - incorrect data version: "));
		Serial.println(s_saveController.getDataVersion());
#endif
		saveSettings(true);
	}
	else
	{
#ifdef DEBUG_SETTINGS
		printUptime();
		Serial.print(F("CWoodStoveSettings::loadSettings - correct data version: "));
		Serial.println(s_saveController.getDataVersion());
#endif
	}

	// Now load the settings
	s_saveController.rewind();

	// Target flue temperature
	m_targetIdleTemp = s_saveController.readInt();
	m_targetRunTemp = s_saveController.readInt();

	// Over temperature alarm
	m_alarmFlueTemp = s_saveController.readInt();

	// Flue temp wait time
	m_flueTempWaitTime = s_saveController.readInt();

	// Fan controls
	m_fanOnTemp = s_saveController.readInt();
	m_fanOffTemp = s_saveController.readInt();

	// Coefs for the flue temp PID
	m_Kp = s_saveController.readFloat();
	m_Ki = s_saveController.readFloat();
	m_Kd = s_saveController.readFloat();


#ifdef DEBUG_SETTINGS
	printUptime();
	Serial.println();

	Serial.print(F("CWoodStoveSettings::loadSettings - m_targetIdleTemp: "));
	Serial.println(m_targetIdleTemp);

	Serial.print(F("CWoodStoveSettings::loadSettings - m_targetRunTemp: "));
	Serial.println(m_targetRunTemp);

	Serial.print(F("CWoodStoveSettings::loadSettings - m_alarmFlueTemp: "));
	Serial.println(m_alarmFlueTemp);

	Serial.print(F("CWoodStoveSettings::loadSettings - m_flueTempWaitTime: "));
	Serial.println(m_flueTempWaitTime);

	Serial.print(F("CWoodStoveSettings::loadSettings - m_fanOnTemp: "));
	Serial.println(m_fanOnTemp);

	Serial.print(F("CWoodStoveSettings::loadSettings - m_fanOffTemp: "));
	Serial.println(m_fanOffTemp);

	Serial.print(F("CWoodStoveSettings::loadSettings - m_Kp: "));
	Serial.println(m_Kp);

	Serial.print(F("CWoodStoveSettings::loadSettings - m_Ki: "));
	Serial.println(m_Ki);

	Serial.print(F("CWoodStoveSettings::loadSettings - m_Kd: "));
	Serial.println(m_Kd);
#endif
}

void CWoodStoveSettings::saveSettings(bool _saveDefaults)
{

#ifdef DEBUG_SETTINGS
	printUptime();
	Serial.print(F("CWoodStoveSettings::saveSettings - defaults: "));
	Serial.println((_saveDefaults) ? F("true") : F("false"));
#endif

	if(_saveDefaults)
		setDefaults();

	// Now save everything
	s_saveController.rewind();
	s_saveController.updateHeader(WOODSTOVE_DATA_VERSION);

	// Target flue temperatures
	s_saveController.writeInt(m_targetIdleTemp);
	s_saveController.writeInt(m_targetRunTemp );

	// Alarm flue temperature
	s_saveController.writeInt(m_alarmFlueTemp);

	// Flue temp wait time
	s_saveController.writeInt(m_flueTempWaitTime);

	// Fan controls
	s_saveController.writeInt(m_fanOnTemp);
	s_saveController.writeInt(m_fanOffTemp);

	// Coefs for the flue temp PID
	s_saveController.writeFloat(m_Kp);
	s_saveController.writeFloat(m_Ki);
	s_saveController.writeFloat(m_Kd);
}


