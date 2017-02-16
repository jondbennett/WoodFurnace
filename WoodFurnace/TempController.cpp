////////////////////////////////////////////////////////////
// Wood Stove Temperature Controller
////////////////////////////////////////////////////////////
#include <Arduino.h>
#include <math.h>

#include <PID_v1.h>

#include "Pins.h"
#include "Defs.h"
#include "MilliTimer.h"
#include "Settings.h"
#include "TempSensor_Thermocouple.h"
#include "PWMMotor.h"
#include "WSPID.h"
#include "Beeper.h"
#include "FanController.h"
#include "ScreenController.h"
#include "TempController.h"

extern CTempSensor_Thermocouple g_thermocouple;
extern CWoodStoveSettings g_woodStoveSettings;
extern CPWMMotor g_forcedDraftMotor;
extern CBeeper g_beeper;
extern CFanController g_fanController;
extern CScreenController g_screenController;

#ifdef SIMULATION_MODE
#include "StoveSim.h"
extern CStoveSim g_stoveSim;
#endif

#define CHANGESTATE(s)	(changeState(__LINE__, s))
#define TAKEACTION(a)	(takeAction(__LINE__, a))

////////////////////////////////////////////////////////////
// State machine and PID to control the wood stove flue temp
////////////////////////////////////////////////////////////
CTempController::CTempController()
{
	m_airBoostButtonPressed = false;
	m_currentTemp = 0.;
	m_dyingFireLowestTemp = 0.;
	m_dyingFireAlarmInIdle = false;
	m_coldStart = false;
	m_idleSpeedOverride = 0;

	m_pid.SetSampleTime(1000);
	m_pid.SetScale(0.10);

	// Set the control limits of the PID. Note: the use of PWM_MOTOR_STOP as the minimum command
	// *instead of* PWM_MOTOR_MIN_COMMAND is intentional and correct. The PID should be able to
	// stop the forced draft blower if it needs to. So, PWM_MOTOR_MIN_COMMAND is only used in the
	// PWMMotor class as follows: If the command is non-zero then it will be limited on the low end to
	// PWM_MOTOR_MIN_COMMAND. However if the command is zero then the motor will stop. This
	// way, the PID can stop the motor if it needs to, but the motor will never see a non-zero
	// PWM command lower than is needed to keep the motor running.
	m_pid.SetOutputLimits(PWM_MOTOR_STOP, PWM_MOTOR_MAX_COMMAND);

	m_state = state_noFire;
	m_pid.SetMode(MANUAL);
	m_pid.SetOutput(PWM_MOTOR_STOP);
}

CTempController::~CTempController()
{
}

////////////////////////////////////////////////////////////
// Prep the I/O pins
void CTempController::setup()
{
#ifdef DEBUG_TEMP_CONTROLLER
	Serial.println(F("CTempController::setup()"));
#endif

	// Forced draft motor and
	g_forcedDraftMotor.setSpeed(0);

	// Call for heat input
	pinMode(PIN_CALL_FOR_HEAT, INPUT_PULLUP);

	// Mute-alarm button
	pinMode(PIN_MUTE_ALARM, INPUT_PULLUP);
	pinMode(PIN_MUTE_ALARM_C, OUTPUT);
	digitalWrite(PIN_MUTE_ALARM_C, LOW);

	// Start-fire button
	pinMode(PIN_FD_BLAST, INPUT_PULLUP);
	pinMode(PIN_FD_BLAST_C, OUTPUT);
	digitalWrite(PIN_FD_BLAST_C, LOW);

	updateSettings();
}

