#include "InitalHandler.hpp"
#include"../../Common/StringFunction.hpp"
#include <format>

bool InitalHandler::read(PumlReader& stream, std::string& line)
{
    if (line.empty())
        line = stream.getLine();
    bool isRead{ false};
    if (isIgnoredLine(line) || isCommentLine(stream, line))
    {
        line.clear();
        isRead = true;
    }
    else
    {
        if (line.front() == property.plantumlBlockStart)
            isRead = isBlock(stream, line);
        else
            isRead = true;
        line.clear();
    }
    return isRead;
}

void InitalHandler::prepareFile(PumlReader& stream) const
{
    //Удаление символов, мешающих парсингу
    stream.replaceInText("\n ", "\n");
    stream.replaceInText(" \n", "\n");
    stream.replaceInText("\n\n", "\n");
    stream.replaceInText("\r", "");
    stream.replaceInText("\t", " ");
    stream.replaceInText("%newline()", " ");
    stream.replaceInText("  ", " ");
}

bool InitalHandler::isIgnoredLine(std::string& line)
{
    return std::find(property.ignoredFirstSymbol.begin(), property.ignoredFirstSymbol.end(), line.front()) != property.ignoredFirstSymbol.end();
}

bool InitalHandler::isCommentLine(PumlReader& stream, std::string& line)
{
    std::string commentText;
    if (line.front() == property.plantumlOneLineComment)
        commentText = removeExtraSpacesFromText(line.substr(1));
    else if (line.find(property.plantumlMultilineCommentStart) == 0)
    {
        auto pos = line.find(property.plantumlMultilineCommentEnd);
        while (pos == std::string::npos)
        {
            line += " " + stream.getLine();
            pos = line.find(property.plantumlMultilineCommentEnd);
        }
        commentText = removeExtraSpacesFromText(line.substr(property.plantumlMultilineCommentStart.size(),
            line.size() - property.plantumlMultilineCommentStart.size() - property.plantumlMultilineCommentEnd.size()));   
    }
    else
        return false;
    if (isIgnoredComment(commentText) == false)
        storage.back()->instruction.emplace_back(new PlantumlRawBlock{ PLANTUML_BLOCK_TYPE::COMMENT, commentText });
    return true;
    
}

bool InitalHandler::isIgnoredComment(const std::string& comment) const
{
    return (comment.empty() || 
        std::find(property.ignoredComment.begin(), property.ignoredComment.end(), comment) != property.ignoredComment.end());
}

bool InitalHandler::isBlock(PumlReader& stream, std::string& line)
{
    auto type = isBlockEnd(line);
    while (type == PLANTUML_BLOCK_TYPE::UNKNOWN)
    {
        line += "\\n" + stream.getLine();       //используется "\\n" для соответствию тексту и более простому парсингу
        type = isBlockEnd(line);
    }
    switch (type)
    {
    case PLANTUML_BLOCK_TYPE::CALCULATION:
        return isCalculation(line);
    case PLANTUML_BLOCK_TYPE::FUNCTION:
        return isFunction(line);
    case PLANTUML_BLOCK_TYPE::REFERENCE:
        return isReference(line);
    default:
        return false;
    }
}

