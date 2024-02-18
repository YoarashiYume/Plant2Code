#include "SignalsHandler.hpp"

SignalsHandler::SignalsHandler()
{
    table.reset(new SignalTable);
	readProperty.separatorCSV = ';';
	readProperty.variablesColNameVariant = { "Обозначение", "Обозначение состояния", "Обозначение сигнала" };
	readProperty.variablesColTitleVariant = { "Наименование", "Название сигнала", "Название состояния" };
	readProperty.variablesColTypeVariant = { "Тип данных", "\"Тип данных\"" };
	readProperty.variablesColValueVariant = { "Значение" };
	readProperty.prefixes.ignoreVariant = { "Резерв" };
	readProperty.prefixes.innerSignalPrefix = { "Z" };
	readProperty.prefixes.outputSignalPrefix = { "Y" };
	readProperty.prefixes.inputSignalPrefix = { "X" };
	readProperty.prefixes.constPrefix = { "C" };
	readProperty.prefixes.timerSignalPrefix = { "T" };
	readProperty.prefixes.structSignalPrefix = { "S" };
}

bool SignalsHandler::readSignals(const std::string& path)
{
	SignalReader reader;
	if (reader.open(path) == false)
	{
		return false;
	}
	if (reader.initCSVHeader(readProperty))
	{
		return false;
	}
	bool isOkRead{ true };
	while (!reader.isEOF())
	{
		isOkRead &= table->addSignal(reader.getNextSignal());
	}
    return isOkRead;
}

bool SignalsHandler::readSignals(const std::vector<std::string>& paths)
{
    bool readResult{ true };
    for (auto& path : paths)
        readResult &= readSignals(path);
    return readResult;
}

void SignalsHandler::clearData()
{
	table.reset(new SignalTable);
}
