#include "jiminy/core/telemetry/telemetry_sender.h"


namespace jiminy
{
    TelemetrySender::TelemetrySender() :
    objectName_(DEFAULT_TELEMETRY_NAMESPACE),
    telemetryData_(nullptr),
    bufferPosition_()
    {
    }

    void TelemetrySender::configureObject(std::shared_ptr<TelemetryData> telemetryDataInstance,
                                          const std::string & objectName)
    {
        objectName_ = objectName;
        telemetryData_ = telemetryDataInstance;
        bufferPosition_.clear();
    }

    hresult_t TelemetrySender::registerConstant(const std::string & variableName,
                                                const std::string & value)
    {
        const std::string fullFieldName =
            objectName_ + TELEMETRY_FIELDNAME_DELIMITER + variableName;
        return telemetryData_->registerConstant(fullFieldName, value);
    }

    void TelemetrySender::updateValues()
    {
        // Write the value directly in the buffer holder using the pointer stored in the map.
        for (const auto & pair : bufferPosition_)
        {
            std::visit([](auto && arg) { *arg.second = *arg.first; }, pair);
        }
    }

    uint32_t TelemetrySender::getLocalNumEntries() const
    {
        return static_cast<uint32_t>(bufferPosition_.size());
    }

    const std::string & TelemetrySender::getObjectName() const
    {
        return objectName_;
    }
}