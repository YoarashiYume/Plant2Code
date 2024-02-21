#pragma once
#include "Base/ISignalReader.hpp"
#include "../Common/Log/Logable.hpp"
#include "../Common/File.hpp"

#include <istream>

class SignalReader : public ISignalReader<Logable>
{
public:
	using col_index_type = std::int32_t;
	using prefix_type = SignalReadProperty::prefix_type;
protected:
	std::string currentPath;		///< - Текущий путь к файлу
	unique_ifstream_type stream;	///< - Поток чтения из файла
	col_index_type typeIndex,		///< - Индекс столбца с типом сигнала
		nameIndex,					///< - Индекс столбца с названием переменной
		titleIndex,					///< - Индекс столбца с названием сигнала
		valueIndex;					///< - Индекс столбца со значением константы
	prefix_type prefixData;			///< - Данные префикса
	char separator;					///< - Разделитель CSV таблицы

	/**
	///\brief - Сохранение данных о префиксах сигналов
	///\param [in] other - Передаваемые данные
	///\return - true, если файл был успешно открыт, false в противном случае
	*/
	void copyPrefixData(const prefix_type& other);
	/**
	///\brief - Определяет сообщение об ошибке при инициализации header`a таблицы
	///\return - Строка с текстом ошибки
	*/
	std::string defineInitError() const;
	/**
	///\brief - Определяет индекс данных из вариантов названия для заголовка
	///\param [in] variants - Варианты названий столбца
	///\param [in] header - Header таблицы
	///\return - Индекс столбца с интересующими данными, в случае неудачи -1
	*/
	std::int32_t defineData(const SignalReadProperty::col_variant_type& variants, const std::vector<std::string>& header) const;
	/**
	///\brief - Определяет тип PlantUML сигнала
	///\param [in] plantumlType - PlantUML сигнал
	///\return - Тип сигнала, в в случае неудачи RAW_SIGNAL_TYPE::UNKMOWN
	*/
	RAW_SIGNAL_TYPE defineSignalType(const std::string& plantumlType) const;
	/**
	///\brief - Проверяет префикс типов сигнала для текущего сигнала
	///\param [in] plantumlType - PlantUML сигнал
	///\param [in] variants - Варианты префикса типа сигнала
	///\return - true, если сигнал подходит текущему типу, false в противном случае
	*/
	bool checkPrefix(const std::string& plantumlType, const prefix_type::prefix_variant_type& variants) const;

private:
	/**
	///\brief - Сбрасывает данные
	*/
	void resetData();

public:

	SignalReader();

	/**
	///\brief - Открывает файл сигнала по указанному пути
	///\param [in] path - Путь к файлу сигнала
	///\return - true, если файл был успешно открыт, false в противном случае
	*/
	bool open(const std::string& path) override;
	/**
	///\brief - Проверяет, был ли достигнут конец файла (EOF)
	///\return - true, если EOF был достигнут, false в противном случае
	*/
	bool isEOF() const override;
	/**
	///\brief - Инициализирует номера колонок для соответствующих данных
	///\return - true, если инициализация была успешной, false в противном случае
	*/
	bool initCSVHeader(const SignalReadProperty& property) override;
	/**
	/// \brief - Возвращает следующий сигнал из файла
	/// \return - Указатель на объект RawReadData с информацией о сигнале
	*/
	std::shared_ptr<RawReadData> getNextSignal() override;
};

