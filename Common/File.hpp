#pragma once
#include <fstream>
#include <memory>
#include <vector>
#include <sstream>

/// \brief ������� ��� std::ifstream.
using unique_ifstream_type = std::unique_ptr<std::ifstream, void(*)(std::ifstream* ptr)>;
/// \brief ������� ��� std::ifstream.
using unique_ofstream_type = std::unique_ptr<std::ofstream, void(*)(std::ofstream* ptr)>;

/*
///\brief - Deleter ��� ifstream
///\param[in, out] ptr - ��������� �� ifstream
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
///\brief - Deleter ��� ofstream
///\param[in, out] ptr - ��������� �� ofstream
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
\brief ������� ������������� ������ CSV-����� � ������ ������
\param text �������� ������
\param splitter �����������, ������������ � CSV
\param saveEmpty ���� ������������� ���������� ������ �����
\return ������ �����
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