////////////////////////////////////////////////////////////
// Normal Screen
////////////////////////////////////////////////////////////
#ifndef SCREEN_NORMAL_H
#define SCREEN_NORMAL_H

////////////////////////////////////////////////////////////
// Display target and actual flue temperatures
////////////////////////////////////////////////////////////
class CScreen_Normal : public CScreen_Base
{
protected:

	int m_lastTargetTemp;
	int m_lastFlueTemp;
	int m_lastForcedDraftPercent;

	bool m_showingAlarm;

	void updateStatics();
	void updateDynamics();

public:

	CScreen_Normal(int _id);
	virtual ~CScreen_Normal();

	void init();

	void buttonCheck(CButtonController &_buttons);
	void processOneSecond();
};

#endif
