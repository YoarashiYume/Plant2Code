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
        {
            PLANTUML_BLOCK_TYPE blockType;
            if (isServiceBlock(stream, line, blockType))
                isRead = proceedServiceBlock(line, blockType);
            else if (isNote(stream, line))
                isRead = true;
        }
            
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
    for (auto& ignored : property.ignoredFirstSymbol)
        if (line.find(ignored) == 0)
            return true;
    return false;
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
        if (src.find(variant) == 0)
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
static bool tryToFindReference(const std::string& src, const InitialHandlerProperty::multi_variant_setting& variants)
{
    std::string _;
    return tryToFindReference(src, variants, _);
}

static bool isPostfix(const std::string& line, const std::string& postfix)
{
    if (line.length() >= postfix.length()) 
        return (0 == line.compare(line.length() - postfix.length(), postfix.length(), postfix));
    return false;
}
bool InitalHandler::isServiceBlock(PumlReader& stream, std::string& line, PLANTUML_BLOCK_TYPE& blockType)
{
    blockType = PLANTUML_BLOCK_TYPE::UNKNOWN;
    std::string variant;
    if (tryToFindReference(line, property.plantumlIf, variant))
        blockType = PLANTUML_BLOCK_TYPE::IF_START;
    else if (tryToFindReference(line, property.plantumlElseIf, variant))
        blockType = PLANTUML_BLOCK_TYPE::IF_ELSE_START;
    else if (tryToFindReference(line, property.plantumlElse, variant))
        blockType = PLANTUML_BLOCK_TYPE::ELSE_START;
    else if (tryToFindReference(line, property.plantumlEndIf))
        blockType = PLANTUML_BLOCK_TYPE::IF_END;
    else if (tryToFindReference(line, property.plantumlSwitch, variant))
        blockType = PLANTUML_BLOCK_TYPE::SWITCH_START;
    else if (tryToFindReference(line, property.plantumlCase, variant))
        blockType = PLANTUML_BLOCK_TYPE::CASE_START;
    else if (tryToFindReference(line, property.plantumlEndSwitch))
        blockType = PLANTUML_BLOCK_TYPE::SWITCH_END;
    else if (tryToFindReference(line, property.plantumlWhile, variant))
        blockType = PLANTUML_BLOCK_TYPE::WHILE_START;
    else if (tryToFindReference(line, property.plantumlEndWhile))
        blockType = PLANTUML_BLOCK_TYPE::WHILE_END;
    if (variant.size())
    {
        //Удаление префикса, в другом месте будет вызывать дублирование этой функции
        line = removeExtraSpacesFromText(line.substr(variant.size()));
        variant.clear();
    }
    std::string tempNewLine;
    std::string oldLine = line;

    switch (blockType)
    {
    case PLANTUML_BLOCK_TYPE::IF_START:
    case PLANTUML_BLOCK_TYPE::IF_ELSE_START:
        variant = property.plantumlIfPostfix;
        while (!isPostfix(line, variant) && !stream.isEOF())
        {
            line += " " + stream.getLine();
        }
        break;
    case PLANTUML_BLOCK_TYPE::ELSE_START:
        variant = property.plantumElsePostfix;
        while (!isPostfix(line, variant))
        {
            tempNewLine = stream.getLine();
            if (tryToFindReference(tempNewLine, property.plantumlEndWhile))
            {
                line += " " + tempNewLine;
                tempNewLine = stream.getLine();
            }
            line += " " + tempNewLine;
        }        
        break;
    case PLANTUML_BLOCK_TYPE::CASE_START:
    case PLANTUML_BLOCK_TYPE::SWITCH_START:
        while (!stream.isEOF())
        {
            auto count = checkStringBalance(line, property.plantumlConditionStart, property.plantumlConditionEnd);
            if (count == 0 && line.back() == property.plantumlConditionEnd)
                break;
            if (count < 0)
            {
                if (blockType == PLANTUML_BLOCK_TYPE::SWITCH_START)
                    log(LOG_TYPE::WARNING, std::format("Некорректное кол-во скобок в строке switch \"{}\" в файле \"{}\". Дальнейшая работа может быть некорректна",
                    line, storage.back()->path));
                else
                {
                    blockType = PLANTUML_BLOCK_TYPE::UNKNOWN;
                    log(LOG_TYPE::WARNING, std::format("Некорректное кол-во скобок в строке case \"{}\" в файле \"{}\"",
                        line, storage.back()->path));
                    return false;
                }

            }
            line += " " + stream.getLine();
        }
        break;
    case PLANTUML_BLOCK_TYPE::WHILE_START:
        variant = property.plantumWhilePostfix;
        while (!isPostfix(line, variant) && !stream.isEOF())
        {
            line += " " + stream.getLine();
        }
        break;
    case PLANTUML_BLOCK_TYPE::WHILE_END:
        variant = property.plantumEndWhilePostfix;
        while (!isPostfix(line, variant) && !stream.isEOF())
        {
            tempNewLine = stream.getLine();
            if (tryToFindReference(tempNewLine, property.plantumlElse))
            {
                line += " " + tempNewLine;
                tempNewLine = stream.getLine();
            }
            line += " " + tempNewLine;
        }
        break;
    default:
        break;
    }
    if (blockType != PLANTUML_BLOCK_TYPE::UNKNOWN && stream.isEOF())
    {
        log(LOG_TYPE::ERROR, std::format("Ошибка в считывании строки \"{}\". Файл \"{}\" не может быть прочитан.",
            oldLine, storage.back()->path));
    }
    else if (!stream.isEOF() && variant.size())
    {
        line = removeExtraSpacesFromText( line.substr(0, line.size() - variant.size()));
    }
    if (line.size() >= 2 && line.front() == property.plantumlConditionStart && line.back() == property.plantumlConditionEnd)
    {
        line = removeExtraSpacesFromText(line.substr(1, line.size() - 2));
    }
    removeSequence(line, "\\n");
    removeExtraSpacesFromText(line);
    return blockType != PLANTUML_BLOCK_TYPE::UNKNOWN && !stream.isEOF();
}

