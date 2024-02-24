#include "SignalReader.hpp"

#include <format>
#include <algorithm>

static bool sortNames(const std::string& l, const std::string& r)
{
	return l.size() > r.size();
}

static auto readCSVLine(std::ifstream& stream, char separator)
{
	std::string line;
	std::getline(stream, line);
	return splitToVector(line, separator, true);
}

void SignalReader::copyPrefixData(const prefix_type& other)
{
	prefixData = other;
	/* 
	Сортировка гарантирует, что изначально будут проверены более длинные типы
	*/
	std::ranges::sort(prefixData.innerSignalPrefix, sortNames);
	std::ranges::sort(prefixData.outputSignalPrefix, sortNames);
	std::ranges::sort(prefixData.innerSignalPrefix, sortNames);
	std::ranges::sort(prefixData.constPrefix, sortNames);
	std::ranges::sort(prefixData.timerSignalPrefix, sortNames);
	std::ranges::sort(prefixData.structSignalPrefix, sortNames);
}

std::string SignalReader::defineInitError() const
{
	std::string error;
	if (nameIndex == -1)
		error = "Отсутствует колонка с названием переменных.";
	if (valueIndex == -1)
		error += "Отсутствует колонка со значениями для констант.";
	if (typeIndex == -1)
		error += "Отсутствует колонка с типами данных.";
	return error;
}

std::int32_t SignalReader::defineData(const SignalReadProperty::col_variant_type& variants, const std::vector<std::string>& header) const
{
	std::int32_t index{ -1 };
	for (auto& currentVariant : variants)
	{
		auto pos = std::ranges::find(header, currentVariant);
		if (pos != header.end())
		{
			index = static_cast<std::int32_t>(std::distance(header.begin(), pos));
			break;
		}
	}
	return index;
}

RAW_SIGNAL_TYPE SignalReader::defineSignalType(const std::string& plantumlType) const
{
	/*
	Реализация не подразумевает, что различные типы сигналы имеют одинаковые/префиксы, входящие в состав префикса других типов
	*/
	if (checkPrefix(plantumlType, prefixData.inputSignalPrefix))
		return RAW_SIGNAL_TYPE::INPUT_SIGNAL;
	if (checkPrefix(plantumlType, prefixData.innerSignalPrefix))
		return RAW_SIGNAL_TYPE::INNER_SIGNAL;
	if (checkPrefix(plantumlType, prefixData.outputSignalPrefix))
		return RAW_SIGNAL_TYPE::OUTPUT_SIGNAL;
	if (checkPrefix(plantumlType, prefixData.timerSignalPrefix))
		return RAW_SIGNAL_TYPE::TIMER;
	if (checkPrefix(plantumlType, prefixData.constPrefix))
		return RAW_SIGNAL_TYPE::CONST;
	if (checkPrefix(plantumlType, prefixData.structSignalPrefix))
		return RAW_SIGNAL_TYPE::STRUCT;
	return RAW_SIGNAL_TYPE::UNKMOWN;
}

bool SignalReader::checkPrefix(const std::string& plantumlType, const prefix_type::prefix_variant_type& variants) const
{
	for (auto& currentVariant : variants)
		if (std::mismatch(currentVariant.begin(), currentVariant.end(), plantumlType.begin()).first == currentVariant.end())
			return true;
	return false;
}

void SignalReader::resetData()
{
	typeIndex = nameIndex = titleIndex = valueIndex = -1;
	separator = 32;
	currentPath = std::string{};
}

SignalReader::SignalReader()
	:stream(nullptr, closeFunction)
{
	resetData();
}

bool SignalReader::open(const std::string& path)
{
	resetData();
	currentPath = path;
	stream.reset(new std::ifstream{ currentPath });
	bool isOpen{ stream->is_open() };
	if (isOpen == false)
		this->log(LOG_TYPE::WARNING, std::format("Невозможно открыть файл \"{}\"", currentPath));
	return isOpen;
}

