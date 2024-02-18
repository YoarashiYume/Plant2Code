#pragma once
#include "SignalType.hpp"
#include <string>
#include <vector>


/*
///\brief - базовый тип, хранящий считанную информацию о сигнале
*/
struct RawReadData
{
	RAW_SIGNAL_TYPE signalType;			///< - Тип сигнала
	std::string name;					///< - Название переменной
	std::string title;					///< - Название сигнала
	virtual ~RawReadData() = default;
};

/*
///\brief - Тип, хранящий считанную информацию о таймере
*/
using RawTimerData = RawReadData;

/*
///\brief - Тип, хранящий считанную информацию о константе
*/
struct RawConstData : RawReadData
{
	std::string value;					///< - Значение константы
};
/*
///\brief - Тип, хранящий считанную информацию о сигнале
*/
struct RawSignalData : RawReadData
{
	std::string type;					///< - Тип сигнала
	virtual ~RawSignalData() = default;
};
