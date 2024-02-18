#pragma once
#include "Base/ISignalsHandler.hpp"
#include "../../Common/Log/Logable.hpp"
#include "../../Signals/SignalReader.hpp"
#include "SignalTable/SignalTable.hpp"
class SignalsHandler : public ISignalsHandler<Logable, SignalReader>
{
public:
private:
	std::shared_ptr<SignalTable> table;
	SignalReadProperty readProperty;
public:
	SignalsHandler();
	// ”наследовано через ISignalsHandler
	bool readSignals(const std::string& path) override;
	bool readSignals(const std::vector<std::string>& paths) override;
	void clearData() override;
};