////////////////////////////////////////////////////////////
// Rapid processing for the PWM motor controller and
// mute/blast buttons
void CTempController::processFast()
{
	m_currentTemp = g_thermocouple.temperature();

	// Send it to the PWM
	double pidOutput = m_pid.Compute(m_currentTemp);

	// During the initial PID setup, when kI & kD are probably
	// zero, it is helpful to get the forced draft blower running
	// without PID control so that the stove will idle.
	if( (m_state == state_idle) && (m_idleSpeedOverride > 0) )
		pidOutput = m_idleSpeedOverride;

	// Set the speed computed by the PID
	g_forcedDraftMotor.setSpeed((int)roundf(pidOutput));

	// Check for the alarm mute button
	if(digitalRead(PIN_MUTE_ALARM) == 0)
	{
		// If we are alarmed, you can mute the system
		// for a while.
		if(m_state == state_alarm)
		{
			g_beeper.mute(BEEPER_ALARM_MUTE_TIME  * ONE_SECOND_MS);
			return;
		}

		// If the fire is dying the mute button
		// just shuts off the alarm
		if(m_state == state_dyingFire)
		{
			g_beeper.off();
		}

#ifdef SIMULATION_MODE
		g_stoveSim.bumpToMinForcedDraftTemp();
#endif
	}

	// Check the FD boost button and see if
	// we need help getting the fire started. This only
	// works in idle or dying fire
	if( (digitalRead(PIN_FD_BLAST) == 0) &&
		(m_state == state_noFire || m_state == state_idle || m_state == state_dyingFire) )
		m_airBoostButtonPressed = true;
}

