////////////////////////////////////////////////////////////
// WSPID wrapper around PID_v1
////////////////////////////////////////////////////////////

#include <Arduino.h>

#include <PID_v1.h>

#include "WSPID.h"

////////////////////////////////////////////////////////////
// Make PID_v1 more acceptable to my stylistic tastes, and
// add scaling to make it easier to match the scale of the
// to setpoint to the range of the control output.
////////////////////////////////////////////////////////////
CWSPID::CWSPID()
{
	m_input = m_output = m_setpoint = 0.;
	m_scale = 1.;

	// Create the PID controller
	m_pid = new PID(&m_input, &m_output, &m_setpoint, 0., 0., 0., DIRECT);
}

CWSPID::~CWSPID()
{
	delete m_pid;
	m_pid = 0;
}

void CWSPID::SetMode(int Mode)
{
	m_pid->SetMode(Mode);
}

int CWSPID::GetMode()
{
	return m_pid->GetMode();
}

void CWSPID::SetOutputLimits(double Min, double Max)
{
	m_pid->SetOutputLimits(Min, Max);
}

void CWSPID::SetSampleTime(int NewSampleTime)
{
	m_pid->SetSampleTime(NewSampleTime);
}

void CWSPID::SetTunings(double Kp, double Ki, double Kd)
{
	m_pid->SetTunings(Kp, Ki, Kd);
}

double CWSPID::GetKp()
{
	return m_pid->GetKp();
}

double CWSPID::GetKi()
{
	return m_pid->GetKi();
}

double CWSPID::GetKd()
{
	return m_pid->GetKd();
}

void CWSPID::SetSetpoint(double _setpoint)
{
	m_setpoint = _setpoint * m_scale;
}

double CWSPID::Compute(double _input)
{
	m_input = _input * m_scale;

	m_pid->Compute();

	return m_output;
}

void CWSPID::SetScale(double _scale)
{
	m_scale = _scale;
}

void CWSPID::SetOutput(double _o)
{
	if(GetMode() == AUTOMATIC)
		return;

	m_output = _o;
}
