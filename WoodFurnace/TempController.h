////////////////////////////////////////////////////////////
// Wood Stove Temperature Controller
////////////////////////////////////////////////////////////
#ifndef TempController_h
#define TempController_h

////////////////////////////////////////////////////////////
// State machine and PID to control the wood stove flue temp
////////////////////////////////////////////////////////////
class CTempController
{
public:
	// The various alarm types
	typedef enum
	{
		alarm_none = 0,
		alarm_badProbe,
		alarm_overTemp,
	} CTempController_alarmE;

	// States within the state machine
	typedef enum
	{
		state_noFire = 0,
		state_idle,
		state_running,
		state_dyingFire,
		state_alarm,
		state_airBoost,
	} CTempController_stateE;

protected:
	// Actions taken by the state machine
	typedef enum
	{
		action_none,

		action_forcedDraftOff,
		action_forcedDraftIdle,
		action_forcedDraftRun,
		action_forcedDraftFull,

		action_startAirBoostTimer,

		action_startFuelWaitAlarm,
		action_stopFuelWaitAlarm,

		action_alarmConditionOn,
		action_alarmConditionOff,

	} CTempController_actionE;

	CTempController_stateE m_state;

	int m_currentTemp;
	CMilliTimer m_flueTempTimer;

	int m_dyingFireLowestTemp;
	CMilliTimer m_fuelWaitTimer;

	bool m_airBoostButtonPressed;
	CMilliTimer m_airBoostTimer;

	bool m_dyingFireAlarmInIdle;
	bool m_coldStart;

	int m_idleSpeedOverride;

	CWSPID m_pid;

	void changeState(int _lineNo, CTempController_stateE _newState);
	void takeAction(int _lineNo, CTempController_actionE _action);

	void printState(CTempController_stateE _state, bool _lf);

public:
	CTempController();
	virtual ~CTempController();

	bool callingForHeat();
	CTempController_stateE getState() { return m_state; }

	int getTargetTemp();

	void setup();
	void processFast();
	void processOneSecond();

	void updateSettings();

	CTempController_alarmE temperatureAlarm();

	int getIdleSpeedOverride()
	{
		return m_idleSpeedOverride;
	}

	void setIdleSpeedOverride(int _s)
	{
		m_idleSpeedOverride = _s;
	}
};

#endif