PLANTUML_BLOCK_TYPE InitalHandler::isBlockEnd(const std::string& line) const
{
    auto& lastChar{ line.back() };
    if (lastChar == property.plantumlBlockCalculationEnd)
        return PLANTUML_BLOCK_TYPE::CALCULATION;
    else if (lastChar == property.plantumlBlockFunctionEnd)
        return PLANTUML_BLOCK_TYPE::FUNCTION;
    else if (lastChar == property.plantumlBlockRefEnd)
        return PLANTUML_BLOCK_TYPE::REFERENCE;
    return PLANTUML_BLOCK_TYPE::UNKNOWN;
}
static std::vector<std::string> splitToVector(const std::string& text, const std::string& splitter)
{
    std::vector<std::string> vec;
    if (!text.empty())
    {
        std::string copy = text;
        auto pos = copy.find(splitter);
        while (pos != std::string::npos)
        {
            vec.emplace_back(copy.substr(0, pos));
            copy = copy.substr(pos + splitter.size());
            pos = copy.find(splitter);
        }
        if (copy.size())
            vec.emplace_back(copy);
    }
    return vec;
}
static std::vector<std::string> splitToVector(const std::string& text, const std::string& splitter, char startLine, char endLine)
{
    auto result = splitToVector(text, splitter);
    for (auto& el : result)
        el = startLine + el + endLine;
    return result;
}
std::string InitalHandler::findCommentInCalculation(std::vector<std::string>& lines) const
{
    for (auto iter = lines.begin(); iter != lines.end(); iter = std::next(iter))
    {
        for (auto& assign : property.plantumlAssignVariant)
        {
            auto isAssign = iter->find(assign) != std::string::npos;
            if (isAssign)
                if (iter == lines.begin())
                    return{};
                else
                {
                    std::string comment;
                    for (auto iter_ = lines.begin(); iter != iter_; iter_ = std::next(iter_))
                        comment += " " + *iter_;
                    lines.erase(lines.begin(), iter);
                    return removeExtraSpacesFromText(std::move(comment));
                }
        }
        
    }
    return {};
}
std::vector<std::string> InitalHandler::linkCalculation(std::vector<std::string>& lines)
{
    std::vector<std::string> linked{ std::move(lines.front())};
    for (auto iter = std::next(lines.begin()); iter != lines.end(); iter = std::next(iter))
    {
        bool isAssign{ iter->find(property.plantumlExpandSymbol) != std::string::npos };
        for (auto& assign : property.plantumlAssignVariant)
            isAssign |= iter->find(assign) != std::string::npos;
        if (isAssign)
            linked.emplace_back(std::move(*iter));
        else
            linked.back() += " " + std::move(*iter);
    }
    return linked;
}
static std::int32_t findExpandIndex(const std::vector<std::string>& lines, const std::string& expandSymbol)
{
    std::int32_t index{ -1 };
    for (size_t i = 0; i < lines.size(); ++i)
        if (lines.at(i).find(expandSymbol) != std::string::npos)
        {
            index = static_cast<decltype(index)>(i);
            break;
        }
    return index;
}
bool InitalHandler::expandCalculation(std::vector<std::string>& lines)
{
    auto index = findExpandIndex(lines, property.plantumlExpandSymbol);
    while (index != -1)
    {
        //Раскрываются вычисления вида\nL0 = 0\n...\nL5 = 0
        if (lines.at(index) == property.plantumlExpandSymbol)
        {
            if (index == 0 || index == (lines.size() - 1))
            {
                log(LOG_TYPE::ERROR,std::format("Раскрытие строки \"{}\" в файле \"{}\" невозможно. Отсутствует один из аргументов для расширения", join(lines), storage.back()->path));
                return false;
            }
            auto expanded{ expandSimple(lines.at(index - 1), lines.at(index + 1)) };
            if (expanded.empty())
                return false;
            std::vector<std::string> data{ lines.begin(), std::next(lines.begin(), index -1)};
            data.insert(data.end(), std::make_move_iterator(expanded.begin()), std::make_move_iterator(expanded.end()));
            data.insert(data.end(), std::next(lines.begin(), index + 2), lines.end());
            std::swap(lines, data);
        }
        else
        {
            //Раскрываются вычисления вида\nL0[0...32] = 0
            auto expanded{ expandArray(lines.at(index)) };
            if (expanded.empty())
                return false;
            std::vector<std::string> data{ lines.begin(), std::next(lines.begin(), index) };
            data.insert(data.end(), std::make_move_iterator(expanded.begin()), std::make_move_iterator(expanded.end()));
            data.insert(data.end(), std::next(lines.begin(), index+1), lines.end());
            std::swap(lines, data);
        }
        index = findExpandIndex(lines, property.plantumlExpandSymbol);
    }
    return true;
}
std::vector<std::string> InitalHandler::expandArray(std::string& line)
{
    auto expandSymbol{ line.find(property.plantumlExpandSymbol)},
        lArray{ line.find(property.plantumlArrayStart) },
        rArray{ line.find(property.plantumlArrayEnd) };
    if (lArray == std::string::npos || rArray == std::string::npos)
    {
        log(LOG_TYPE::ERROR, std::format("Раскрытие массива-строки \"{}\" в файле \"{}\" невозможно. Отсутствует корректное обращение в массиве.", line, storage.back()->path));
        return{};
    }
    //+ отлавливает случайное попадание некорректных записей
    if (lArray > expandSymbol || rArray < expandSymbol)
    {
        log(LOG_TYPE::ERROR, std::format("Раскрытие массива-строки \"{}\" в файле \"{}\" невозможно. Раскрытие происходит вне массива.", line, storage.back()->path));
        return{};
    }
    auto left = line.substr(lArray+1, expandSymbol - lArray -1);
    auto right = line.substr(expandSymbol + property.plantumlExpandSymbol.size(),
        rArray - (expandSymbol + property.plantumlExpandSymbol.size()));
    std::int32_t lIndex, rIndex;
    try
    {
        lIndex = std::stoi(left);
        rIndex = std::stoi(right);
    }
    catch (...)
    {
        log(LOG_TYPE::ERROR, std::format("Раскрытие массива-строки \"{}\" в файле \"{}\" невозможно. Индекс не является значением.", line, storage.back()->path));
        return{};
    }
    if (lIndex > rIndex)
        std::swap(lIndex, rIndex);
    std::string l{ line .substr(0, lArray+1)},
        r{ line.substr(rArray) };
    std::vector<std::string> result;
    for (std::int32_t i = lIndex; i <= rIndex; ++i)
    {
        result.emplace_back(l + std::to_string(i) + r);
        if (result.back().find(property.plantumlExpandSymbol) != std::string::npos)
        {
            log(LOG_TYPE::ERROR, std::format("Раскрытие массива-строки \"{}\" в файле \"{}\" невозможно. Строка не может содержать несколько раскрытий.", line, storage.back()->path));
            return{};
        }
    }
    
    return result;
}
bool InitalHandler::isFunction(std::string& line)
{
    removeSequence(line, "\\n", " ");
    removeSequence(line, "\n", " ");
    removeSequence(line, property.plantumlInlineCommentMarker, " ");
    removeExtraSpacesFromText(line);
    if (std::count(line.begin(), line.end(), property.plantumlFunctionCallSymbol) % 2 != 0)
    {
        log(LOG_TYPE::ERROR, std::format("Блок вызова функции \"{}\" содержит нечетное кол-во символов вызова функции. Файл \"{}\"", line, storage.back()->path));
        return false;
    }
    auto splittedData{ splitToVector(line.substr(1, line.size() -2), {property.plantumlFunctionCallSymbol})};
    bool isAllFunctionOk{ true };
    for (decltype(splittedData)::size_type i = 0; i < splittedData.size(); ++i)
    {
        if (i % 2 == 0)
        {
            removeExtraSpacesFromText(splittedData.at(i));
            //Не сохраняем пустые комментарии.
            if (splittedData.at(i).size())
                storage.back()->instruction.emplace_back(new PlantumlRawBlock{ PLANTUML_BLOCK_TYPE::COMMENT, splittedData.at(i) });
            continue;
        }
        auto& function = splittedData.at(i);
        bool shouldBeArgs = function.find(property.plantumlFunctionArgSeparator) != std::string::npos;
        bool isArgs = function.find(property.plantumlFunctionArgsEnd) != std::string::npos ||
            function.find(property.plantumlFunctionArgsStart) != std::string::npos;
        if (isArgs != shouldBeArgs)
        {
            if (shouldBeArgs == false)
                log(LOG_TYPE::ERROR, std::format("Блок вызова функции \"{}\" содержит содержит аргументы в \"{}\" без разделителя аргументов. Файл \"{}\"", line, function, storage.back()->path));
            else
                log(LOG_TYPE::ERROR, std::format("Блок вызова функции \"{}\" содержит не содержит аргументы в \"{}\", но присутствует разделитель аргументов. Файл \"{}\"", line, function, storage.back()->path));
            isAllFunctionOk = false;
            continue;
        }
        std::string inArgs, outArgs, signature{ function };
        if (isArgs)
        {
            if (splitFunctionData(line, function, signature, inArgs, outArgs) == false)
            {
                isAllFunctionOk = false;
                continue;
            }
        }
        storage.back()->instruction.emplace_back(new PlantumlRawFunctionBlock{ PLANTUML_BLOCK_TYPE::FUNCTION, signature, inArgs, outArgs });
    }
    return isAllFunctionOk;
}
bool InitalHandler::splitFunctionData(const std::string& line, const std::string& functionData, std::string& signature, std::string& inputArgs, std::string& outputArgs)
{
    if (checkStringBalance(functionData, property.plantumlFunctionArgsStart, property.plantumlFunctionArgsEnd) != 0)
    {
        log(LOG_TYPE::ERROR, std::format("Блок вызова функции \"{}\" в \"{}\" отсутствует баланс скобок в сигнатуре. Файл \"{}\"", line, functionData, storage.back()->path));
        return false;
    }
    if (std::count(functionData.begin(), functionData.end(), property.plantumlFunctionArgsStart) != 2)
    {
        log(LOG_TYPE::ERROR, std::format("Блок вызова функции \"{}\" в \"{}\" некорректное кол-во выделений аргументов. Файл \"{}\"", line, functionData, storage.back()->path));
        return false;
    }
    std::string args{ functionData.substr(functionData.find(property.plantumlFunctionArgsStart)) };
    signature = removeExtraSpacesFromText(functionData.substr(0, functionData.find(property.plantumlFunctionArgsStart)));
    bool isNotCorrectSignature = signature.find(property.plantumlFunctionArgSeparator) != std::string::npos ||
        signature.find(property.plantumlFunctionArgsEnd) != std::string::npos ||
        signature.find(property.plantumlFunctionArgsStart) != std::string::npos;
    if (isNotCorrectSignature)
    {
        log(LOG_TYPE::ERROR, std::format("Блок вызова функции \"{}\" в \"{}\" содержит некорректную сигнатуру \"{}\". Файл \"{}\"", line, functionData, signature, storage.back()->path));
        return false;
    }
    inputArgs = removeExtraSpacesFromText(args.substr(0, args.find(property.plantumlFunctionArgSeparator)));
    outputArgs = removeExtraSpacesFromText((args.substr(args.find(property.plantumlFunctionArgSeparator) + property.plantumlFunctionArgSeparator.size())));
    inputArgs = inputArgs.substr(1, inputArgs.size() - 2);
    outputArgs = outputArgs.substr(1, outputArgs.size() - 2);
    return true;
}
std::pair<std::string, std::string> InitalHandler::splitByAssign(const std::string& line, std::string& assignReturn) const
{
    std::size_t index{0};
    for (auto& assign : property.plantumlAssignVariant)
    {
        index = line.find(assign);
        if (index != std::string::npos)
        {
            assignReturn = assign;
            return std::make_pair(removeExtraSpacesFromText(line.substr(0, index)),
                removeExtraSpacesFromText(line.substr(index + 1+assign.size())));;
        }
    }
    return {};
}

