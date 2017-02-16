////////////////////////////////////////////////////////////
// Setup PID coefs
////////////////////////////////////////////////////////////
#ifndef Screen_Setup_PID_h
#define Screen_Setup_PID_h

////////////////////////////////////////////////////////////
// Display PID control values
////////////////////////////////////////////////////////////
class CScreen_Setup_PID: public CScreen_Base
{
protected:

	typedef enum
	{
		field_PID_Kp = 0,
		field_PID_Ki,
		field_PID_Kd,
	} CScreen_Setup_PID_FieldE;
	CScreen_Setup_PID_FieldE m_field;	// Which field are we editing?

	CMilliTimer m_buttonTimer;

	void updateStatics();
	void updateDynamics();
	void updatePTInfo();
public:

	CScreen_Setup_PID(int _id);
	virtual ~CScreen_Setup_PID();

	void init();

	void buttonCheck(CButtonController &_buttons);
	void processOneSecond();
};

#endif
