////////////////////////////////////////////////////////////
// Screen Controller
////////////////////////////////////////////////////////////
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#include "Pins.h"
#include "Defs.h"
#include "ScreenController.h"

////////////////////////////////////////////////////////////
// This object simply dispatches it's process calls
// to the appropriate screen object
////////////////////////////////////////////////////////////
#define CScreenController_Invalid_ScreenID (-1)

extern Adafruit_RGBLCDShield g_lcd;

CScreenController::CScreenController()
{
}

CScreenController::~CScreenController()
{
}

void CScreenController::setup()
{
#ifdef DEBUG_SCREEN_CONTROLLER
	printUptime();
	Serial.println(F("CScreenController::setup()"));
#endif

	m_screenID = CScreenController_Invalid_ScreenID;
	for(int _ = 0; _ < CScreenController_MAX_SCREENS; _++)
		m_screenList[_] = 0;

	m_buttonController.setup();
}

void CScreenController::addScreen(CScreen_Base *_screen)
{
#ifdef DEBUG_SCREEN_CONTROLLER
	printUptime();
	Serial.print(F("CScreenController::addScreen: "));
	Serial.println(_screen->getID());
#endif

	for(int _ = 0; _ < CScreenController_MAX_SCREENS; _++)
	{
		if(m_screenList[_] == 0)
		{
			m_screenList[_] = _screen;
			break;
		}
	}
}

CScreen_Base *CScreenController::findScreen(int _screenID)
{
	if(_screenID < 0)
		return 0;

	for(int _ = 0; _ < CScreenController_MAX_SCREENS; _++)
		if(m_screenList[_] && m_screenList[_]->getID() == _screenID)
			return m_screenList[_];

	return 0;
}

void CScreenController::setScreen(int _screen)
{
	// Don't keep resetting the screen
	if(m_screenID == _screen)
		return;

	if(!findScreen(_screen))
	{
#ifdef DEBUG_SCREEN_CONTROLLER
		printUptime();
		Serial.print(F("CScreenController::setScreen ** Screen not found: "));
		Serial.println(_screen);
#endif
		return;
	}


#ifdef DEBUG_SCREEN_CONTROLLER
	printUptime();
	Serial.print(F("CScreenController::setScreen: "));
	Serial.println(_screen);
#endif

	// And do an immediate update
	m_screenID = _screen;
	findScreen(m_screenID)->init();
	m_buttonController.maskButtonsUntilClear();
}

void CScreenController::processFast()
{
	m_buttonController.processFast();

	CScreen_Base *screen = findScreen(m_screenID);
	if(screen)
		screen->buttonCheck(m_buttonController);
}

void CScreenController::processOneSecond()
{
	CScreen_Base *screen = findScreen(m_screenID);
	if(screen)
	{
#ifdef DEBUG_SCREEN_CONTROLLER
		printUptime();
		Serial.print(F("CScreenController::processOneSecond: "));
		Serial.println(screen->getID());
#endif
		screen->processOneSecond();
	}
}

////////////////////////////////////////////////////////////
// Button Controller
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Read the buttons from the keyboard and monitor how
// long each button has been pressed.
//
// NOTE: This object uses the buttons build into the
// LCD Shield
////////////////////////////////////////////////////////////
/*
#define UPKEY_ARV (144)		 //that's read "analogue read value"
#define DOWNKEY_ARV (329)
#define LEFTKEY_ARV (505)
#define RIGHTKEY_ARV (0)
#define SELKEY_ARV (742)
#define NOKEY_ARV (1023)
#define KEY_THRESHOLD (5)

static int rawReadButtons()
{
	int rawAnalog = analogRead(PIN_ANALOG_KEYPAD);

	if (rawAnalog > UPKEY_ARV - KEY_THRESHOLD && rawAnalog < UPKEY_ARV + KEY_THRESHOLD ) return BC_BUTTON_UP;
	else if (rawAnalog > DOWNKEY_ARV - KEY_THRESHOLD && rawAnalog < DOWNKEY_ARV + KEY_THRESHOLD )return BC_BUTTON_DOWN;
	else if (rawAnalog > RIGHTKEY_ARV - KEY_THRESHOLD && rawAnalog < RIGHTKEY_ARV + KEY_THRESHOLD ) return BC_BUTTON_RIGHT;
	else if (rawAnalog > LEFTKEY_ARV - KEY_THRESHOLD && rawAnalog < LEFTKEY_ARV + KEY_THRESHOLD ) return BC_BUTTON_LEFT;
	else if (rawAnalog > SELKEY_ARV - KEY_THRESHOLD && rawAnalog < SELKEY_ARV + KEY_THRESHOLD ) return BC_BUTTON_SELECT;

	return BC_BUTTON_NONE;
}
*/

