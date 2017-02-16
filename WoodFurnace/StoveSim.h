////////////////////////////////////////////////////////////
// Naive Wood stove simulator
////////////////////////////////////////////////////////////
#ifndef StoveSim_h
#define StoveSim_h

////////////////////////////////////////////////////////////
// For testing the wood stove controller
////////////////////////////////////////////////////////////
#define STOVE_SIM_DELAY_TIME				(30)	// Seconds

class CStoveSim
{
protected:
	double m_fuelLoad;		// Oz
	double m_curTemp;		// Degrees F
	double m_temps[STOVE_SIM_DELAY_TIME];

	int m_forcedDraftSpeed;		// 0 - 255

	int m_idleTemp;

public:
	CStoveSim();
	virtual ~CStoveSim();

	void processOneSecond();

	void setForcedDraftBlower(int _speed);
	int getPWM();
	int getTemperature();
	double getFuelLoad();

	void bumpToMinForcedDraftTemp();
};

#endif