std::vector<std::string> splitExpanded(std::string& data)
{
    std::vector<std::string> result{ std::string{ data.front()} };
    for (std::vector<std::string>::size_type i = 1; i < data.size(); ++i)
    {
        if (_isdigit(result.back().back()) == _isdigit(data.at(i)))
            result.back() += data.at(i);
        else
            result.emplace_back(std::string{ data.at(i) });
    }
    return result;
}
std::vector<std::string> InitalHandler::expandSimple(std::string left, std::string right)
{
    std::string lAssign, rAssign;
    auto leftData{ splitByAssign(left,lAssign) };
    auto rightData{ splitByAssign(right,rAssign) };
    if (leftData.first.empty())
        log(LOG_TYPE::ERROR, std::format("Невозможно раскрыть строку \"{}\", Файл \"{}\"", left, storage.back()->path));
    if (rightData.first.empty())
        log(LOG_TYPE::ERROR, std::format("Невозможно раскрыть строку \"{}\", Файл \"{}\"", right, storage.back()->path));
    if (lAssign != rAssign)
        log(LOG_TYPE::ERROR, std::format("Строки \"{}\" и \"{}\" имеют разное присвоение, Файл \"{}\"", left, right, storage.back()->path));
    if (leftData.second != rightData.second)
        log(LOG_TYPE::ERROR, std::format("Строки \"{}\" и \"{}\" имеют разные правые части, Файл \"{}\"", left, right, storage.back()->path));
    if (leftData.first.empty() || rightData.first.empty() ||
        lAssign != rAssign || leftData.second != rightData.second)
        return {};
    auto splittedLData{ splitExpanded(leftData.first) }, splittedRData{ splitExpanded(rightData.first) };
    if (splittedLData.size() != splittedRData.size())
    {
        log(LOG_TYPE::ERROR, std::format("Строки \"{}\" и \"{}\" имеют несколько изменяемых частей, Файл \"{}\"", left, right, storage.back()->path));
        return {};
    }
    std::int32_t index{-1};
    for (size_t i = 0; i < splittedLData.size(); ++i)
    {
        if (splittedLData.at(i) != splittedRData.at(i))
        {
            if (index != -1)
            {
                log(LOG_TYPE::ERROR, std::format("Строки \"{}\" и \"{}\" имеют несколько изменяемых частей, Файл \"{}\"", left, right, storage.back()->path));
                return {};
            }
            else
                index = static_cast<std::int32_t>(i);
        }
    }
    if (index == 0)
    {
        log(LOG_TYPE::ERROR, std::format("Строки \"{}\" и \"{}\" не могут иметь изменяемый начальный символ, Файл \"{}\"", left, right, storage.back()->path));
        return {};
    }
    if (index == -1)
    {
        log(LOG_TYPE::ERROR, std::format("Строки \"{}\" и \"{}\" не имеют изменяемых частей, Файл \"{}\"", left, right, storage.back()->path));
        return {};
    }
    if (!_isdigit(splittedLData.at(index).front()))
        log(LOG_TYPE::ERROR, std::format("Невозможно раскрыть строку \"{}\" изменяется не цифровое значение \"{}\", Файл \"{}\"", left, splittedLData.at(index), storage.back()->path));
    if (!_isdigit(splittedRData.at(index).front()))
        log(LOG_TYPE::ERROR, std::format("Невозможно раскрыть строку \"{}\" изменяется не цифровое значение \"{}\", Файл \"{}\"", right, splittedRData.at(index), storage.back()->path));
    if (!_isdigit(splittedRData.at(index).front()) || !_isdigit(splittedLData.at(index).front()))
        return {};
    std::int32_t lIndex = std::stoi(splittedLData.at(index)),
        rIndex = std::stoi(splittedRData.at(index));
    if (lIndex > rIndex)
        std::swap(lIndex, rIndex);

    std::vector<std::string> result;
    std::string l{ join({splittedLData.begin(), std::next(splittedLData.begin(), index)})},
        r{ join({std::next(splittedLData.begin(), index +1), splittedLData.end()}) + lAssign + " " + leftData.second};
    for (std::int32_t i = lIndex; i <= rIndex; ++i)
        result.emplace_back(l+ std::to_string(i) + " " + r);
    return result;
}
bool InitalHandler::isCalculation(std::string& line)
{
    line = line.substr(1, line.size() - 2);
    auto splitted = splitToVector(line, property.plantumlInlineCommentMarker, property.plantumlBlockStart, property.plantumlBlockCalculationEnd);
    if (splitted.size() != 1)
    {
        bool isReaded{ true };
        for (auto& splittedLine : splitted)
            isReaded &= isCalculation(splittedLine);
        return isReaded;
    }
    splitted = splitToVector(line, "\\n");
    auto comment = findCommentInCalculation(splitted);
    if (comment.size())
        storage.back()->instruction.emplace_back(new PlantumlRawBlock{ PLANTUML_BLOCK_TYPE::COMMENT, comment });
    if (splitted.empty())
        return true;
    splitted = linkCalculation(splitted);
    if (expandCalculation(splitted) == false)
        return false;
    for (auto& calculation : splitted)
        storage.back()->instruction.emplace_back(new PlantumlRawBlock{ PLANTUML_BLOCK_TYPE::CALCULATION, calculation });
    return true;
}