static int rawReadButtons()
{
	uint8_t rawButtons = g_lcd.readButtons();

	if(rawButtons & BUTTON_UP)
		return BC_BUTTON_UP;

	if(rawButtons & BUTTON_DOWN)
		return BC_BUTTON_DOWN;

	if(rawButtons & BUTTON_LEFT)
		return BC_BUTTON_LEFT;

	if(rawButtons & BUTTON_RIGHT)
		return BC_BUTTON_RIGHT;

	if(rawButtons & BUTTON_SELECT)
		return BC_BUTTON_SELECT;

	return BC_BUTTON_NONE;
}


CButtonController::CButtonController()
{
	// Clear all of the on times
	for(int _ = 0; _ < BC_NBUTTONS; ++_)
		m_buttons[_] = 0;

	// Clear the readings
	m_buttonRead1 = BC_BUTTON_NONE;
	m_buttonRead2 = BC_BUTTON_NONE;

	// Set our state to the start
	m_state = state_1;

	m_maskButtonsUntilClear = false;
}

CButtonController::~CButtonController()
{
}

void CButtonController::setup()
{
}

void CButtonController::processFast()
{
	unsigned long curMillies = millis();

	// Read the raw button values in such a way
	// that the buttons on the shield, and the digital
	// input buttons can both work
	int rawButtons = rawReadButtons();

	// is this the first read of this cycle?
	if(m_state == state_1)
	{
		m_buttonRead1 = rawButtons;
		m_buttonRead2 = 0;

		m_debounceTime = curMillies + BC_DEBOUNCE_DELAY;
		m_state = state_2;
		return;
	}

	// Second read of the cycle
	if(m_state == state_2)
	{
		// Have we waited long enough?
		if(curMillies < m_debounceTime)
			return;

		// Read them again
		m_buttonRead2 = rawButtons;
		m_state = state_1;
	}

	if((m_buttonRead1 > BC_BUTTON_NONE) &&
	   (m_buttonRead2 > BC_BUTTON_NONE) &&
	   (m_buttonRead1 == m_buttonRead2))
	{
		// A button is pressed, so sets it "on time". DO NOT reset it
		if(m_buttons[m_buttonRead2] == 0)
			m_buttons[m_buttonRead2] = curMillies;
	}
	else
	{
		// Nothing pressed so clear all of the on times
		for(int _ = 0; _ < BC_NBUTTONS; ++_)
			m_buttons[_] = 0;

		if(m_maskButtonsUntilClear)
		{
#ifdef DEBUG_SCREEN_CONTROLLER
			printUptime();
			Serial.println(F("CButtonController::processFast() - unmasking buttons"));
#endif
			m_maskButtonsUntilClear = false;
		}
	}
}

// Return ms since button pressed - 0 = not pressed
unsigned long CButtonController::getButton(int _buttonID)
{
	if(m_maskButtonsUntilClear)
		return 0;

	if((_buttonID >= 0) && (_buttonID < BC_NBUTTONS))
	{
		if(m_buttons[_buttonID] > 0)
			return millis() - m_buttons[_buttonID];
		else
			return 0;
	}

	return 0;
}

bool CButtonController::anyButtonsPressed()
{
	// Scan the on times
	for(int _ = 0; _ < BC_NBUTTONS; ++_)
	{
		// If the button is pressed then we're done
		if(m_buttons[_] > 0)
			return true;
	}

	// no buttons pressed
	return false;
}

void CButtonController::maskButtonsUntilClear()
{
	m_maskButtonsUntilClear = true;
#ifdef DEBUG_SCREEN_CONTROLLER
	printUptime();
	Serial.println(F("CButtonController::maskButtonsUntilClear() - masking buttons"));
#endif
}
