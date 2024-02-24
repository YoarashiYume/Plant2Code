#pragma once
#include "PlantumlBlockType.hpp"
#include <string>

/**
///\Структура, представляющая блок
*/
struct PlantumlBlock
{
	PLANTUML_BLOCK_TYPE type;						///< Тип блока
	virtual ~PlantumlBlock() = default;

	PlantumlBlock(const PLANTUML_BLOCK_TYPE& type)
		: type(type)
	{
	}
};

/**
///\brief Структура, представляющая raw блока
*/
struct PlantumlRawBlock : PlantumlBlock
{
	std::string text;								 ///< Текст блока
	virtual ~PlantumlRawBlock() = default;

	PlantumlRawBlock(const PLANTUML_BLOCK_TYPE& type, const std::string& text)
		: text(text), PlantumlBlock(type)
	{
	}
};