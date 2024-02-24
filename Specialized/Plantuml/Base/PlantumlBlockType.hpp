#pragma once
#include <cstdint>
/**
///\brief Перечисление, представляющее типы блоков PlantUML
*/
enum class PLANTUML_BLOCK_TYPE : std::uint8_t
{
	UNKNOWN = 0,				///< Неизвестный тип блока
	COMMENT,					///< Комментарий
	CALCULATION,				///< Вычисление
	FUNCTION,					///< Функция
	START_REFERENCE,			///< Начало алгоритма
	REFERENCE,					///< Переход к алгоритму
	END_REFERENCE,				///< Конец алгоритма
	IF_START,					///< Начало условного оператора if
	IF_ELSE_START,				///< Начало ветви if-else
	IF_END,						///< Конец условного оператора if
	ELSE_START,					///< Начало ветви else
	ELSE_END,					///< Конец ветви else
	WHILE_START,				///< Начало цикла while
	WHILE_END,					///< Конец цикла while
	SWITCH_START,				///< Начало оператора switch
	SWITCH_END,					///< Конец оператора switch
	CASE_START,					///< Начало блока case в операторе switch
	CASE_END					///< Конец блока case в операторе switch
};