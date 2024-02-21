#pragma once

#include <cstdint>
/*
///\brief - ���� ����������� ��������
*/
enum class RAW_SIGNAL_TYPE : std::uint8_t
{
	UNKMOWN,			///< - ����������� ������
	INPUT_SIGNAL,		///< - ������� ������
	OUTPUT_SIGNAL,		///< - �������� ������
	INNER_SIGNAL,		///< - ���������� ������
	TIMER,				///< - ������
	CONST,				///< - ���������
	STRUCT				///< - ��������
};