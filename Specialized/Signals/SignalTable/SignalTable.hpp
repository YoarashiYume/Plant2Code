#pragma once
#include <memory>
#include <unordered_map>
#include "SignalData.hpp"
#include "../../../Common/Log/Logable.hpp"
/**
///\brief Класс для хранения таблицы сигналов
*/
class SignalTable : public Logable
{
public:
	using key_type = std::string;
	using store_type = std::shared_ptr<RawReadData>;
	using storage_type = std::unordered_map<key_type, store_type>;

	const char structSeparator = '.';	///< - Символ разделения структуры
	const char arrayStart = '[';		///< - Символ начала массива
	const char arrayEnd = ']';			///< - Символ конеца массива
	const std::string arraySize = "..";	///< - Символ размера массива в названии переменнной

protected:
	storage_type inputSignals;			///< - Данные о входных сигналах
	storage_type outputSignals;			///< - Данные о выходных сигналах
	storage_type innerValues;			///< - Данные о внутренних сигналах
	storage_type constValues;			///< - Данные о константах
	storage_type timers;				///< - Данные о таймерах
	storage_type structData;			///< - Данные о структурах
	storage_type unknownSignals;		///< - Данные о неизвестных сигналах

	
	
	/**
	///\brief Добавить данные о структуре
	///\param[in, out] newSignal Добавляемый сигнал
	///\return true если добавление успешно, иначе false
	*/
	bool addStructData(store_type newSignal);
	/**
	///\brief Добавить данные о сигнале
	///\param[in, out] newSignal Добавляемый сигнал
	///\param[in, out] storage Хранилище данных
	///\param[in] realName Реальное имя добавляемого сигнала
	///\return true если добавление успешно, иначе false
	*/
	bool addSignalData(store_type newSignal, storage_type& storage, const std::string realName);
	/**
	///\brief Конструирование нового сигнала
	///\param[in, out] rawSignal Исходный сигнал
	///\param[in] isPtr Является ли сигнал указателем
	///\param[in] elementCount Кол-во элементов в массиве
	///\return Добавляемый сигнал
	*/
	store_type createFullSignal(const store_type& rawSignal, bool isPtr = false, SignalData::element_count_type elementCount = 1) const;
	/**
	///\brief Удалить указатель из типа
	///\param[in] oldType Старый тип
	///\return std::string Тип без указателя
	*/
	std::string removePtrFromType(const std::string oldType);
	/**
	///\brief Получить количество элементов по имени
	///\param[in, out] name Имя сигнала
	///\param[in] realName Реальное имя сигнала
	///\return Количество элементов в имени сигнала
	*/
	SignalData::element_count_type getElementCountFromName(std::string& name, const std::string realName);
	/**
	///\brief Получить количество элементов по типу
	///\param[in, out] type Тип сигнала
	///\param[in] realName Реальное имя сигнала
	///\return Количество элементов в типе сигнала
	*/
	SignalData::element_count_type getElementCount(std::string& type, const std::string name);
	/**
	///\brief Получить количество элементов из сигнала, объявленного одной строкой
	///\param[in, out] name Данные сигнала
	///\param[in] realName Реальное имя сигнала
	///\return Количество элементов в типе сигнала
	*/
	SignalData::element_count_type getElementCountFromOneLinesSignal(std::string& name, const std::string realName);
	/**
	///\brief Проверка и изменение существующего сигнала, при совпадении параметров
	///\param[in, out] newSignal Новый сигнал
	///\param[in, out]] oldSignal Существующий сигнал
	///\param[in] newElementCount Новое кол-во элементов
	///\return Количество элементов в типе сигнала
	*/
	bool checkExistSignal(RawSignalData* newSignal, SignalData* oldSignal, SignalData::element_count_type newElementCount);
public:
	/**
	///\brief Добавить новый сигнал
	///\param newSignal Новый сигнал
	///\return true если добавление успешно, иначе false
	*/
	bool addSignal(store_type newSignal);
	/**
	///\brief Получить сигнал по ключу
	///\param key Ключ сигнала
	///\return Сигнал по ключу
	*/
	store_type getSignal(const key_type& key);
};

