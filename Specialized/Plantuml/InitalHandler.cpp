#include "InitalHandler.hpp"
#include"../../Common/StringFunction.hpp"
#include <format>

bool InitalHandler::read(PumlReader& stream, std::string& line)
{
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
        break;
    case PLANTUML_BLOCK_TYPE::REFERENCE:
        break;
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
            data.insert(data.end(), std::next(lines.begin(), index + 1), lines.end());
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
    if (lArray > expandSymbol || rArray < expandSymbol)
    {
        log(LOG_TYPE::ERROR, std::format("Раскрытие массива-строки \"{}\" в файле \"{}\" невозможно. Раскрытие происходит вне массива.", line, storage.back()->path));
        return{};
    }
    auto left = line.substr(lArray+1, expandSymbol - lArray -1);
    auto right = line.substr(expandSymbol + property.plantumlExpandSymbol.size(),
        rArray - (expandSymbol + property.plantumlExpandSymbol.size()) -1);
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