bool InitalHandler::proceedServiceBlock(std::string& line, PLANTUML_BLOCK_TYPE blockType)
{
    switch (blockType)
    {      
    
    case PLANTUML_BLOCK_TYPE::IF_ELSE_START:
    case PLANTUML_BLOCK_TYPE::ELSE_START:
        storage.back()->instruction.emplace_back(new PlantumlBlock{ PLANTUML_BLOCK_TYPE::IF_END });
    case PLANTUML_BLOCK_TYPE::IF_START:
        if (blockType != PLANTUML_BLOCK_TYPE::ELSE_START)
            return proceedServiceCommentSeparator(line, blockType);
        return true;
    case PLANTUML_BLOCK_TYPE::SWITCH_END:
        storage.back()->instruction.emplace_back(new PlantumlBlock{ PLANTUML_BLOCK_TYPE::CASE_END });
    case PLANTUML_BLOCK_TYPE::WHILE_END:    
    case PLANTUML_BLOCK_TYPE::IF_END:
        storage.back()->instruction.emplace_back(new PlantumlBlock{ blockType });
        return true;
    case PLANTUML_BLOCK_TYPE::SWITCH_START:
        storage.back()->instruction.emplace_back(new PlantumlRawBlock{ PLANTUML_BLOCK_TYPE::COMMENT, line });
        storage.back()->instruction.emplace_back(new PlantumlBlock{ blockType });
        return true;
    case PLANTUML_BLOCK_TYPE::CASE_START:
        if (storage.back()->instruction.back()->type != PLANTUML_BLOCK_TYPE::SWITCH_START)
            storage.back()->instruction.emplace_back(new PlantumlBlock{ PLANTUML_BLOCK_TYPE::CASE_END });
        storage.back()->instruction.emplace_back(new PlantumlRawBlock{ blockType, line });
        return true;
    case PLANTUML_BLOCK_TYPE::WHILE_START:
        return proceedServiceCommentSeparator(line, blockType);
    default:
        break;
    }
    return false;
}

bool InitalHandler::proceedServiceCommentSeparator(std::string& line, PLANTUML_BLOCK_TYPE blockType)
{
    auto separatorIndex = line.find(property.plantumlServiceCommentSeparator);
    if (separatorIndex == std::string::npos)
    {
        log(LOG_TYPE::ERROR, std::format("Служебный блок \"{}\" не имеет символа отделения \"{}\" условия. Файл \"{}\"",
            line, property.plantumlServiceCommentSeparator, storage.back()->path));
        return false;
    }
    storage.back()->instruction.emplace_back(new PlantumlRawBlock{ PLANTUML_BLOCK_TYPE::COMMENT, line.substr(0, separatorIndex+1) });
    storage.back()->instruction.emplace_back(new PlantumlRawBlock{ blockType,
        removeExtraSpacesFromText(line.substr(separatorIndex + 1)) });
    return true;
}

bool InitalHandler::isNote(PumlReader& stream, std::string& line)
{
    std::string variant;
    if (tryToFindReference(line, property.plantumlFastNote, variant))
        if (storage.back()->algorithmName.empty())
            storage.back()->algorithmName = removeExtraSpacesFromText(line.substr(variant.size()));
    if (tryToFindReference(line, property.plantumlFullNote))
        return proceedFullNote(stream, line);
    return false;
}

