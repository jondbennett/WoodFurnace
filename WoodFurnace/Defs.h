#ifndef Defs_h
#define Defs_h

/////////////////////////////////////////////
// Used from time to time...
#define ONE_SECOND_MS					(1000L)
#define UNUSED(x) ((void)(x))

/////////////////////////////////////////////
// Data version for saving and loading the EEPROM
#define WOODSTOVE_DATA_VERSION	(1)

/////////////////////////////////////////////
// Flue temps and limits.
#define MIN_FLUE_TEMP_IDLE			(125)	// It won't target a flue idle temp below this
#define DEF_FLUE_TEMP_IDLE			(150)	// Default target flue idle temp
#define MAX_FLUE_TEMP_IDLE			(175)	// It won't target a flue idle temp above this

#define MIN_FLUE_TEMP_RUN			(325)	// It won't target a flue temp below this
#define DEF_FLUE_TEMP_RUN			(350) 	// Default target flue temperature (F)
#define MAX_FLUE_TEMP_RUN			(475)	// It won't target a flue temp above this

#define MIN_FLUE_TEMP_ALARM			(400)	// If the flue temp goes above this
#define DEF_FLUE_TEMP_ALARM			(475)	// value, enter special processing to
#define MAX_FLUE_TEMP_ALARM			(600)	// activate alarm and cool things off

#define MIN_FLUE_TEMP_WAIT_TIME		(60L)	// Wow, that would be fast
#define DEF_FLUE_TEMP_WAIT_TIME		(120L)	// Seconds to wait for flue to come to target temp with FD blower on.
#define MAX_FLUE_TEMP_WAIT_TIME		(300L)	// five minutes should be plenty

#define TEMP_DYING_FIRE_OFFSET		(10)	// When we are trying to hold a flue temp, and the flue stays
											// this far *below* target for flue_temp_wait_time then the
											// fire must be dying. The "feed me" alarm may be triggered

/////////////////////////////////////////////
// Forced draft controls
#define MIN_FORCED_DRAFT_TEMP			(100)	// As flue temp. The FDB will not come on below this temperature
#define DEF_FORCED_DRAFT_BOOST_TIME		(60L)	// Seconds

/////////////////////////////////////////////
// Forced air *circulation* fan controls (expressed as flue temperatures)
#define MIN_FAN_ON_TEMP		(150)
#define DEF_FAN_ON_TEMP		(250)	// Temp above which the fan is turned on
#define MAX_FAN_ON_TEMP		(350)

#define MIN_FAN_OFF_TEMP	(125)
#define DEF_FAN_OFF_TEMP	(200)	// Temp below which the fan is turned off
#define MAX_FAN_OFF_TEMP	(350)

#define MIN_FAN_HYSTERESIS	(25)	// Min temperature between fan on and fan off

/////////////////////////////////////////////
// Empirical limits of the forced draft blower motor
#define PWM_MOTOR_STARTUP_TIME	(500L)				// Time needed to get the motor running (ms)
#define PWM_MOTOR_STOP			(0)					// Stops the motor
#define PWM_MOTOR_MIN_COMMAND	(3)					// Lowest command that will keep the motor running
#define PWM_MOTOR_MAX_COMMAND	(215)				// Largest PWM command (fan going as fast as possible)

// Coefs for the flue temp PID
#define DEF_KP	(0.0)
#define DEF_KI	(0.0)
#define DEF_KD	(0.0)

/////////////////////////////////////////////
// Beeper on/off times for add-fuel and
// alarm notifications
#define BEEPER_ADD_FUEL_ON_TIME		(1L)		// Seconds
#define BEEPER_ADD_FUEL_OFF_TIME	(60L - BEEPER_ADD_FUEL_ON_TIME)

// How long to beg for fuel to be added
#define DEF_ADD_FUEL_BEEP_TIME		(5L * 60L)	// Five minutes should do it

#define BEEPER_ALARM_ON_TIME		(5L)		// Seconds
#define BEEPER_ALARM_OFF_TIME		(30L - BEEPER_ALARM_ON_TIME)
#define BEEPER_ALARM_MUTE_TIME		(5L * 60L)	// Five minutes should do it

/////////////////////////////////////////////
// Screen IDs (each must be a unique number)
#define SCREEN_ID_NORMAL				(1)
#define SCREEN_ID_SETUP_FLUE_TEMP		(2)
#define SCREEN_ID_SETUP_FLUE_TEMP_WAIT	(3)
#define SCREEN_ID_SETUP_FAN_TEMPS		(4)
#define SCREEN_ID_SETUP_MIDLE			(5)
#define SCREEN_ID_SETUP_PID				(6)

/////////////////////////////////////////////
// How long to hold various buttons (in MS)
#define SETUP_TIME_ENTER_SETUP	(2000L)	// Normal screen, press & hold select to enter setup mode
#define SETUP_TIME_BUMP			(750L)	// how long to hold a +/- key to move by larger units
#define HOLD_TIME_SYSTEM_RESET	(5000L)	// how long to hold the left key (normal screen) to reset to defaults

/////////////////////////////////////////////
// Dang relay board is backwards
#define RELAY_ON	(0)
#define RELAY_OFF	(1)

/////////////////////////////////////////////
// Log and plot info for setting up the PID
//#define SERIAL_LOG				// Logging info to serial port (radio?)
#define SERIAL_PLOT					// Info for the plotter

/////////////////////////////////////////////
// Debug Settings
//#define DEBUG_INO
//#define DEBUG_SETTINGS
//#define DEBUG_FAN_CONTROLLER
//#define DEBUG_TEMP_CONTROLLER
//#define DEBUG_PWM_MOTOR
//#define DEBUG_BEEPER
//#define DEBUG_TEMPSENSOR

//#define DEBUG_SCREEN_CONTROLLER
//#define DEBUG_SCREEN_NORMAL
//#define DEBUG_SCREEN_SETUP_FLUE_TEMP
//#define DEBUG_SCREEN_SETUP_FLUE_TEMP_WAIT
//#define DEBUG_SCREEN_SETUP_DRAFT
//#define DEBUG_SCREEN_SETUP_FAN
//#define DEBUG_SCREEN_MIDLE
//#define DEBUG_SCREEN_SETUP_PID

// Print uptime in seconds
extern void printUptime(bool _colonSpace = true);

/////////////////////////////////////////////
// Simulation
//#define SIMULATION_MODE
//#define SUMULATION_MODE_CALL_FOR_HEAT
//#define DEBUG_SIMULATOR
#endif