////////////////////////////////////////////////////////////
// Slow processing for state machine and
// alarm
void CTempController::processOneSecond()
{
	// Provide the log info
#ifdef SERIAL_LOG
	Serial.print(F("LOG, "));

	printUptime(false);
	Serial.print(F(", "));

	Serial.print(callingForHeat() ? F("H, ") : F("I, "));

	printState(m_state, false);
	Serial.print(F(", "));

	Serial.print(m_currentTemp);
	Serial.print(F(", "));

	Serial.print(g_forcedDraftMotor.getSpeed());
	Serial.print(F(", "));

	Serial.println(g_fanController.isFanOn());
#endif

#ifdef SERIAL_PLOT
	Serial.print(callingForHeat() ? 10 : 0);	// The "10" just makes it visible on the plot

	Serial.print(F(", "));
	if(getTargetTemp() > 0)
		Serial.print(getTargetTemp());
	else
		Serial.print(0);

	Serial.print(F(", "));
	Serial.print(m_currentTemp);

	Serial.print(F(", "));
	Serial.print(g_forcedDraftMotor.getSpeed());

#ifdef SIMULATION_MODE
	Serial.print(F(", "));
	Serial.print(g_stoveSim.getFuelLoad(), 2);
#endif

	Serial.println();
#endif

	// Alarm is handled in a special way
	if( (temperatureAlarm() != alarm_none) &&
		(m_state != state_alarm) )
	{
#ifdef DEBUG_TEMP_CONTROLLER
		printUptime();
		Serial.println(F("CTempController::processOneSecond() - alarm condition detected"));
#endif
		TAKEACTION(action_alarmConditionOn);
		CHANGESTATE(state_alarm);
	}

	if( (m_state != state_alarm) &&
		(m_fuelWaitTimer.getState() == CMilliTimer::expired) )
	{
#ifdef DEBUG_TEMP_CONTROLLER
		printUptime();
		printState(m_state, false);
		Serial.println(F("CTempController::processOneSecond() - fuel wait timer expired - alarm silenced"));
#endif
		TAKEACTION(action_stopFuelWaitAlarm);
	}

	// Run the state machine
	switch(m_state)
	{
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	// Wait for the flue to heat above MIN_FORCED_DRAFT_TEMP, and then
	// transition to "idle"
	default:
	case state_noFire:

		// Reset the flue temp timer if needed
		if(m_flueTempTimer.getState() != CMilliTimer::notSet)
			m_flueTempTimer.reset();

		// If we got here then we will need to do a
		// cold start
		m_coldStart = true;

		// If the fire is warm enough then go to idle.
		if(m_currentTemp >= MIN_FORCED_DRAFT_TEMP)
		{
#ifdef DEBUG_TEMP_CONTROLLER
				printUptime();
				printState(m_state, false);
				Serial.println(F("CTempController::processOneSecond() - flue warm enough - changing state to state_idle"));
#endif
			TAKEACTION(action_forcedDraftIdle);
			CHANGESTATE(state_idle);
			break;
		}

		// Air boost works in noFire
		if(m_airBoostButtonPressed)
		{
			if(m_currentTemp < g_settings.m_targetRunTemp - TEMP_DYING_FIRE_OFFSET)
			{
#ifdef DEBUG_TEMP_CONTROLLER
				printUptime();
				printState(m_state, false);
				Serial.println(F("CTempController::processOneSecond() - no-fire air boost requested"));
#endif
				TAKEACTION(action_forcedDraftFull);
				TAKEACTION(action_startAirBoostTimer);
				CHANGESTATE(state_airBoost);
				break;
			}
			else
			{
				m_airBoostButtonPressed = false;
			}
		}

		break;	// case state_noFire:

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	// Idle - holding idle temp and waiting for a call for heat
	case state_idle:

		// Should we transition to no-fire?
		if(m_currentTemp < MIN_FORCED_DRAFT_TEMP)
		{
#ifdef DEBUG_TEMP_CONTROLLER
				printUptime();
				printState(m_state, false);
				Serial.println(F("CTempController::processOneSecond() - flue too cool - changing state to state_noFire"));
#endif
			TAKEACTION(action_forcedDraftOff);
			CHANGESTATE(state_noFire);
		}

		// If the flue is hot enough, and the thermostat is calling for heat
		// then enter running mode
		if( (m_currentTemp >= MIN_FORCED_DRAFT_TEMP) && callingForHeat() )
		{
#ifdef DEBUG_TEMP_CONTROLLER
			printUptime();
			printState(m_state, false);
			Serial.println(F("CTempController::processOneSecond() - thermostat calling for heat"));
#endif
			TAKEACTION(action_stopFuelWaitAlarm);
			TAKEACTION(action_forcedDraftRun);
			CHANGESTATE(state_running);
			break;
		}

		// If the flue temp timer times out then we have a dying fire
		if(m_flueTempTimer.getState() == CMilliTimer::expired)
		{
#ifdef DEBUG_TEMP_CONTROLLER
			printUptime();
			printState(m_state, false);
			Serial.println(F("CTempController::processOneSecond() - detected dying fire in idle state"));
#endif
			// Alarm if needed
			if(m_dyingFireAlarmInIdle)
			{
				m_dyingFireAlarmInIdle = false;
				TAKEACTION(action_startFuelWaitAlarm);
			}

			TAKEACTION(action_forcedDraftOff);
		}

		// Is the fire dying in idle mode?
		// If the flue is too cold then set the flue-temperature wait timer.
		// We are watching for the fire to die out while running.
		if(m_currentTemp < (g_settings.m_targetIdleTemp - TEMP_DYING_FIRE_OFFSET) )
		{
			if(m_flueTempTimer.getState() == CMilliTimer::notSet)
			{
				// If this is a cold start then assume that it could take a lot
				// longer to reach operating temperature
				if(m_coldStart)
					m_flueTempTimer.start(MAX_FLUE_TEMP_WAIT_TIME * ONE_SECOND_MS);
				else
					m_flueTempTimer.start(g_settings.m_flueTempWaitTime * ONE_SECOND_MS);
			}
		}
		else
		{
			if(m_flueTempTimer.getState() != CMilliTimer::notSet)
				m_flueTempTimer.reset();

			if(m_fuelWaitTimer.getState() != CMilliTimer::notSet)
				TAKEACTION(action_stopFuelWaitAlarm);
		}

		// Air boost works in idle if the temperature
		// is below the run temp
		if(m_airBoostButtonPressed)
		{
			if(m_currentTemp < (g_settings.m_targetRunTemp - TEMP_DYING_FIRE_OFFSET))
			{
#ifdef DEBUG_TEMP_CONTROLLER
				printUptime();
				printState(m_state, false);
				Serial.println(F("CTempController::processOneSecond() - idle air boost requested"));
#endif
				TAKEACTION(action_forcedDraftFull);
				TAKEACTION(action_startAirBoostTimer);
				CHANGESTATE(state_airBoost);
				break;
			}
			else
			{
				m_airBoostButtonPressed = false;
			}
		}

		break;	// case state_idle:

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	// Running - wait for thermostat to drop heat request,
	// or for the fire to die
	case state_running:

		// If the call for heat is satisfied then stop the forced draft
		// and return to idle
		if(!callingForHeat())
		{
#ifdef DEBUG_TEMP_CONTROLLER
			printUptime();
			printState(m_state, false);
			Serial.println(F("CTempController::processOneSecond() - thermostat no longer calling for heat"));
#endif
			m_flueTempTimer.reset();
			m_dyingFireAlarmInIdle = true;
			TAKEACTION(action_forcedDraftIdle);
			CHANGESTATE(state_idle);
			break;
		}

		// If the temperature timer expires then the fire is dying
		// (or not well started) and we can go to dying-fire state
		if(m_flueTempTimer.getState() == CMilliTimer::expired)
		{
#ifdef DEBUG_TEMP_CONTROLLER
			printUptime();
			printState(m_state, false);
			Serial.println(F("CTempController::processOneSecond() - flue failed to reach temperature - dying fire"));
#endif
			m_flueTempTimer.reset();

			m_dyingFireLowestTemp = m_currentTemp;

			TAKEACTION(action_startFuelWaitAlarm);
			TAKEACTION(action_forcedDraftOff);
			CHANGESTATE(state_dyingFire);
			break;
		}

		// If the flue is too cold then set the flue-temperature wait timer.
		// We are watching for the fire to die out while running.
		if(m_currentTemp < (g_settings.m_targetRunTemp - TEMP_DYING_FIRE_OFFSET))
		{
				// If this is a cold start then assume that it could take a lot
				// longer to reach operating temperature
				if(m_coldStart)
					m_flueTempTimer.start(MAX_FLUE_TEMP_WAIT_TIME * ONE_SECOND_MS);
				else
					m_flueTempTimer.start(g_settings.m_flueTempWaitTime * ONE_SECOND_MS);
		}
		else
		{
			// OK, the fire has been up to operating temperature
			// so this is no longer a cold start condition
			m_coldStart = false;
			if(m_flueTempTimer.getState() != CMilliTimer::notSet)
				m_flueTempTimer.reset();
		}

		break;	// case state_running:

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	// Dying fire - wait for fire to be re-fueled and warm
	// up,or for the fire to die completely
	case state_dyingFire:

		if(m_currentTemp < m_dyingFireLowestTemp)
			m_dyingFireLowestTemp = m_currentTemp;

		// If the flue warms up well above the lowest noted temperature then
		// they must has added fuel, so head for idle (and then running?)
		// Or, if the temp goes so low that the forced draft will not activate
		// then go to idle, and probably stay there.
		if(m_currentTemp > (m_dyingFireLowestTemp + TEMP_DYING_FIRE_OFFSET))
		{
#ifdef DEBUG_TEMP_CONTROLLER
			printUptime();
			printState(m_state, false);
			Serial.println(F("CTempController::processOneSecond() - flue warmed back up - switching to idle"));
#endif

			TAKEACTION(action_stopFuelWaitAlarm);
			TAKEACTION(action_forcedDraftIdle);
			CHANGESTATE(state_idle);
			break;
		}

		if(m_currentTemp < MIN_FORCED_DRAFT_TEMP)
		{
#ifdef DEBUG_TEMP_CONTROLLER
			printUptime();
			printState(m_state, false);
			Serial.println(F("CTempController::processOneSecond() - flue too cool - switching to idle"));

#endif
			TAKEACTION(action_stopFuelWaitAlarm);
			TAKEACTION(action_forcedDraftOff);
			CHANGESTATE(state_noFire);
			break;
		}

		// Air boost always works in dying_fire
		if(m_airBoostButtonPressed)
		{
			if(m_currentTemp < (g_settings.m_targetRunTemp - TEMP_DYING_FIRE_OFFSET))
			{
#ifdef DEBUG_TEMP_CONTROLLER
				printUptime();
				printState(m_state, false);
				Serial.println(F("CTempController::processOneSecond() - dying fire air boost requested"));
#endif
				TAKEACTION(action_forcedDraftFull);
				TAKEACTION(action_startAirBoostTimer);
				CHANGESTATE(state_airBoost);
				break;
			}
			else
			{
				m_airBoostButtonPressed = false;
			}
		}

		break;	// 	case state_dyingFire:

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	// Alarmed - over temp or bad probe, we'll clear the
	// alarm condition when things get better
	case state_alarm:

		if(temperatureAlarm() == alarm_none)
		{
			TAKEACTION(action_alarmConditionOff);
			TAKEACTION(action_forcedDraftIdle);
			CHANGESTATE(state_idle);
			break;
		}

		break;	// case state_alarm:

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	// Air boost - feed air to the fire or a while, or
	// until the fire gets hot enough
	case state_airBoost:

		m_airBoostButtonPressed = false;
		bool endAirBoost = false;

		// It ends on timeout
		if(m_airBoostTimer.getState() == CMilliTimer::expired)
		{
#ifdef DEBUG_TEMP_CONTROLLER
			printUptime();
			printState(m_state, false);
			Serial.println(F("CTempController::processOneSecond() - air boost completed (timeout)"));
#endif
			endAirBoost = true;
		}

		// Or temperature
		if(m_currentTemp >= g_settings.m_targetIdleTemp)
		{
#ifdef DEBUG_TEMP_CONTROLLER
		printUptime();
		printState(m_state, false);
		Serial.println(F("CTempController::processOneSecond() - air boost completed (hit idle temp)"));
#endif
			endAirBoost = true;
		}

		if(endAirBoost)
		{

			m_airBoostTimer.reset();
			TAKEACTION(action_forcedDraftOff);	// Dump the integrator
			TAKEACTION(action_forcedDraftIdle);
			CHANGESTATE(state_idle);
			break;
		}

		break;	// case state_airBoost:
	}
}

