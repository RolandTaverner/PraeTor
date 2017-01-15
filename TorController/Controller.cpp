#include "Controller.h"

//-------------------------------------------------------------------------------------------------
Controller::Controller(const Tools::Configuration::ConfigurationView &config) :
    m_config(config)
{
}

//-------------------------------------------------------------------------------------------------
Controller::~Controller()
{
}

//-------------------------------------------------------------------------------------------------
const Tools::Configuration::ConfigurationView &Controller::getConf() const
{
    return m_config;
}