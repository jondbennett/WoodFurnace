////////////////////////////////////////////////////////////
// Set target flue temp
////////////////////////////////////////////////////////////
#ifndef Screen_Setup_FlueTemp_h
#define Screen_Setup_FlueTemp_h

////////////////////////////////////////////////////////////
// ???
////////////////////////////////////////////////////////////
class CScreen_Setup_FlueTemp : public CScreen_Base
{
protected:

	typedef enum
	{
		field_idleTemp = 0,
		field_runTemp,
		field_alarmTemp,
	} CScreen_Setup_FlueTemp_FieldE;
	CScreen_Setup_FlueTemp_FieldE m_field;

	void updateStatics();
	void updateDynamics();

	CMilliTimer m_buttonTimer;

public:

	CScreen_Setup_FlueTemp(int _id);
	virtual ~CScreen_Setup_FlueTemp();

	void init();

	void buttonCheck(CButtonController &_buttons);
	void processOneSecond();
};

#endif