////////////////////////////////////////////////////////////
// There was some kind of settings change so
// update the PID
void CTempController::updateSettings()
{
#ifdef DEBUG_TEMP_CONTROLLER
	printUptime();
	Serial.println(F("CTempController::updateSettings() - loading settings"));
#endif

	if( (m_pid.GetKp() != g_settings.m_Kp) ||
		(m_pid.GetKi() != g_settings.m_Ki) ||
		(m_pid.GetKd() != g_settings.m_Kd) )
	{
		m_pid.SetTunings(g_settings.m_Kp, g_settings.m_Ki, g_settings.m_Kd);
	}

	if(m_state == state_idle)
		m_pid.SetSetpoint(g_settings.m_targetIdleTemp);

	if(m_state == state_running)
		m_pid.SetSetpoint(g_settings.m_targetRunTemp);
}

bool CTempController::callingForHeat()
{
#ifdef SUMULATION_MODE_CALL_FOR_HEAT
	return true;
#endif

	if(digitalRead(PIN_CALL_FOR_HEAT) == 0)
		return true;

	return false;
}

int CTempController::getTargetTemp()
{
	if(m_state == state_idle)
		return g_settings.m_targetIdleTemp;

	if(m_state == state_running)
		return g_settings.m_targetRunTemp;

	return THERMOCOUPLE_INVALID_TEMP;
}

