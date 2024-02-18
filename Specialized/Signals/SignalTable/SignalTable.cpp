#include "SignalTable.hpp"
#include <format>
static std::uint32_t substringCount(const std::string& string, const std::string& substring)
{
	std::uint32_t occurrences = 0;
	std::string::size_type pos = 0;
	while ((pos = string.find(substring, pos)) != std::string::npos) {
		++occurrences;
		pos += substring.length();
	}
	return occurrences;
}
bool SignalTable::addStructData(store_type newSignal)
{
	auto separatorCount = std::count(newSignal->name.begin(), newSignal->name.end(), structSeparator) -
		substringCount(newSignal->name, arraySize) * std::count(arraySize.begin(), arraySize.end(), structSeparator);
	if (separatorCount >= 2)
	{
		this->log(LOG_TYPE::ERROR, std::format("Структура \"{}\" имеет несколько обращений к полям.", newSignal->name));
		return false;
	}
	std::string structName = newSignal->name.substr(0, newSignal->name.find(structSeparator));
	if (structName == newSignal->name || !structData.contains(structName))
	{
		auto emplace = structData.emplace(structName, new StructData);
		if (emplace.second == false && emplace.first->second->title.size())
		{
			this->log(LOG_TYPE::WARNING, std::format("Дублирование структуры \"{}\". Дублируемая структура не будет добавлена.", newSignal->name));
			return false;
		}
		else if (emplace.second == false)
			//Допускаем, что изначально объявлено поле, а не структура
			emplace.first->second->title = newSignal->title;
		else
		{
			auto* ptr = dynamic_cast<StructData*>(emplace.first->second.get());
			auto* other = dynamic_cast<RawSignalData*>(newSignal.get());
			ptr->title = other->title;
			ptr->type = other->type;
			ptr->signalType = other->signalType;
			ptr->name = other->title;
		}
	}
	if (structName == newSignal->name)
		return true;
	newSignal->name = newSignal->name.substr(newSignal->name.find(structSeparator)+1);
	if (dynamic_cast<StructData*>(structData.at(structName).get())->fields.contains(newSignal->name))
	{
		this->log(LOG_TYPE::WARNING, std::format("Структура \"{}\" имеет дублируемое поле \"{}\". Дубликат будет проигнорирован.", structName + structSeparator + newSignal->name
			, newSignal->name));
	}
	else
	{
		storage_type tempStorage;
		auto isSignalData = addSignalData(newSignal, tempStorage, structName + structSeparator + newSignal->name);
		if (isSignalData)
		{
			auto ptr = dynamic_cast<SignalData*>(tempStorage.begin()->second.get());
			auto data = std::dynamic_pointer_cast<SignalData>(createFullSignal(tempStorage.begin()->second, ptr->isPtr, ptr->elementCount));
			dynamic_cast<StructData*>(structData.at(structName).get())->fields.emplace(newSignal->name, data);
		}
		else
			this->log(LOG_TYPE::WARNING, std::format("Структура \"{}\" имеет некорректное поле \"{}\".", structName + structSeparator + newSignal->name
				, newSignal->name));
		newSignal->name = structName + structSeparator + newSignal->name;
	}
	return true;
}

SignalData::element_count_type SignalTable::getElementCountFromName(std::string& name, const std::string realName)
{
	//+1 Т.к. если [0], то размер 1
	return getElementCount(name, realName) + 1;
}

SignalData::element_count_type SignalTable::getElementCount(std::string& type, const std::string name)
{
	auto startArrayPos = type.find(arrayStart);
	auto endArrayPos = type.find(arrayEnd);
	if (startArrayPos == std::string::npos || endArrayPos == std::string::npos)
	{
		this->log(LOG_TYPE::WARNING, std::format("Некорректно заполнен сигнал \"{}\" - некорректное обращение по индексу массива."
			" Сигнал не будет добавлен.", name));
		return false;
	}
	SignalData::element_count_type elementCount{ 1 };
	auto substr = type.substr(startArrayPos + 1, endArrayPos - startArrayPos - 1);
	try
	{
		elementCount = static_cast<SignalData::element_count_type>(std::stoi(substr));
	}
	catch (...)
	{
		this->log(LOG_TYPE::WARNING, std::format("Некорректно заполнен сигнал \"{}\" - некорректное обращение по индексу массива."
			" Сигнал не будет добавлен.", name));
		return 0;
	}
	type = removePtrFromType(type);
	return elementCount;
}

