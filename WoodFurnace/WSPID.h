////////////////////////////////////////////////////////////
// WSPID wrapper around PID_v1
////////////////////////////////////////////////////////////
#ifndef WSPID_h
#define WSPID_h

////////////////////////////////////////////////////////////
// Make PID_v1 more acceptable to my stylistic tastes, and
// add scaling to make it easier to match the scale of the
// to setpoint to the range of the control output.
////////////////////////////////////////////////////////////
class CWSPID
{
protected:

	double m_input;
	double m_output;
	double m_setpoint;

	double m_scale;

	PID *m_pid;

public:
	CWSPID();
	virtual ~CWSPID();

	void SetMode(int Mode);
	int GetMode();

	void SetOutputLimits(double Min, double Max);
	void SetSampleTime(int NewSampleTime);
	void SetTunings(double Kp, double Ki, double Kd);

	double GetKp();
	double GetKi();
	double GetKd();

	void SetSetpoint(double _setpoint);

	double Compute(double _input);
	void SetScale(double _scale);

	void SetOutput(double _o);
};

#endif