static bool tryToFindReference(const std::string& src, const InitialHandlerProperty::multi_variant_setting& variants, std::string& refName)
{
    for (auto& variant : variants)
        if (src.find(variant) != std::string::npos)
        {
            refName = variant;
            return true;
        }
    return false;
}

bool InitalHandler::isReference(std::string& line)
{
    std::string ref;
    PLANTUML_BLOCK_TYPE referenceType{ PLANTUML_BLOCK_TYPE::REFERENCE };
    bool (InitalHandler:: *checkFunction)(const std::string&) = nullptr;
    if (tryToFindReference(line, property.plantumlReferenceStart, ref))
    {
        checkFunction = &InitalHandler::isOkStart;
        referenceType = PLANTUML_BLOCK_TYPE::START_REFERENCE;
    }
    else if (tryToFindReference(line, property.plantumlReferenceEnd, ref))
    {
        referenceType = PLANTUML_BLOCK_TYPE::END_REFERENCE;
        checkFunction = &InitalHandler::isOkEnd;
    }
    if (checkFunction != nullptr)
    {
        
        const std::uint32_t offset = 1 + static_cast<std::uint32_t>(ref.size());
        line = removeExtraSpacesFromText(line.substr(offset, line.size() - offset - 1));
        if ((this->*checkFunction)(line) == false)
            return false;
    }
    else
        line = removeExtraSpacesFromText(line.substr(1, line.size() - 2));
    this->storage.back()->instruction.emplace_back(new PlantumlRawBlock{ referenceType, line });
    return true;
}