SignalData::element_count_type SignalTable::getElementCountFromOneLinesSignal(std::string& name, const std::string realName)
{
	auto startArrayPos = name.find(arrayStart);
	auto endArrayPos = name.find(arrayEnd);
	if (startArrayPos == std::string::npos || endArrayPos == std::string::npos)
	{
		this->log(LOG_TYPE::WARNING, std::format("Некорректно заполнен сигнал \"{}\" - некорректное заполнение типа."
			" Сигнал не будет добавлен.", realName));
		return false;
	}
	SignalData::element_count_type elementCount{ 1 };
	startArrayPos = name.find(arraySize) + 1;
	auto substr = name.substr(startArrayPos + 1, endArrayPos - startArrayPos - 1);
	try
	{
		elementCount = static_cast<SignalData::element_count_type>(std::stoi(substr));
	}
	catch (...)
	{
		this->log(LOG_TYPE::WARNING, std::format("Некорректно заполнен сигнал \"{}\" - некорректное заполнение типа."
			" Сигнал не будет добавлен.", realName));
		return 0;
	}
	name = removePtrFromType(name);
	return elementCount+1;
}

bool SignalTable::checkExistSignal(RawSignalData* newSignal, SignalData* oldSignal, SignalData::element_count_type newElementCount)
{
	if (newSignal->type != oldSignal->type || oldSignal->isPtr == false)
	{
		this->log(LOG_TYPE::ERROR, std::format("Сигнал \"{}\" имеет различные типы.", newSignal->name));
		return false;
	}
	if (oldSignal->elementCount < newElementCount)
		oldSignal->elementCount = newElementCount;
	return true;
}

bool SignalTable::addSignalData(store_type newSignal, storage_type& storage, const std::string realName)
{
	auto emplaceResult{ true };
	if (newSignal->name.find(structSeparator) != std::string::npos)
	{
		//Считываются сигналы вида Signal[0..24]
		if (newSignal->name.find(structSeparator) != newSignal->name.find(arraySize))
		{
			log(LOG_TYPE::ERROR, std::format("Сигнал \"{}\" содержит символ обращения к структуре \"{}\"", realName, structSeparator));
			return false;
		}
		SignalData::element_count_type elementCountInName{ getElementCountFromOneLinesSignal(newSignal->name, realName)};
		SignalData::element_count_type elementCountInType{ getElementCount(dynamic_cast<RawSignalData*>(newSignal.get())->type,
			realName) };
		if (elementCountInName == 0 || elementCountInType == 0)
			return false;
		if (elementCountInName != elementCountInType)
		{
			this->log(LOG_TYPE::WARNING, std::format("Cигнал \"{}\" - имеет различные размеры массива {} и {}."
				" Используется большее значение.", realName, std::to_string(elementCountInName), std::to_string(elementCountInType)));
			elementCountInName = elementCountInType = elementCountInType > elementCountInName ? elementCountInType : elementCountInName;
		}
		emplaceResult = storage.emplace(newSignal->name, createFullSignal(newSignal,true, elementCountInName)).second;
	}
	else
	{
		//Считываются сигналы вида Signal[24]
		auto startArrayPos = newSignal->name.find(arrayStart);
		auto endArrayPos = newSignal->name.find(arrayEnd);
		
		if (startArrayPos != std::string::npos || endArrayPos != std::string::npos)
		{
			if (startArrayPos == std::string::npos || endArrayPos == std::string::npos)
			{
				this->log(LOG_TYPE::WARNING, std::format("Некорректно заполнен сигнал \"{}\" - некорректное обращение по индексу массива."
					" Сигнал не будет добавлен.", realName));
				return false;
			}
			SignalData::element_count_type elementCount{ getElementCountFromName(newSignal->name, realName) };
			if (elementCount == 0)
				return false;
			auto emplace = storage.emplace(newSignal->name, createFullSignal(newSignal, true, elementCount));
			auto ptr = dynamic_cast<SignalData*>(emplace.first->second.get());
			auto& oldType = dynamic_cast<RawSignalData*>(newSignal.get())->type;
			if (oldType.find(arrayStart) != std::string::npos)
			{
				this->log(LOG_TYPE::ERROR, std::format("Двумерные массивы не поддерживаются. Сигнал \"{}\". Приведение к простому массиву.", realName));
				oldType = removePtrFromType(oldType);
			}
			if (emplace.second)
				ptr->type = oldType;
			else
				//Необходима проверка, т.к. происходит модификация уже добавленного сигнала, составляющего массив. Следовательно тип сигнала должен совпадать
				emplace.second = checkExistSignal(dynamic_cast<RawSignalData*>(newSignal.get()), ptr, elementCount);
			emplaceResult = emplace.second;
		}
		else
		{
			emplaceResult = storage.emplace(newSignal->name, createFullSignal(newSignal)).second;
		}
	}
	if (emplaceResult == false)
		this->log(LOG_TYPE::WARNING, std::format("Дублирование сигнала \"{}\". Дублируемый сигнал не будет добавлен.", realName));
	return emplaceResult;
}

