#pragma once
#include "Base/PlantumlBlock.hpp"
#include <vector>
#include <memory>
/*
///\brief Считанные данные алгоритма
*/
struct InitialHandlerData
{
	std::string path;											///< Путь к алгоритму
	std::string algorithmName;									///< Название алгоритма
	std::vector < std::shared_ptr<PlantumlBlock> > instruction;	///< Инструкции алгоритма
};