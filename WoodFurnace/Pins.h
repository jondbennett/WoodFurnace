#ifndef Pins_h
#define Pins_h


// Blast the forced draft motor for
// a while to get the fire going
#define PIN_FD_BLAST			(2)
#define PIN_FD_BLAST_C			(3)

// Please stop beeping button
#define PIN_MUTE_ALARM			(4)
#define PIN_MUTE_ALARM_C		(5)

// Forced draft blower pin for PWM
#define PIN_FORCED_DRAFT_GND	(8)
#define PIN_FORCED_DRAFT_PWM	(9)


// Thermocouple board IO
#define PIN_THERMOCOUPLE_SO		(10)
#define PIN_THERMOCOUPLE_CS		(11)
#define PIN_THERMOCOUPLE_SCK	(12)
#define PIN_THERMOCOUPLE_VCC	(13)
//#define PIN_THERMOCOUPLE_GND	(XX)


// Circulation fan relay (big fan)
#define PIN_FAN_RELAY			(A0)

// "Please add wood" beeper
#define PIN_BEEPER_RELAY		(A1)

// Call for fan input
#define PIN_CALL_FOR_FAN		(A2)

// Call for heat input
#define PIN_CALL_FOR_HEAT		(A3)

#endif



