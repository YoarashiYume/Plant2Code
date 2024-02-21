#pragma once
#include "SignalType.hpp"
#include <string>
#include <vector>


/*
///\brief - ������� ���, �������� ��������� ���������� � �������
*/
struct RawReadData
{
	RAW_SIGNAL_TYPE signalType;			///< - ��� �������
	std::string name;					///< - �������� ����������
	std::string title;					///< - �������� �������
	virtual ~RawReadData() = default;
};

/*
///\brief - ���, �������� ��������� ���������� � �������
*/
using RawTimerData = RawReadData;

/*
///\brief - ���, �������� ��������� ���������� � ���������
*/
struct RawConstData : RawReadData
{
	std::string value;					///< - �������� ���������
};
/*
///\brief - ���, �������� ��������� ���������� � �������
*/
struct RawSignalData : RawReadData
{
	std::string type;					///< - ��� �������
	virtual ~RawSignalData() = default;
};
