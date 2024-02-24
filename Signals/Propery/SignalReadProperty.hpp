#pragma once
#include <string>
#include <vector>

struct PrefixData
{
	using prefix_variant_type = std::vector<std::string>;
	using ignore_variant_type = std::vector<std::string>;
	prefix_variant_type inputSignalPrefix;		///< - Префиксы входных сигналов
	prefix_variant_type outputSignalPrefix;		///< - Префиксы выходных сигналов
	prefix_variant_type innerSignalPrefix;		///< - Префиксы внутренних сигналов
	prefix_variant_type constPrefix;			///< - Префиксы констант
	prefix_variant_type timerSignalPrefix;		///< - Префиксы таймеров
	prefix_variant_type structSignalPrefix;		///< - Префиксы структур
	ignore_variant_type ignoreVariant;			///< - Название с которыми сигналы игнорируются
};

/*
///\brief - Структура, задающая свойства для чтения сигнала.
*/
struct SignalReadProperty
{
	using col_variant_type = std::vector<std::string>;
	using prefix_type = PrefixData;

	char separatorCSV;							///< - Символ разделения для столбцов CSV-таблицы
	col_variant_type variablesColNameVariant;	///< - Варианты заголовков колонок для названия переменных
	col_variant_type variablesColTitleVariant;	///< - Варианты заголовков колонок для названия сигнала
	col_variant_type variablesColTypeVariant;	///< - Варианты заголовков колонок для типа переменных
	col_variant_type variablesColValueVariant;	///< - Варианты заголовков колонок для значения переменных
	

	prefix_type prefixes;						///< - Префиксы сигналов
};