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
///\brief Структура, представляющая raw-блок
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
/**
///\brief Структура, представляющая raw-блок функции
*/
struct PlantumlRawFunctionBlock : PlantumlRawBlock
{
	std::string inArgs,								///< Входные аргументы ф-ии
		outArgs;									///< Выходные аргументы функции
	virtual ~PlantumlRawFunctionBlock() = default;

	PlantumlRawFunctionBlock(const PLANTUML_BLOCK_TYPE& type, const std::string& text)
		: PlantumlRawBlock(type, text)
	{
	}

	PlantumlRawFunctionBlock(const PLANTUML_BLOCK_TYPE& type, const std::string& text, const std::string& inArgs, const std::string& outArgs)
		: outArgs(outArgs), inArgs(inArgs), PlantumlRawBlock(type, text)
	{
	}
};