#pragma once

#include "../../Common/Log/Base/ILogable.hpp"
#include "RawSignalData.hpp"
#include "../Propery/SignalReadProperty.hpp"

#include <type_traits>
#include <memory>

/**
///\brief - ������������ ������ ��� ���������� ��������
///\tparam T - ��� ����������� ��������, ������� ������ ���� ����������� �� ILogable
 */
template<typename T,
	typename = std::enable_if<std::is_base_of_v<ILogable, T>>>
class ISignalReader : public T
{
public:
	/**
	///\brief - ��������� ���� ������� �� ���������� ����
	///\param [in] path - ���� � ����� �������
	///\return - true, ���� ���� ��� ������� ������, false � ��������� ������
	*/
	virtual bool open(const std::string& path) = 0;
	/**
	///\brief - ���������, ��� �� ��������� ����� ����� (EOF)
	///\return - true, ���� EOF ��� ���������, false � ��������� ������
	*/
	virtual bool isEOF() const = 0;
	/**
	///\brief - �������������� ������ ������� ��� ��������������� ������
	///\return - true, ���� ������������� ���� ��������, false � ��������� ������
	*/
	virtual bool initCSVHeader(const SignalReadProperty& property) = 0;
	/**
	/// \brief - ���������� ��������� ������ �� �����
	/// \return - ��������� �� ������ RawReadData � ����������� � �������
	*/
	virtual std::shared_ptr<RawReadData> getNextSignal() = 0;

	virtual ~ISignalReader() = default;
};
