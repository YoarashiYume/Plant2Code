#pragma once
#include "Base/ISignalReader.hpp"
#include "../Common/Log/Logable.hpp"
#include "../Common/File.hpp"

#include <istream>

class SignalReader : public ISignalReader<Logable>
{
public:
	using col_index_type = std::int32_t;
	using prefix_type = SignalReadProperty::prefix_type;
protected:
	std::string currentPath;		///< - ������� ���� � �����
	unique_ifstream_type stream;	///< - ����� ������ �� �����
	col_index_type typeIndex,		///< - ������ ������� � ����� �������
		nameIndex,					///< - ������ ������� � ��������� ����������
		titleIndex,					///< - ������ ������� � ��������� �������
		valueIndex;					///< - ������ ������� �� ��������� ���������
	prefix_type prefixData;			///< - ������ ��������
	char separator;					///< - ����������� CSV �������

	/**
	///\brief - ���������� ������ � ��������� ��������
	///\param [in] other - ������������ ������
	///\return - true, ���� ���� ��� ������� ������, false � ��������� ������
	*/
	void copyPrefixData(const prefix_type& other);
	/**
	///\brief - ���������� ��������� �� ������ ��� ������������� header`a �������
	///\return - ������ � ������� ������
	*/
	std::string defineInitError() const;
	/**
	///\brief - ���������� ������ ������ �� ��������� �������� ��� ���������
	///\param [in] variants - �������� �������� �������
	///\param [in] header - Header �������
	///\return - ������ ������� � ������������� �������, � ������ ������� -1
	*/
	std::int32_t defineData(const SignalReadProperty::col_variant_type& variants, const std::vector<std::string>& header) const;
	/**
	///\brief - ���������� ��� PlantUML �������
	///\param [in] plantumlType - PlantUML ������
	///\return - ��� �������, � � ������ ������� RAW_SIGNAL_TYPE::UNKMOWN
	*/
	RAW_SIGNAL_TYPE defineSignalType(const std::string& plantumlType) const;
	/**
	///\brief - ��������� ������� ����� ������� ��� �������� �������
	///\param [in] plantumlType - PlantUML ������
	///\param [in] variants - �������� �������� ���� �������
	///\return - true, ���� ������ �������� �������� ����, false � ��������� ������
	*/
	bool checkPrefix(const std::string& plantumlType, const prefix_type::prefix_variant_type& variants) const;

private:
	/**
	///\brief - ���������� ������
	*/
	void resetData();

public:

	SignalReader();

	/**
	///\brief - ��������� ���� ������� �� ���������� ����
	///\param [in] path - ���� � ����� �������
	///\return - true, ���� ���� ��� ������� ������, false � ��������� ������
	*/
	bool open(const std::string& path) override;
	/**
	///\brief - ���������, ��� �� ��������� ����� ����� (EOF)
	///\return - true, ���� EOF ��� ���������, false � ��������� ������
	*/
	bool isEOF() const override;
	/**
	///\brief - �������������� ������ ������� ��� ��������������� ������
	///\return - true, ���� ������������� ���� ��������, false � ��������� ������
	*/
	bool initCSVHeader(const SignalReadProperty& property) override;
	/**
	/// \brief - ���������� ��������� ������ �� �����
	/// \return - ��������� �� ������ RawReadData � ����������� � �������
	*/
	std::shared_ptr<RawReadData> getNextSignal() override;
};

