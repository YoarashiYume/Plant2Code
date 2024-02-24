#pragma once
#include "../../../Common/Log/Base/ILogable.hpp"
#include "InitialHandlerProperty.hpp"
#include <type_traits>
template<typename T,
	typename = std::enable_if<std::is_base_of_v<ILogable, T>>>
/**
///\brief Интерфейс для первичной обработки plantuml
*/
class IInitalHandler : public T
{
public:
	/**
	///\brief Инициализация обработчика новым свойством
	///\param[in] newProperty - Новые свойство для инициализации
	*/
	virtual void init(const InitialHandlerProperty& newProperty) = 0;
	/**
	///\brief Чтение файла PlantUML по указанному пути
	///\param[in] path Путь к файлу PlantUML
	///\return true, если чтение прошло успешно, false в противном случае
	*/
	virtual bool readPlantuml(const std::string& path) = 0;
	/**
	///\brief Чтение нескольких файлов PlantUML по указанным путям
	///\param[in] paths Список путей к файлам PlantUML
	///\return true, если чтение прошло успешно, false в противном случае
	*/
	virtual bool readPlantuml(const std::vector<std::string>& paths) = 0;
	/**
	///\brief Очистка данных обработчика
   */
	virtual void clearData() = 0;
};