bool InitalHandler::isOkStart(const std::string& refName)
{
    auto lastReferenceData = getFirstReferenceIter();
    if (lastReferenceData == storage.back()->instruction.end())
    {
        if (refName.empty())
            return !isStartReferenceDuplication(refName);
        log(LOG_TYPE::ERROR, std::format("Алгоритм должен начинаться с ссылки без имени. Файл \"{}\"", storage.back()->path));
    }
    else
    {
        if (lastReferenceData->get()->type != PLANTUML_BLOCK_TYPE::START_REFERENCE)
            return !isStartReferenceDuplication(refName);
        log(LOG_TYPE::ERROR, std::format("Алгоритм \"{}\" имеет незакрытую ссылку \"{}\" => Объявление ссылки \"{}\" невозможно. Файл \"{}\"",
            storage.back()->algorithmName, dynamic_cast<PlantumlRawBlock*>(lastReferenceData->get())->text, refName, storage.back()->path));
    }
    return false;
}

bool InitalHandler::isOkEnd(const std::string& refName)
{
    auto lastReferenceData = getFirstReferenceIter();
    if (lastReferenceData == storage.back()->instruction.end())
    {
        log(LOG_TYPE::ERROR, std::format("Алгоритм \"{}\" не имеет начальной ссылки => Закрыть ссылку невозможно. Файл \"{}\"",
            storage.back()->algorithmName, storage.back()->path));
    }
    else
    {
        if (lastReferenceData->get()->type != PLANTUML_BLOCK_TYPE::START_REFERENCE)
        {
            log(LOG_TYPE::ERROR, std::format("Алгоритм \"{}\" имеет незакрытую ссылку \"{}\" => Объявление закрытия ссылки \"{}\" невозможно. Файл \"{}\"",
                storage.back()->algorithmName, dynamic_cast<PlantumlRawBlock*>(lastReferenceData->get())->text, refName, storage.back()->path));
        }
        else
        {
            if (dynamic_cast<PlantumlRawBlock*>(lastReferenceData->get())->text == refName)
                return true;
            log(LOG_TYPE::ERROR, std::format("Алгоритм \"{}\". Невозможно закрыть ссылку \"{}\" ссылкой \"{}\" => Пропущено окончание ссылки \"{}\". Файл \"{}\"",
                storage.back()->algorithmName, dynamic_cast<PlantumlRawBlock*>(lastReferenceData->get())->text, refName, 
                storage.back()->algorithmName, dynamic_cast<PlantumlRawBlock*>(lastReferenceData->get())->text, storage.back()->path));
        }
    }
    return false;
}

