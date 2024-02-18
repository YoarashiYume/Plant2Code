#pragma once

#include "../../Common/Log/Base/ILogable.hpp"
#include "RawSignalData.hpp"
#include "../Propery/SignalReadProperty.hpp"

#include <type_traits>
#include <memory>

/**
///\brief - Интерфейсный шаблон для считывания сигналов
///\tparam T - Тип считывателя сигналов, который должен быть производным от ILogable
 */
template<typename T,
	typename = std::enable_if<std::is_base_of_v<ILogable, T>>>
class ISignalReader : public T
{
public:
	/**
	///\brief - Открывает файл сигнала по указанному пути
	///\param [in] path - Путь к файлу сигнала
	///\return - true, если файл был успешно открыт, false в противном случае
	*/
	virtual bool open(const std::string& path) = 0;
	/**
	///\brief - Проверяет, был ли достигнут конец файла (EOF)
	///\return - true, если EOF был достигнут, false в противном случае
	*/
	virtual bool isEOF() const = 0;
	/**
	///\brief - Инициализирует номера колонок для соответствующих данных
	///\return - true, если инициализация была успешной, false в противном случае
	*/
	virtual bool initCSVHeader(const SignalReadProperty& property) = 0;
	/**
	/// \brief - Возвращает следующий сигнал из файла
	/// \return - Указатель на объект RawReadData с информацией о сигнале
	*/
	virtual std::shared_ptr<RawReadData> getNextSignal() = 0;

	virtual ~ISignalReader() = default;
};
