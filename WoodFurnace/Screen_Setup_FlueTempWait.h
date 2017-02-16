////////////////////////////////////////////////////////////
// Set flue temp wait time
////////////////////////////////////////////////////////////
#ifndef Screen_Setup_FlueTempWait_h
#define Screen_Setup_FlueTempWait_h

////////////////////////////////////////////////////////////
// ???
////////////////////////////////////////////////////////////
class CScreen_Setup_FlueTempWait : public CScreen_Base
{
protected:

	void updateStatics();
	void updateDynamics();

	CMilliTimer m_buttonTimer;

public:

	CScreen_Setup_FlueTempWait(int _id);
	virtual ~CScreen_Setup_FlueTempWait();

	void init();

	void buttonCheck(CButtonController &_buttons);
	void processOneSecond();
};

#endif
