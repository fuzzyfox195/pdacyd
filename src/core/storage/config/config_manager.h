#pragma once

#include "system_config.h"

namespace PDACYD {

class ConfigManager {
public:
    bool begin();

    bool loadSystemConfig();
    bool loadUIConfig();

    bool saveSystemConfig();
    bool saveUIConfig();

    const SystemConfig& getSystemConfig() const;
    const UIConfig& getUIConfig() const;

    void setSystemConfig(const SystemConfig& config);
    void setUIConfig(const UIConfig& config);

private:
    SystemConfig system_config_;
    UIConfig ui_config_;

    bool initialized_;
};

}