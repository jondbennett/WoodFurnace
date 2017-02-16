////////////////////////////////////////////////////
// Thermocouple temperature sensor based on MAX6675
////////////////////////////////////////////////////
#ifndef TempSensor_Thermocouple_H
#define TempSensor_Thermocouple_H

#define THERMOCOUPLE_EMA_ALPHA		(0.3)	// Exponential moving average
#define THERMOCOUPLE_INVALID_TEMP	(-461)

class CTempSensor_Thermocouple
{
private:
	// Instance Vars
	void *m_thermocoupleInterface;
	unsigned long m_nextReadTime;
	int m_temp;

	// Private Methods
	void updateTemp();

public:
	// 'structors
	CTempSensor_Thermocouple();
	virtual ~CTempSensor_Thermocouple();

	// Prep for operation
	void init();

	// Operate
	int temperature();
};

#endif

