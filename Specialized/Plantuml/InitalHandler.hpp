#pragma once
#include "Base/IInitalHandler.hpp"
#include "../../Plantuml/PumlReader.hpp"
#include "../../Common/Log/Logable.hpp"
#include "InitialHandlerData.hpp"
/**
///\brief Реализация первичного обработчика
*/
class InitalHandler : public IInitalHandler<Logable>
{
private:
	using storage_type = std::vector<std::shared_ptr<InitialHandlerData>>;
protected:
	InitialHandlerProperty property;							///< Свойства обработчика
	storage_type storage;	///< Хранилище данные

	/**
	///\brief Чтение из файла PlantUML
	///\param[in, out] stream Поток для чтения файла
	///\param[in, out] line Текущая строка
	///\return true, если чтение прошло успешно, false в противном случае
	*/
	bool read(PumlReader & stream, std::string& line);
	/**
	///\brief Подготовка файла для чтения
	///\param stream Поток для чтения файла
	*/
	void prepareFile(PumlReader& stream) const;
	/**
	///\brief Проверка, является ли строка игнорируемой
	///\param[in, out] line Строка для проверки
	///\return true, если строка игнорируемая, false в противном случае
	*/
	bool isIgnoredLine(std::string& line);
	/**
	///\brief Проверка, является ли строка комментарием
	///\param[in, out] stream Поток для чтения файла
	///\param[in, out] line Строка для проверки
	///\return true, если строка является комментарием, false в противном случае
	*/
	bool isCommentLine(PumlReader& stream, std::string& line);
	/**
	///\brief Проверка, является ли комментарий игнорируемым
	///\param[in] comment Строка для проверки
	///\return true, если комментарий игнорируемый, false в противном случае
	*/
	bool isIgnoredComment(const std::string& comment) const;
	/**
	///\brief Проверка, является ли строка блоком кода
	///\param[in, out] stream Поток для чтения файла
	///\param[in, out] line Строка для проверки
	///\return true, если строка является блоком, false в противном случае
   */
	bool isBlock(PumlReader& stream, std::string& line);
	/**
	///\rief Определение типа завершения блока кода
	///\param[in] line Строка для проверки
	///\return Тип завершения блока кода. В случае, если блок считан не полностью PLANTUML_BLOCK_TYPE::UNKNOWN
   */
	PLANTUML_BLOCK_TYPE isBlockEnd(const std::string& line) const;
	/**
	///\brief Проверка, является ли строка корректным блоком вычислением (+ считывание)
	///\param[in, out] line Строка для проверки
	///\return true, если строка является корректным блоком, false в противном случае
	*/
	bool isCalculation(std::string& line);	
	/**
	///\brief Поиск комментария в блоке вычислений
	///\param[in, out] lines Список строк с вычислением
	///\return Найденный комментарий, либо пустая строка
	*/
	std::string findCommentInCalculation(std::vector<std::string>& lines) const;
	/**
	///\brief Объединение единых вычислений, разделенных \n
	///\param[in, out] lines Список строк с вычислениями
	///\return Объединенный список строк
	*/
	std::vector<std::string> linkCalculation(std::vector<std::string>& lines);
	/**
	///\brief Раскрытие вычисления (Раскрытие символа plantumlExpandSymbol)
	///\param[in, out] lines Список строк с вычислением
	///\return true, если раскрытие прошло успешно, false в противном случае
	*/
	bool expandCalculation(std::vector<std::string>& lines);
	/**
	///\brief Раскрытие вычислений, происходящее не в массиве
	///\param[in] left Строка, от которой берутся начальные данные для раскрытия
	///\param[in] right Строка, от которой берутся конечные данные для раскрытия
	///\return Раскрытые вычисления, возвращает пустой массив, в случае неудачи
   */
	std::vector<std::string> expandSimple(std::string left, std::string right);	
	/**
	///\brief Разделение строки на две части по знаку присваивания
	///\param[in] line Строка для разделения
	///\param[out] assignReturn Знак присваивания, по которому происходило разделение
	///\return Пара строк - левая и правая части выражения от символа разделения
	*/
	std::pair<std::string, std::string> splitByAssign(const std::string& line, std::string& assignReturn) const;
	/**
	///\brief Раскрытие вычислений, происходящее в массиве
	///\param line Строка с массивом для раскрытия
	///\return Раскрытые вычисления, возвращает пустой массив, в случае неудачи
	*/
	std::vector<std::string> expandArray(std::string& line);
	/**
	///\brief Проверка, является ли строка корректным блоком функции (+ считывание)
	///\param[in, out] line Строка для проверки
	///\return true, если строка является корректным блоком, false в противном случае
	*/
	bool isFunction(std::string& line);
	/**
	///\brief Проверка, является ли строка корректным блоком функции (+ считывание)
	///\param[in] line Проверяемая строка
	///\param[in] functionData Проверяемая функция
	///\param[out] signature Сигнатура функции
	///\param[out] inputArgs Входные аргументы функции
	///\param[out] outputArgs Выходные аргументы функции
	///\return true, если строка корректна и удалось разбить функцию на составные части, false в противном случае
	*/
	bool splitFunctionData(const std::string& line, const std::string& functionData, std::string& signature, std::string& inputArgs, std::string& outputArgs);
	/**
	///\brief Проверка, является ли строка корректным блоком ссылки (+ считывание)
	///\param[in, out] line Строка для проверки
	///\return true, если строка является корректным блоком, false в противном случае
	*/
	bool isReference(std::string& line);
	/**
	///\brief Проверка, является ли корректным объявление начала ссылки
	///\param[in] refName Название ссылки
	///\return true, если является корректным блоком, false в противном случае
	*/
	bool isOkStart(const std::string& refName);
	/**
	///\brief Проверка, является ли корректным объявление окончания ссылки
	///\param[in] refName Название ссылки
	///\return true, если является корректным блоком, false в противном случае
	*/
	bool isOkEnd(const std::string& refName);
	/**
	///\brief Поиск последних данных об объявлении начала/конца ссылки
	///\return Итератор на инструкцию алгоритма, если объявление было найдено, иначе .end()
	*/
	decltype(InitialHandlerData::instruction)::const_iterator getFirstReferenceIter() const;
	/**
	///\brief Проверка на дублирование названия ссылки
	///\param[in] refName Название ссылки
	///\return true, если ссылка уже объявлена, false в противном случае
	*/
	bool isStartReferenceDuplication(const std::string& refName);
	/**
	///\brief Проверка является ли блок служебным
	///\param[in] stream Поток для чтения файла
	///\param[in] line Строка для проверки
	///\param[out] blockType Тип считанного блока
	///\return true, если блок является служебным, false в противном случае
	*/
	bool isServiceBlock(PumlReader& stream, std::string& line, PLANTUML_BLOCK_TYPE& blockType);
	/**
	///\brief Обработка и сохранение служебного блока
	///\param[in] line Строка со считанным блоком
	///\param[out] blockType Тип считанного блока
	///\return true, если обработка успешна, false в противном случае
	*/
	bool proceedServiceBlock(std::string& line, PLANTUML_BLOCK_TYPE blockType);
	/**
	///\brief Метод отделения текстового условия от кода
	///\param[in] line Строка со считанным блоком
	///\param[out] blockType Тип считанного блока
	///\return true, если отделение было успешным, false в противном случае
	*/
	bool proceedServiceCommentSeparator(std::string& line, PLANTUML_BLOCK_TYPE blockType);
	/**
	///\brief Проверка на считывание описания алгоритма (+ считывание)
	///\param[in] stream Поток для чтения файла
	///\param[in] line Строка для проверки
	///\return true, если считывается описание, false в противном случае
	*/
	bool isNote(PumlReader& stream, std::string& line);
	/**
	///\brief Считывание полного описания
	///\param[in] stream Поток для чтения файла
	///\param[in] line Строка для считывания
	///\return true, если считывание успешно, false в противном случае
	*/
	bool proceedFullNote(PumlReader& stream, std::string& line);
	/**
	///\brief Считывание сигнала из описания
	///\param[in] line Строка для считывания
	///\param[in] signalStorage Хранилище сигналов
	///\param[in] signalType Тип считываемого сигнала
	///\return true, если считывание успешно, false в противном случае
	*/
	bool proceedNoteSignal(std::string& line, InitialHandlerData::signal_storage_type* signalStorage,
		const RAW_SIGNAL_TYPE signalType);
public:
	// Унаследовано через IInitalHandler
	void init(const InitialHandlerProperty& newProperty) override;
	bool readPlantuml(const std::string& path) override;
	bool readPlantuml(const std::vector<std::string>& paths) override;
	void clearData() override;
};