bool SignalReader::isEOF() const
{
	while (stream->peek() != EOF)
	{
		auto curPos = stream->tellg();
		auto splittedLineCSV = readCSVLine(*stream, separator);
		if (splittedLineCSV.empty())
			continue;
		if (titleIndex != -1)
			if (std::find(prefixData.ignoreVariant.begin(), prefixData.ignoreVariant.end(), splittedLineCSV.at(titleIndex)) != prefixData.ignoreVariant.end())
				continue;
		stream->seekg(curPos);
		return false;
	}
	return stream->peek() == EOF;
}

bool SignalReader::initCSVHeader(const SignalReadProperty& property)
{
	if (stream->tellg() == 0)
	{
		copyPrefixData(property.prefixes);

		separator = property.separatorCSV;
		auto splittedHeader = readCSVLine(*stream, separator);

		typeIndex = defineData(property.variablesColTypeVariant, splittedHeader);
		nameIndex = defineData(property.variablesColNameVariant, splittedHeader);
		titleIndex = defineData(property.variablesColTitleVariant, splittedHeader);
		valueIndex = defineData(property.variablesColValueVariant, splittedHeader);

		if (titleIndex == -1)
			this->log(LOG_TYPE::WARNING, std::format("Некорректное заполнение файла \"{}\". Отсутствуют данные о названии сигналов.", currentPath));

		auto isCorrectInit = ((nameIndex != -1) && ((valueIndex != -1) || (typeIndex != -1)));
		if (isCorrectInit == false)
			this->log(LOG_TYPE::ERROR, std::format("Некорректное заполнение файла \"{}\". Невозможно считать сигналы. {}", currentPath, defineInitError()));
		return isCorrectInit;
	}
	else
		this->log(LOG_TYPE::WARNING, std::format("Повторная инициализация файла \"{}\"", currentPath));
	return false;
}

std::shared_ptr<RawReadData> SignalReader::getNextSignal()
{
	std::shared_ptr<RawReadData> signalInfo{ new RawReadData };
	signalInfo->signalType = RAW_SIGNAL_TYPE::UNKMOWN;
	if (nameIndex != -1)
	{
		auto splittedLineCSV{ readCSVLine(*stream, separator) };
		std::string signalName = splittedLineCSV.at(nameIndex);
		auto currentType = defineSignalType(signalName);
		switch (currentType)
		{
		case RAW_SIGNAL_TYPE::UNKMOWN:
			this->log(LOG_TYPE::ERROR, std::format("Некорректное заполнение файла \"{}\". Невозможно определить тип сигнала {}", currentPath, splittedLineCSV.at(nameIndex)));
			return signalInfo;
		case RAW_SIGNAL_TYPE::INPUT_SIGNAL:
		case RAW_SIGNAL_TYPE::OUTPUT_SIGNAL:
		case RAW_SIGNAL_TYPE::INNER_SIGNAL:
		case RAW_SIGNAL_TYPE::STRUCT:
			signalInfo.reset(new RawSignalData);
			if (typeIndex == -1)
				this->log(LOG_TYPE::ERROR, std::format("Файл \"{}\" не содержит данные о типе сигнала {}.", currentPath, signalName));
			else
				dynamic_cast<RawSignalData*>(signalInfo.get())->type = splittedLineCSV.at(typeIndex);
			break;
		case RAW_SIGNAL_TYPE::TIMER:
			signalInfo.reset(new RawTimerData);
			break;
		case RAW_SIGNAL_TYPE::CONST:
			signalInfo.reset(new RawConstData);
			if (valueIndex == -1)
				this->log(LOG_TYPE::ERROR, std::format("Файл \"{}\" не содержит данные о значении константы {}.", currentPath, signalName));
			else
				dynamic_cast<RawConstData*>(signalInfo.get())->value = splittedLineCSV.at(typeIndex);
			break;
		}
		signalInfo->signalType = currentType;
		signalInfo->name = signalName;
		if (titleIndex == -1)
			this->log(LOG_TYPE::WARNING, std::format("Файл \"{}\" не содержит данные о названии сигнала {}.", currentPath, signalInfo->name));
		else
			signalInfo->title = splittedLineCSV.at(titleIndex);
		return signalInfo;
	}
	else
		this->log(LOG_TYPE::ERROR, std::format("Некорректное заполнение файла \"{}\". Невозможно считать сигнал. {}", currentPath, defineInitError()));
	return signalInfo;
}
