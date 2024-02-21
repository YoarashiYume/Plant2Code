#pragma once
#include <string>
#include <vector>

struct PrefixData
{
	using prefix_variant_type = std::vector<std::string>;
	using ignore_variant_type = std::vector<std::string>;
	prefix_variant_type inputSignalPrefix;		///< - �������� ������� ��������
	prefix_variant_type outputSignalPrefix;		///< - �������� �������� ��������
	prefix_variant_type innerSignalPrefix;		///< - �������� ���������� ��������
	prefix_variant_type constPrefix;			///< - �������� ��������
	prefix_variant_type timerSignalPrefix;		///< - �������� ��������
	prefix_variant_type structSignalPrefix;		///< - �������� ��������
	ignore_variant_type ignoreVariant;			///< - �������� � �������� ������� ������������
};

/*
///\brief - ���������, �������� �������� ��� ������ �������.
*/
struct SignalReadProperty
{
	using col_variant_type = std::vector<std::string>;
	using prefix_type = PrefixData;

	char separatorCSV;							///< - ������ ���������� ��� �������� CSV-�������
	col_variant_type variablesColNameVariant;	///< - �������� ���������� ������� ��� �������� ����������
	col_variant_type variablesColTitleVariant;	///< - �������� ���������� ������� ��� �������� �������
	col_variant_type variablesColTypeVariant;	///< - �������� ���������� ������� ��� ���� ����������
	col_variant_type variablesColValueVariant;	///< - �������� ���������� ������� ��� �������� ����������
	

	prefix_type prefixes;						///< - �������� ��������
};