SignalTable::store_type SignalTable::createFullSignal(const store_type& rawSignal, bool isPtr, SignalData::element_count_type  elementCount) const
{
	store_type newSignal{ new SignalData };
	auto* ptr = dynamic_cast<SignalData*>(newSignal.get());
	auto* other = dynamic_cast<RawSignalData*>(rawSignal.get());
	ptr->name = other->name;
	ptr->signalType = other->signalType;
	ptr->title = other->title;
	ptr->type = other->type;
	ptr->isPtr = isPtr;
	ptr->elementCount = elementCount;
	return newSignal;
}

std::string SignalTable::removePtrFromType(const std::string oldType)
{
	auto startArrayPos = oldType.find(arrayStart);
	return oldType.substr(0, startArrayPos);
}


bool SignalTable::addSignal(store_type newSignal)
{
	storage_type* storage{nullptr};
	switch (newSignal->signalType)
	{
	case RAW_SIGNAL_TYPE::INPUT_SIGNAL:
		return addSignalData(newSignal, inputSignals, newSignal->name);
	case RAW_SIGNAL_TYPE::OUTPUT_SIGNAL:
		return addSignalData(newSignal, outputSignals, newSignal->name);
	case RAW_SIGNAL_TYPE::INNER_SIGNAL:
		return addSignalData(newSignal, innerValues, newSignal->name);
	case RAW_SIGNAL_TYPE::TIMER:
		storage = &inputSignals;
		break;
	case RAW_SIGNAL_TYPE::STRUCT:
		return addStructData(newSignal);
	case RAW_SIGNAL_TYPE::CONST:
		storage = &inputSignals;
		break;
	default:
		newSignal->signalType = RAW_SIGNAL_TYPE::UNKMOWN;
	case RAW_SIGNAL_TYPE::UNKMOWN:
		this->log(LOG_TYPE::WARNING, std::format("Попытка добавления неизвестного сигнала \"{}\". Дублируемый сигнал не будет добавлен.", newSignal->name));
		storage = &unknownSignals;
		break;
	}
	auto addResult = storage->emplace(newSignal->name, newSignal);
	if (addResult.second == false)
		this->log(LOG_TYPE::WARNING, std::format("Дублирование сигнала \"{}\". Дублируемый сигнал не будет добавлен.", newSignal->name));
	return addResult.second;
}

SignalTable::store_type SignalTable::getSignal(const key_type& key)
{
	return store_type();
}
