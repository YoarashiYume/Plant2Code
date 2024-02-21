#pragma once
#include <fstream>
#include <memory>
#include <vector>
#include <sstream>

/// \brief ќбертка над std::ifstream.
using unique_ifstream_type = std::unique_ptr<std::ifstream, void(*)(std::ifstream* ptr)>;
/// \brief ќбертка над std::ifstream.
using unique_ofstream_type = std::unique_ptr<std::ofstream, void(*)(std::ofstream* ptr)>;

/*
///\brief - Deleter дл€ ifstream
///\param[in, out] ptr - ”казатель на ifstream
*/
inline void closeFunction(std::ifstream* ptr)
{
	if (ptr)
	{
		ptr->close();
		delete ptr;
	}
}

/*
///\brief - Deleter дл€ ofstream
///\param[in, out] ptr - ”казатель на ofstream
*/
inline void closeFunction(std::ofstream* ptr)
{
	if (ptr)
	{
		ptr->close();
		delete ptr;
	}
}
/**
\brief ‘ункиц€ преобразующа€ строку CSV-файла в массив данных
\param text исходна€ строка
\param splitter разделитель, используемый в CSV
\param saveEmpty флаг необходимости сохранени€ пустых полей
\return список полей
*/
inline std::vector<std::string> splitToVector(const std::string& text, const char splitter, bool saveEmpty = false)
{
	std::vector<std::string> vec;
	if (!text.empty())
	{
		std::basic_stringstream<char> ss{ text.data() };
		std::string string;

		while (std::getline(ss, string, splitter))
			if (saveEmpty)
				vec.emplace_back(string);
			else if (string.empty() == false)
				vec.emplace_back(string);
		if (text.back() == splitter)
			vec.emplace_back();
	}
	return vec;
}