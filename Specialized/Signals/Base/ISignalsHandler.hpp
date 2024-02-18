#pragma once
#include "../../../Signals/Base/ISignalReader.hpp"

template<typename T, typename U,
	typename = std::enable_if<std::is_base_of_v<ILogable, T>>,
	typename = std::enable_if<std::is_base_of_v<ISignalReader<ILogable>, U>>
>
class ISignalsHandler : T
{
public:
	using signal_reader_type = U;
public:

	virtual bool readSignals(const std::string& path) = 0;
	virtual bool readSignals(const std::vector<std::string>& paths) = 0;

	virtual void clearData() = 0;

	virtual ~ISignalsHandler() = default;
};

