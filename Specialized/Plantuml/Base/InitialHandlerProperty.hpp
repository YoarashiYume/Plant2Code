#pragma once
#include <vector>
#include <string>

/**
///\brief Структура для хранения свойств обработчика plantuml
*/
struct InitialHandlerProperty
{
	using one_variant_setting = std::string;
	using multi_variant_setting = std::vector<std::string>;
	using multi_variant_char_setting = std::vector<char>;

	char plantumlBlockStart{ ':' };															///< Начало блока PlantUML
	char plantumlBlockCalculationEnd{']'};													///< Окончание блока вычисления PlantUML
	char plantumlBlockFunctionEnd{'|'};														///< Окончание блока функции PlantUML
	char plantumlBlockRefEnd{ ';'};															///< Окончание блока ссылки PlantUML
	char plantumlArrayEnd{ ']' };															///< Окончание массива
	char plantumlArrayStart{ '[' };															///< Начало массива 

	char plantumlOneLineComment{ '\'' };													///< Однострочный комментарий PlantUML
	one_variant_setting plantumlMultilineCommentStart{ "/\'" };								///< Начало многострочного комментария PlantUML
	one_variant_setting plantumlMultilineCommentEnd{ "\'/" };								///< Окончание многострочного комментария PlantUML		
	one_variant_setting plantumlInlineCommentMarker{ "$comment()" };						///< Маркер встроенного комментария PlantUML
			
	multi_variant_setting ignoredComment{ {"blank"} };										///< Игнорируемые комментарий
	multi_variant_setting ignoredFirstSymbol{ "!", "@", "detach"};									///< Слова, с которых начинаются игнорируемые

	multi_variant_setting plantumlIf{ {"if"} };												///< Условие if PlantUML
	multi_variant_setting plantumlElseIf{ {"elseif"}, {"else if"} };						///< Условие elseif PlantUML
	one_variant_setting plantumlIfPostfix{ "then (да)" };									///< Поствикс if и elseif PlantUML

	multi_variant_setting plantumlElse{ {"else"} };											///< Условие else PlantUML
	one_variant_setting plantumElsePostfix{ "(нет)" };										///< Поствикс else PlantUML

	multi_variant_setting plantumlEndIf{ {"endif"} };										///< Окончание условия if PlantUML

	multi_variant_setting plantumlSwitch{ {"switch"} };										///< Конструкция switch PlantUML
	multi_variant_setting plantumlCase{ {"case"} };											///< Конструкция case PlantUML
	multi_variant_setting plantumlEndSwitch{ {"endswitch"} };								///< Окончание конструкции switch PlantUML

	multi_variant_setting plantumlWhile{ {"while"} };										///< Конструкция while PlantUML
	one_variant_setting plantumWhilePostfix{ "is (да)" };									///< Поствикс while PlantUML
	multi_variant_setting plantumlEndWhile{ {"endwhile"} };									///< Окончание конструкции while PlantUML
	one_variant_setting plantumEndWhilePostfix{ "(нет)" };									///< Поствикс конца while PlantUML


	char plantumlServiceCommentSeparator{ '?' };											///< Символ отделения комментария в служебных блоках

	multi_variant_setting plantumlAssignVariant{ "+=", "-=", "*=", "/=", "%=", "=" };		 ///< Символы присваивания
	one_variant_setting plantumlExpandSymbol{ "..." };										 ///< Символ развертывания PlantUML

	char plantumlFunctionCallSymbol{ '\"' };												///< Символы вызова функции
	one_variant_setting plantumlFunctionArgSeparator{ "->" };								///< Символ разделения аргументов функции
	char plantumlFunctionArgsEnd{ ')' };													///< Окончание массива
	char plantumlFunctionArgsStart{ '(' };													///< Начало массива 

	char plantumlConditionEnd{ ')' };													///< Окончание выделения условия
	char plantumlConditionStart{ '(' };													///< Начало выделения условия 

	multi_variant_setting plantumlReferenceStart{"Вход"};									///< Начало ссылки 
	multi_variant_setting plantumlReferenceEnd{"Выход"};									///< Конец ссылки
};