decltype(InitialHandlerData::instruction)::const_iterator InitalHandler::getFirstReferenceIter() const
{
    if (storage.back()->instruction.empty())
        return storage.back()->instruction.end();
    decltype(InitialHandlerData::instruction)::const_reverse_iterator iter;
    for (iter = storage.back()->instruction.rbegin(); iter != storage.back()->instruction.rend(); ++iter)
        if (iter->get()->type == PLANTUML_BLOCK_TYPE::END_REFERENCE ||
            iter->get()->type == PLANTUML_BLOCK_TYPE::START_REFERENCE)
            break;
    return --(iter.base());;
}

bool InitalHandler::isStartReferenceDuplication(const std::string& refName)
{
    for (auto& instruction : storage.back()->instruction)
    {
        if (instruction->type == PLANTUML_BLOCK_TYPE::START_REFERENCE &&
            dynamic_cast<PlantumlRawBlock*>(instruction.get())->text == refName)
        {
            log(LOG_TYPE::ERROR, std::format("Алгоритм \"{}\" имеет многократное объявление ссылки \"{}\". Файл \"{}\"",
                storage.back()->algorithmName, refName, storage.back()->path));
            return true;
        }
    }
    return false;
}

void InitalHandler::init(const InitialHandlerProperty& newProperty)
{
    property = newProperty;
}

bool InitalHandler::readPlantuml(const std::string& path)
{
    PumlReader stream;
    if (stream.open(path) == false)
    {
        log(LOG_TYPE::ERROR, std::format("Невозможно открыть файл \"{}\"", path));
        return false;
    }
    else
        prepareFile(stream);
    std::string currentText;
    bool readingResult{ true };
    storage.emplace_back(new InitialHandlerData);
    storage.back()->path = path;
    while (stream.isEOF() == false)
        readingResult &= read(stream, currentText);
    return readingResult;
}

bool InitalHandler::readPlantuml(const std::vector<std::string>& paths)
{
    bool isOk{ true };
    for (auto& path : paths)
        isOk &= readPlantuml(path);
    return isOk;
}

void InitalHandler::clearData()
{
    storage.clear();
}