////////////////////////////////////////////////////////////
// Varous logging / debugging stuff
void CTempController::changeState(int _lineNo, CTempController_stateE _newState)
{
#ifdef DEBUG_TEMP_CONTROLLER
	printUptime();
	Serial.print(F("CTempController::changeState() - Line #: "));
	Serial.print(_lineNo);
	Serial.print(F(" new state: "));
	printState(_newState, true);

#else
	UNUSED(_lineNo);
#endif

	// Change the state
	m_state = _newState;
}

void CTempController::takeAction(int _lineNo, CTempController_actionE _action)
{
#ifdef DEBUG_TEMP_CONTROLLER
	printUptime();
	Serial.print(F("CTempController::takeAction() - Line #: "));
	Serial.print(_lineNo);
	Serial.print(F(" action: "));
	Serial.println((_action == action_none) ? F("action_none") :
				   (_action == action_forcedDraftOff) ? F("action_forcedDraftOff") :
				   (_action == action_forcedDraftIdle) ? F("action_forcedDraftIdle") :
				   (_action == action_forcedDraftRun) ? F("action_forcedDraftRun") :
				   (_action == action_forcedDraftFull) ? F("action_forcedDraftFull") :
				   (_action == action_startAirBoostTimer) ? F("action_startAirBoostTimer") :
				   (_action == action_startFuelWaitAlarm) ? F("action_startFuelWaitAlarm") :
				   (_action == action_stopFuelWaitAlarm) ? F("action_stopFuelWaitAlarm") :
				   (_action == action_alarmConditionOn) ? F("action_alarmConditionOn") :
				   (_action == action_alarmConditionOff) ? F("action_alarmConditionOff") :
				   (F("*** ERROR UNKNOWN ***")));
#else
	UNUSED(_lineNo);
#endif

	// Take the action
	switch(_action)
	{
	default:
	case action_none:
		break;

	case action_forcedDraftOff:
		m_pid.SetMode(MANUAL);
		m_pid.SetOutput(PWM_MOTOR_STOP);
		break;

	case action_forcedDraftIdle:
		m_pid.SetMode(AUTOMATIC);
		m_pid.SetSetpoint(g_settings.m_targetIdleTemp);
		break;

	case action_forcedDraftRun:
		m_pid.SetMode(AUTOMATIC);
		m_pid.SetSetpoint(g_settings.m_targetRunTemp);
		break;

	case action_forcedDraftFull:
		m_pid.SetMode(MANUAL);
		m_pid.SetOutput(PWM_MOTOR_MAX_COMMAND);
		break;

	case action_startAirBoostTimer:
		if(m_airBoostTimer.getState() == CMilliTimer::notSet)
			m_airBoostTimer.start(DEF_FORCED_DRAFT_BOOST_TIME * ONE_SECOND_MS);
		break;

	case action_startFuelWaitAlarm:
		m_fuelWaitTimer.start(DEF_ADD_FUEL_BEEP_TIME * ONE_SECOND_MS);
		g_beeper.beep(BEEPER_ADD_FUEL_ON_TIME, BEEPER_ADD_FUEL_OFF_TIME);
		break;

	case action_stopFuelWaitAlarm:
		m_fuelWaitTimer.reset();
		g_beeper.off();
		break;

	case action_alarmConditionOn:
		m_pid.SetMode(MANUAL);
		m_pid.SetOutput(PWM_MOTOR_STOP);
		g_fanController.forceFanOn(true);
		g_beeper.beep(BEEPER_ALARM_ON_TIME, BEEPER_ALARM_OFF_TIME);
		break;

	case action_alarmConditionOff:
		g_beeper.beep(0, 0);
		g_fanController.forceFanOn(false);
		break;
	}
}

CTempController::CTempController_alarmE CTempController::temperatureAlarm()
{
	// Make sure the thermocouple is ok
	if(m_currentTemp == THERMOCOUPLE_INVALID_TEMP)
		return alarm_badProbe;

	// And the flue temp is not too high
	if(m_currentTemp >= g_settings.m_alarmFlueTemp)
		return alarm_overTemp;

	return alarm_none;
}

void CTempController::printState(CTempController_stateE _state, bool _lf)
{
	Serial.print((_state == state_noFire) ? F("state_noFire") :
				 (_state == state_idle) ? F("state_idle") :
				 (_state == state_running) ? F("state_running") :
				 (_state == state_dyingFire) ? F("state_dyingFire") :
				 (_state == state_alarm) ? F("state_alarm") :
				 (_state == state_airBoost) ? F("state_airBoost") :
				 (F("*** ERROR UNKNOWN ***")));
	if(_lf)
		Serial.println();
	else
		Serial.print(F(" "));
}

