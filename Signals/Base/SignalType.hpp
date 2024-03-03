#pragma once

#include <cstdint>
/*
///\brief - Типы считываемых сигналов
*/
enum class RAW_SIGNAL_TYPE : std::uint8_t
{
	UNKMOWN,			///< - Неизвестный сигнал
	INPUT_SIGNAL,		///< - Входной сигнал
	OUTPUT_SIGNAL,		///< - Выходной сигнал
	INNER_SIGNAL,		///< - Внутренний сигнал
	TIMER,				///< - Таймер
	CONST,				///< - Константа
	STRUCT				///< - Структура
};