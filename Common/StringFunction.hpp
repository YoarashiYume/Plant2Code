#pragma once
#include <string>
#include <algorithm>
/*
	///\brief - Функция tolower для int8_t
	///\param[in] ch - Символ типа int8_t
	///\return - Символ в нижнем регистре
	*/
inline std::uint8_t __tolower(const char ch)
{
	return tolower(static_cast<std::uint8_t>(ch));
}
/*
///\brief - Функция преобразования строки к нижнему регистру
///\param[in] text - Преобразуемая строка
///\return - Результат преобразования
*/
inline std::string toLower(std::string text)
{
	std::transform(text.begin(), text.end(), text.begin(), tolower);
	return text;
}
/*
///\brief - Функция iscntrl для int8_t
///\param[in] ch - Символ типа int8_t
///\return - Результат функции iscntrl
*/
inline bool _iscntrl(const char ch)
{
	return iscntrl(static_cast<std::uint8_t>(ch)) != 0;
}
/*
///\brief - Функция isspace для int8_t
///\param[in] ch - Символ типа int8_t
///\return - Результат функции isspace
*/
inline bool _isspace(const char ch)
{
	return isspace(static_cast<std::uint8_t>(ch)) != 0;
}
/*
///\brief - Функция isalnum для int8_t
///\param[in] ch - Символ типа int8_t
///\return - Результат функции isalnum
*/
inline bool _isalnum(const char ch)
{
	return isalnum(static_cast<std::uint8_t>(ch)) != 0;
}
/*
///\brief - Функция isalpha для int8_t
///\param[in] ch - Символ типа int8_t
///\return - Результат функции isalpha
*/
inline bool _isalpha(const char ch)
{
	return isalpha(static_cast<std::uint8_t>(ch)) != 0;
}
/*
///\brief - Функция isupper для int8_t
///\param[in] ch - Символ типа int8_t
///\return - Результат функции isupper
*/
inline bool _isupper(const char ch)
{
	return isupper(static_cast<std::uint8_t>(ch)) != 0;
}
/*
///\brief - Функция isdigit для int8_t
///\param[in] ch - Символ типа int8_t
///\return - Результат функции isdigit
*/
inline bool _isdigit(const char ch)
{
	return isdigit(static_cast<std::uint8_t>(ch)) != 0;
}
/*
	///\brief - Функция удаляющая лишние пробельные символы
	///\param[in, out] str - Обрабатываемая строка
	*/
inline void removeExtraSpacesFromText(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](const std::string::value_type ch) -> char
		{
			if (_iscntrl(ch))
				return 32;
			return ch;
		});
	str.erase(std::unique(str.begin(), str.end(),
		[](const char& a, const char& b)
		{
			if (a < 0 || b < 0)
				return false;
			return _isspace(a) && _isspace(b);
		}),
		str.end());

	if (!str.empty() && _isspace(*str.begin()))
		str.erase(str.begin());
	if (!str.empty() && _isspace(*str.rbegin()))
		str.pop_back();
}
/*
	///\brief - Функция удаляющая лишние пробельные символы
	///\param[in] str - Обрабатываемая строка
	///\return Обработанная строка
	*/
inline std::string removeExtraSpacesFromText(std::string&& str)
{
	std::string result = str;
	removeExtraSpacesFromText(result);
	return result;
}
/*
	///\brief - Функция добавляющая слово к строке
	///\param[in] src - Добавляемое слово
	///\param[in, out] dest - Преобразуемая строка
	///\param[in] separator - Символ-разделитель
	*/
inline void appendString(const std::string& src, std::string& dest, const char separator = ' ')
{
	if ((dest.size() && dest.back() != separator) && (src.size() && src.front() != separator))
		dest += separator;
	dest += src;
}
/*
///\brief - Функция добавляющая слово к строке
///\param[in] src - Добавляемое слово
///\param[in, out] dest - Преобразуемая строка
///\param[in] separator - Символ-разделитель
*/
inline std::string join(const std::vector<std::string>& src, const char separator = ' ')
{
	std::string result;
	for (auto& el : src)
		appendString(el, result, separator);
	return result;
}
/*
///\brief - Функция заменяющая последовательность
///\param[in, out] str - Преобразуемая строка
///\param[in] escape - Удаляемая последовательность
///\param[in] escape - Новая последовательность
*/
inline void removeSequence(std::string& str, const std::string& escape, const std::string& replacement = " ")
{
	auto pos = str.find(escape);
	while (pos != std::string::npos) {
		str.replace(pos, escape.length(), replacement);
		pos = str.find(escape, pos + replacement.size());
	}
}
/*
///\brief - Функция заменяющая последовательность
///\param[in] str - Преобразуемая строка
///\param[in] escape - Удаляемая последовательность
///\param[in] escape - Новая последовательность
///\return - Результат функции преобразования
*/
inline std::string removeSequence(const std::string& str, const std::string& escape, const std::string& replacement = " ")
{
	auto copy = str;
	removeSequence(copy, escape, replacement);
	return copy;
}
/*
///\brief - Функция проверяющая баланс символов startSymbol и endSymbol
///\param[in] line - Проверяемая строка
///\param[in] startSymbol - Начинающий символ
///\param[in] endSymbol - Оканчивающий символ
///\param[in,out] firstZeroPos - Позиция окончания первого блока
///\return - Возвращает 0, если соблюден баланс. -1, если символов, в любом месте строки, endSymbol больше. >0, если больше символов startSymbol
*/
inline std::int32_t checkStringBalance(const std::string& line, const char startSymbol, const char endSymbol, std::string::size_type* firstZeroPos = nullptr)
{
	if (firstZeroPos)
		*firstZeroPos = std::string::npos;
	std::int32_t nonCloseBlockCount{};
	for (std::string::size_type i = 0; i < line.size(); ++i)
	{
		auto& ch = line.at(i);
		if (ch == startSymbol)
			++nonCloseBlockCount;
		else if (ch == endSymbol)
		{
			if (--nonCloseBlockCount < 0)
				break;
			if (firstZeroPos)
				if (nonCloseBlockCount == 0 && *firstZeroPos == std::string::npos)
					*firstZeroPos = i;

		}
	}
	return nonCloseBlockCount;
}