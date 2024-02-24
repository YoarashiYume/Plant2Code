#include "PumlReader.hpp"
#include "../Common/File.hpp"
#include "../Common/StringFunction.hpp"
#include <algorithm>
bool PumlReader::saveData()
{
	unique_ifstream_type stream{ new std::ifstream{ currentPath }, closeFunction };
	if (stream->is_open())
	{
		std::string line;
		while (std::getline(*stream, line))
		{
			if (line.empty())
				continue;
			if (data.size())
				data += '\n';
			removeExtraSpacesFromText(line);
			data += line;
		}
		return true;
	}
	return false;
}

std::string PumlReader::getLineFromFile()
{
	if (data.size())
	{
		fileData.str(std::move(data));
		data.clear();
	}
	std::string line;
	std::getline(fileData, line);
	return line;
}

bool PumlReader::open(const std::string& path)
{
	currentPath = path;
	return saveData();
}

bool PumlReader::isEOF() const
{
	return fileData.eof();
}

void PumlReader::replaceInText(const std::string& oldSubstring, const std::string& newSubstring)
{
	auto pos = data.find(oldSubstring);
	while (pos != std::string::npos) 
	{
		data.replace(pos, oldSubstring.length(), newSubstring);
		pos = data.find(oldSubstring);
	}
}


std::string PumlReader::getPath() const
{
	return currentPath;
}
