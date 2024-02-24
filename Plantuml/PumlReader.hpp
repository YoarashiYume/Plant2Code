#pragma once
#include <sstream>
#include "Base/IPumlReader.hpp"
/**
///\brief Класс, представляющий считыватель файлов в формате Puml
*/
class PumlReader : public IPumlReader
{
private:
	std::string currentPath;	///< Текущий путь к файлу
	std::string data;			///< Данные файла
	std::stringstream fileData;	///< Поток данных файла

	/**
	///\brief Сохраняет данные файла
	///\return true, если данные сохранены успешно, иначе false
	*/
	bool saveData();
protected:
	/**
	///\brief Получает строку из файла
	///\return Строка из файла
	*/
	std::string getLineFromFile() override;
public:
	// Унаследовано через IPumlReader
	bool open(const std::string& path) override;
	bool isEOF() const override;
	void replaceInText(const std::string& oldSubstring, const std::string& newSubstring) override;
	std::string getPath() const override;
};

