#pragma once
#include "Base/PlantumlBlock.hpp"
#include "../Signals/Base/SignalData.hpp"
#include <memory>
#include <vector>
/*
///\brief Считанные данные алгоритма
*/
struct InitialHandlerData
{
	using instruction_type = std::shared_ptr<PlantumlBlock>;
	using signal_type = std::shared_ptr<SignalData>;
	using signal_storage_type = std::vector<signal_type>;
	std::string path;											///< Путь к алгоритму
	std::string algorithmName;									///< Название алгоритма
	std::vector<instruction_type> instruction;					///< Инструкции алгоритма
	signal_storage_type inputSignal,							///< Входные сигналы алгоритма
		localSignal,											///< Локальные сигналы алгоритма
		outputSignal;											///< Выходные сигналы алгоритма
	std::string description;									///< Примечания алгоритма
};