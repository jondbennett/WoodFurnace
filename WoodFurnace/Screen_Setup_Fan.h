////////////////////////////////////////////////////////////
// Fan Screen
////////////////////////////////////////////////////////////
#ifndef Screen_Setup_Fan_h
#define Screen_Setup_Fan_h

////////////////////////////////////////////////////////////
// Setup the circulation fan controller
////////////////////////////////////////////////////////////
class CScreen_Setup_Fan : public CScreen_Base
{
protected:

	typedef enum
	{
		field_fan_on_temp = 0,
		field_fan_off_temp,
	} CScreen_Setup_Fan_FieldE;
	CScreen_Setup_Fan_FieldE m_field;	// Which field (fan-on-temp or fan-off-temp are we editing?

	CMilliTimer m_buttonTimer;

	void updateStatics();
	void updateDynamics();

public:

	CScreen_Setup_Fan(int _id);
	virtual ~CScreen_Setup_Fan();

	void init();

	void buttonCheck(CButtonController &_buttons);
	void processOneSecond();
};

#endif
