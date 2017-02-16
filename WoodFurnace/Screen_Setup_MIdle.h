////////////////////////////////////////////////////////////
// Manual Idle
////////////////////////////////////////////////////////////
#ifndef Screen_Setup_MIdle_h
#define Screen_Setup_MIdle_h

////////////////////////////////////////////////////////////
// Setup forced draft fixed (non-PID) in idle
////////////////////////////////////////////////////////////
class CScreen_Setup_MIdle : public CScreen_Base
{
	protected:

	void updateStatics();
	void updateDynamics();
	void updateValue();

	CMilliTimer m_buttonTimer;

	public:

	CScreen_Setup_MIdle(int _id);
	virtual ~CScreen_Setup_MIdle();

	void init();

	void buttonCheck(CButtonController &_buttons);
	void processOneSecond();
};

#endif