bool InitalHandler::proceedFullNote(PumlReader& stream, std::string& line)
{
    bool isDescription{ false };
    bool isOkRead{ true };
    RAW_SIGNAL_TYPE signalType{};
    InitialHandlerData::signal_storage_type* currentSignals{nullptr};
    while (!stream.isEOF())
    {
        line = removeExtraSpacesFromText(stream.getLine());
        if (line.find(property.plantumlEndFullNote) == 0)
        {
            //Дополнительная очистка, нужна для корректного поиска функии в дальнейшей работе
            removeExtraSpacesFromText(storage.back()->algorithmName);
            return isOkRead;
        }
        if (isDescription)
        {
            appendString(line, storage.back()->description, '\n');
            continue;
        }
        if ((tryToFindReference(line, property.plantumlNoteInputSignal)))
        {
            currentSignals = &storage.back()->inputSignal;
            signalType = RAW_SIGNAL_TYPE::INPUT_SIGNAL;
            continue;
        }
        else if ((tryToFindReference(line, property.plantumlNoteLocalSignal)))
        {
            currentSignals = &storage.back()->localSignal;
            signalType = RAW_SIGNAL_TYPE::INNER_SIGNAL;
            continue;
        }
        else if ((tryToFindReference(line, property.plantumlNoteOutputSignal)))
        {
            currentSignals = &storage.back()->outputSignal;
            signalType = RAW_SIGNAL_TYPE::OUTPUT_SIGNAL;
            continue;
        }
        else if ((tryToFindReference(line, property.plantumlNoteDescription)))
            isDescription = true;
        else
        {
            if (currentSignals == nullptr)
            {
                if (storage.back()->instruction.size() == 1)
                    appendString(line, storage.back()->algorithmName, ' ');
            }
            else
                isOkRead &= proceedNoteSignal(line, currentSignals, signalType);
        }


    }
    log(LOG_TYPE::ERROR, std::format("Невозможно считать описание в алгоритме. Файл \"{}\"", storage.back()->path));
    return false;
}

bool InitalHandler::proceedNoteSignal(std::string& line, InitialHandlerData::signal_storage_type* signalStorage,
    const RAW_SIGNAL_TYPE signalType)
{
    //Запись сигнала <Название типа сигнала (входной, локальный, выходной)>: <Тип сигнала> - <Описание сигнала>
    auto typeSeparatorPos = line.find(property.plantumlNoteSignalTypeSeparator);
    auto titleSeparator = line.find(property.plantumlNoteSignalTitleSeparator);
    //В таймере не проставляется тип
    if ((typeSeparatorPos == std::string::npos && signalType == RAW_SIGNAL_TYPE::INNER_SIGNAL) ||
        (typeSeparatorPos == std::string::npos && titleSeparator == std::string::npos))
    {
        if (signalStorage->empty())
        {
            log(LOG_TYPE::ERROR, std::format("Cигнал типа заполнен некорректно \"{}\" - отсутствует обозначение типа. Файл \"{}\"",
                line, storage.back()->path));
            return false;
        }
        appendString(line, signalStorage->back()->title, ' ');
        
    }
    else
    {
       
        std::string signalName, type;
        if (typeSeparatorPos == std::string::npos && signalType != RAW_SIGNAL_TYPE::INNER_SIGNAL)
            signalName = line.substr(0, titleSeparator);
        else if (typeSeparatorPos != std::string::npos)
        {
            signalName = removeExtraSpacesFromText(line.substr(0, typeSeparatorPos));
            type = removeExtraSpacesFromText((line.substr(typeSeparatorPos + 1, titleSeparator - (typeSeparatorPos + 1))));
        }
        else
        {
            log(LOG_TYPE::ERROR, std::format("Невозможно определить тип сигнала \"{}\". Файл \"{}\"", line, storage.back()->path));
            return false;
        }
        std::string title;
        if (titleSeparator != std::string::npos && titleSeparator < line.size())
            title = removeExtraSpacesFromText(line.substr(titleSeparator + 1));
        std::uint32_t elementCount{ 1 };
        auto arrayPosStart = type.find(property.plantumlArrayStart);
        auto arrayPosEnd = type.find(property.plantumlArrayEnd);
        if (arrayPosStart != std::string::npos)
        {
            if (arrayPosEnd == std::string::npos)
            {
                log(LOG_TYPE::ERROR, std::format("Cигнал \"{}\" имеет некорректный тип \"{}\" - нет символа конца размера массива. Файл \"{}\"", signalName, type, storage.back()->path));
                return false;
            }
            std::string count = type.substr(arrayPosStart+1, arrayPosEnd- arrayPosStart-1);
            try
            {
                elementCount = static_cast<std::uint32_t>(std::stoul(count));
            }
            catch (...)
            {
                log(LOG_TYPE::ERROR, std::format("Cигнал \"{}\" имеет некорректный тип \"{}\" - размер массива не является числом. Файл \"{}\"", signalName, type, storage.back()->path));
                return false;
            }
            type = type.substr(0, arrayPosStart);
        }
        else if (arrayPosEnd != std::string::npos)
        {
            log(LOG_TYPE::ERROR, std::format("Cигнал \"{}\" имеет некорректный тип \"{}\" - нет символа начала размера массива. Файл \"{}\"", signalName, type,storage.back()->path));
            return false;
        }
        auto& copy = signalStorage->emplace_back(new InitialHandlerData::signal_type::element_type{});
        copy->type = type;
        copy->isPtr = arrayPosStart != std::string::npos;
        copy->elementCount = elementCount;
        copy->title = title;
        copy->signalType = signalType;
    }
    return true;
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
