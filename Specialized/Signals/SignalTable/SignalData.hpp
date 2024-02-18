#pragma once
#include "../../../Signals/Base/RawSignalData.hpp"
#include <unordered_map>
/*
///\brief - ���, �������� ��������� ���������� � �������
*/
struct SignalData : RawSignalData
{
	using element_count_type = std::uint32_t;
	bool isPtr;						///< - �������� �� ������ ����������
	element_count_type elementCount;		///< - ���-�� ���������
};
/*
///\brief - ���, �������� ��������� ���������� � ����������
*/
struct StructData : RawSignalData
{
	std::unordered_map<std::string, std::shared_ptr<SignalData>> fields;	///< - ������ � ����� ���������
};
