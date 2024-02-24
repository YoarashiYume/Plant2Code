#pragma once
#include <string>
#include <stdexcept>
/*
///\brief - Интерфейс для чтения из файла PUML
*/
class IPumlReader
{
protected:
	/**
	///\brief - Получает текущую строку из файла.
	///\return - Текущая строка
	*/
	virtual std::string getLineFromFile() = 0;
public:
	/**
	///\brief - Открывает файл PUML для чтения.
	///\param [in] path - Путь к файлу PUML для открытия.
	///\return - true, если файл был успешно открыт, в противном случае false.
	*/
	virtual bool open(const std::string& path) = 0;
	/**
    ///\brief - Проверяет, достигнут ли конец файла.
	///\return - true, если конец файла достигнут, в противном случае false.
	*/
	virtual bool isEOF() const = 0;
	/**
	///\brief - Заменяет указанную подстроку в файле.
	///\param [in] oldSubstring - Подстрока, которую необходимо заменить.
	///\param [in] newSubstring - Новая подстрока для замены.
	*/
	virtual void replaceInText(const std::string& oldSubstring, const std::string& newSubstring) = 0;
	
	/**
	///\brief - Получает текущую строку из файла.
	///\return - Текущая строка
	*/
	virtual std::string getLine() final
	{
		auto nextLine{ getLineFromFile() };
		if (nextLine.empty())
			throw std::runtime_error("Считанная строка не может быть пустой");
		return nextLine;
	}
	/**
	///\brief - Возвращает путь до файла.
	///\return - Текущий путь
	*/
	virtual std::string getPath() const = 0;

};