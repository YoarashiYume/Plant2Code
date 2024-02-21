#pragma once
#include <memory>
#include <unordered_map>
#include "SignalData.hpp"
#include "../../../Common/Log/Logable.hpp"
/**
///\brief ����� ��� �������� ������� ��������
*/
class SignalTable : public Logable
{
public:
	using key_type = std::string;
	using store_type = std::shared_ptr<RawReadData>;
	using storage_type = std::unordered_map<key_type, store_type>;

	const char structSeparator = '.';	///< - ������ ���������� ���������
	const char arrayStart = '[';		///< - ������ ������ �������
	const char arrayEnd = ']';			///< - ������ ������ �������
	const std::string arraySize = "..";	///< - ������ ������� ������� � �������� �����������

protected:
	storage_type inputSignals;			///< - ������ � ������� ��������
	storage_type outputSignals;			///< - ������ � �������� ��������
	storage_type innerValues;			///< - ������ � ���������� ��������
	storage_type constValues;			///< - ������ � ����������
	storage_type timers;				///< - ������ � ��������
	storage_type structData;			///< - ������ � ����������
	storage_type unknownSignals;		///< - ������ � ����������� ��������

	
	
	/**
	///\brief �������� ������ � ���������
	///\param[in, out] newSignal ����������� ������
	///\return true ���� ���������� �������, ����� false
	*/
	bool addStructData(store_type newSignal);
	/**
	///\brief �������� ������ � �������
	///\param[in, out] newSignal ����������� ������
	///\param[in, out] storage ��������� ������
	///\param[in] realName �������� ��� ������������ �������
	///\return true ���� ���������� �������, ����� false
	*/
	bool addSignalData(store_type newSignal, storage_type& storage, const std::string realName);
	/**
	///\brief ��������������� ������ �������
	///\param[in, out] rawSignal �������� ������
	///\param[in] isPtr �������� �� ������ ����������
	///\param[in] elementCount ���-�� ��������� � �������
	///\return ����������� ������
	*/
	store_type createFullSignal(const store_type& rawSignal, bool isPtr = false, SignalData::element_count_type elementCount = 1) const;
	/**
	///\brief ������� ��������� �� ����
	///\param[in] oldType ������ ���
	///\return std::string ��� ��� ���������
	*/
	std::string removePtrFromType(const std::string oldType);
	/**
	///\brief �������� ���������� ��������� �� �����
	///\param[in, out] name ��� �������
	///\param[in] realName �������� ��� �������
	///\return ���������� ��������� � ����� �������
	*/
	SignalData::element_count_type getElementCountFromName(std::string& name, const std::string realName);
	/**
	///\brief �������� ���������� ��������� �� ����
	///\param[in, out] type ��� �������
	///\param[in] realName �������� ��� �������
	///\return ���������� ��������� � ���� �������
	*/
	SignalData::element_count_type getElementCount(std::string& type, const std::string name);
	/**
	///\brief �������� ���������� ��������� �� �������, ������������ ����� �������
	///\param[in, out] name ������ �������
	///\param[in] realName �������� ��� �������
	///\return ���������� ��������� � ���� �������
	*/
	SignalData::element_count_type getElementCountFromOneLinesSignal(std::string& name, const std::string realName);
	/**
	///\brief �������� � ��������� ������������� �������, ��� ���������� ����������
	///\param[in, out] newSignal ����� ������
	///\param[in, out]] oldSignal ������������ ������
	///\param[in] newElementCount ����� ���-�� ���������
	///\return ���������� ��������� � ���� �������
	*/
	bool checkExistSignal(RawSignalData* newSignal, SignalData* oldSignal, SignalData::element_count_type newElementCount);
public:
	/**
	///\brief �������� ����� ������
	///\param newSignal ����� ������
	///\return true ���� ���������� �������, ����� false
	*/
	bool addSignal(store_type newSignal);
	/**
	///\brief �������� ������ �� �����
	///\param key ���� �������
	///\return ������ �� �����
	*/
	store_type getSignal(const key_type& key);
};

