#pragma once
#include "../../../Signals/Base/RawSignalData.hpp"
#include <unordered_map>
/*
///\brief - Тип, хранящий считанную информацию о сигнале
*/
struct SignalData : RawSignalData
{
	using element_count_type = std::uint32_t;
	bool isPtr;								///< - Является ли сигнал указателем
	element_count_type elementCount;		///< - Кол-во элементов
};
/*
///\brief - Тип, хранящий считанную информацию о структурах
*/
struct StructData : RawSignalData
{
	std::unordered_map<std::string, std::shared_ptr<SignalData>> fields;	///< - Данные о полях структуры
};
