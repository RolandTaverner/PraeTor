#include <boost/assert.hpp>
#include <pion/http/types.hpp>

#include "Controller/ControllerErrors.h"
#include "Options/OptionErrors.h"
#include "Process/ProcessErrors.h"
#include "WebServices/ErrorsMapping.h"
#include "WebServices/ResourceActions.h"

using namespace pion::http;

//-------------------------------------------------------------------------------------------------
ErrorsMapping::ErrorsMapping()
{
    {
        ActionMapping am;
        am.setDisallowedMethods({ types::REQUEST_METHOD_DELETE, types::REQUEST_METHOD_POST, types::REQUEST_METHOD_PUT });
        
        am.method(types::REQUEST_METHOD_GET).all(types::RESPONSE_CODE_SERVER_ERROR);
        
        m_actionMap[ResourceActions::nodeControllerInfo] = am;
    }

    {
        ActionMapping am;
        am.setDisallowedMethods({ types::REQUEST_METHOD_DELETE, types::REQUEST_METHOD_PUT });
        
        am.method(types::REQUEST_METHOD_GET).all(types::RESPONSE_CODE_SERVER_ERROR);

        am.method(types::REQUEST_METHOD_POST).category(getControllerErrorsCategory().name())
            .setStatus((int)ControllerErrors::processIsRunning, 409)
            .setStatus((int)ControllerErrors::presetsNotFound, types::RESPONSE_CODE_NOT_FOUND);

        am.method(types::REQUEST_METHOD_POST).category(getProcessErrorsCategory().name())
            .setStatus((int)ProcessErrors::alreadyRunning, 409);
        
        m_actionMap[ResourceActions::nodePresets] = am;
    }

    {
        ActionMapping am;
        am.setDisallowedMethods({ types::REQUEST_METHOD_DELETE, types::REQUEST_METHOD_PUT, types::REQUEST_METHOD_POST });

        am.method(types::REQUEST_METHOD_GET).category(getControllerErrorsCategory().name())
            .setStatus((int)ControllerErrors::presetsNotFound, types::RESPONSE_CODE_NOT_FOUND);

        m_actionMap[ResourceActions::nodeGetPresetsGroup] = am;
    }

    {
        ActionMapping am;
        am.setDisallowedMethods({ types::REQUEST_METHOD_DELETE, types::REQUEST_METHOD_PUT, types::REQUEST_METHOD_POST });

        am.method(types::REQUEST_METHOD_GET).all(types::RESPONSE_CODE_SERVER_ERROR);

        m_actionMap[ResourceActions::nodeProcesses] = am;
    }

    {
        ActionMapping am;
        am.setDisallowedMethods({ types::REQUEST_METHOD_DELETE, types::REQUEST_METHOD_PUT, types::REQUEST_METHOD_POST });

        am.method(types::REQUEST_METHOD_GET).category(getControllerErrorsCategory().name())
            .setStatus((int)ControllerErrors::processNotFound, types::RESPONSE_CODE_NOT_FOUND);

        m_actionMap[ResourceActions::nodeProcessInfo] = am;
    }

    {
        ActionMapping am;
        am.setDisallowedMethods({ types::REQUEST_METHOD_DELETE, types::REQUEST_METHOD_PUT, types::REQUEST_METHOD_POST });

        am.method(types::REQUEST_METHOD_GET).category(getControllerErrorsCategory().name())
            .setStatus((int)ControllerErrors::processNotFound, types::RESPONSE_CODE_NOT_FOUND);

        m_actionMap[ResourceActions::nodeProcessConfigs] = am;
    }

    {
        ActionMapping am;
        am.setDisallowedMethods({ types::REQUEST_METHOD_DELETE, types::REQUEST_METHOD_PUT, types::REQUEST_METHOD_POST });

        am.method(types::REQUEST_METHOD_GET).category(getControllerErrorsCategory().name())
            .setStatus((int)ControllerErrors::processNotFound, types::RESPONSE_CODE_NOT_FOUND);

        am.method(types::REQUEST_METHOD_GET).category(getProcessErrorsCategory().name())
            .setStatus((int)ProcessErrors::noSuchStorage, types::RESPONSE_CODE_NOT_FOUND);

        m_actionMap[ResourceActions::nodeProcessConfig] = am;
    }

    {
        ActionMapping am;
        am.setDisallowedMethods({ types::REQUEST_METHOD_DELETE, types::REQUEST_METHOD_PUT, types::REQUEST_METHOD_POST });

        am.method(types::REQUEST_METHOD_GET).category(getControllerErrorsCategory().name())
            .setStatus((int)ControllerErrors::processNotFound, types::RESPONSE_CODE_NOT_FOUND);

        m_actionMap[ResourceActions::nodeProcessLog] = am;
    }

    {
        ActionMapping am;
        am.setDisallowedMethods({ types::REQUEST_METHOD_POST });

        am.method(types::REQUEST_METHOD_GET).category(getControllerErrorsCategory().name())
            .setStatus((int)ControllerErrors::processNotFound, types::RESPONSE_CODE_NOT_FOUND);

        am.method(types::REQUEST_METHOD_GET).category(getProcessErrorsCategory().name())
            .setStatus((int)ProcessErrors::noSuchStorage, types::RESPONSE_CODE_NOT_FOUND)
            .setStatus((int)ProcessErrors::noSuchOption, types::RESPONSE_CODE_NOT_FOUND);

        am.method(types::REQUEST_METHOD_GET).category(getOptionErrorsCategory().name())
            .setStatus((int)OptionErrors::notFoundInStorage, types::RESPONSE_CODE_NOT_FOUND);

        am.method(types::REQUEST_METHOD_PUT).category(getControllerErrorsCategory().name())
            .setStatus((int)ControllerErrors::processNotFound, types::RESPONSE_CODE_NOT_FOUND);

        am.method(types::REQUEST_METHOD_PUT).category(getProcessErrorsCategory().name())
            .setStatus((int)ProcessErrors::noSuchStorage, types::RESPONSE_CODE_NOT_FOUND)
            .setStatus((int)ProcessErrors::noSuchOption, types::RESPONSE_CODE_NOT_FOUND)
            .setStatus((int)ProcessErrors::cantEditConfigOfRunningProcess, 409)
            .setStatus((int)ProcessErrors::systemOptionEditForbidden, types::RESPONSE_CODE_FORBIDDEN);

        am.method(types::REQUEST_METHOD_PUT).category(getOptionErrorsCategory().name())
            .setStatus((int)OptionErrors::notFoundInStorage, types::RESPONSE_CODE_NOT_FOUND)
            .setStatus((int)OptionErrors::missingValue, 409)
            .setStatus((int)OptionErrors::assigningSingleToListValue, 409)
            .setStatus((int)OptionErrors::assigningListToSingleValue, 409)
            .setStatus((int)OptionErrors::typeCheckFailed, 409)
            .setStatus((int)OptionErrors::constraintCheckFailed, 409);

        am.method(types::REQUEST_METHOD_DELETE).category(getControllerErrorsCategory().name())
            .setStatus((int)ControllerErrors::processNotFound, types::RESPONSE_CODE_NOT_FOUND);

        am.method(types::REQUEST_METHOD_DELETE).category(getProcessErrorsCategory().name())
            .setStatus((int)ProcessErrors::noSuchStorage, types::RESPONSE_CODE_NOT_FOUND)
            .setStatus((int)ProcessErrors::noSuchOption, types::RESPONSE_CODE_NOT_FOUND)
            .setStatus((int)ProcessErrors::cantEditConfigOfRunningProcess, 409)
            .setStatus((int)ProcessErrors::systemOptionEditForbidden, types::RESPONSE_CODE_FORBIDDEN);

        am.method(types::REQUEST_METHOD_DELETE).category(getOptionErrorsCategory().name())
            .setStatus((int)OptionErrors::notFoundInStorage, types::RESPONSE_CODE_NOT_FOUND);

        m_actionMap[ResourceActions::nodeProcessOption] = am;
    }

    {
        ActionMapping am;
        am.setDisallowedMethods({ types::REQUEST_METHOD_DELETE, types::REQUEST_METHOD_PUT, types::REQUEST_METHOD_GET });

        am.method(types::REQUEST_METHOD_POST).category(getControllerErrorsCategory().name())
            .setStatus((int)ControllerErrors::processNotFound, types::RESPONSE_CODE_NOT_FOUND);

        am.method(types::REQUEST_METHOD_POST).category(getProcessErrorsCategory().name())
            .setStatus((int)ProcessErrors::alreadyRunning, 409)
            .setStatus((int)ProcessErrors::processNotRunning, 409)
            .setStatus((int)ProcessErrors::missingRequiredOption, 409)
            .setStatus((int)ProcessErrors::configFileWriteError, types::RESPONSE_CODE_SERVER_ERROR);

        m_actionMap[ResourceActions::nodeProcessAction] = am;
    }
}

//-------------------------------------------------------------------------------------------------
ErrorsMapping::~ErrorsMapping()
{
}

//-------------------------------------------------------------------------------------------------
unsigned ErrorsMapping::getHttpStatusCode(const std::string &action, const std::string &method, const ErrorCode &error) const
{
    ActionMap::const_iterator itAction = m_actionMap.find(action);
    BOOST_ASSERT(itAction != m_actionMap.end());
    if (itAction == m_actionMap.end())
    {
        return types::RESPONSE_CODE_SERVER_ERROR;
    }
    const ActionMapping &am = itAction->second;
    return am.getHttpStatusCode(method, error);
}

//-------------------------------------------------------------------------------------------------
unsigned ErrorsMapping::ActionMapping::getHttpStatusCode(const std::string &method, const ErrorCode &error) const
{
    const ErrorID errorID(error.category(), error.value());

    Mapping::const_iterator itExact = m_mapping.find(MethodErrorID(method, errorID));
    if (itExact != m_mapping.end())
    {
        return itExact->second;
    }

    const ErrorID errorIDCategoryOnly(error.category(), OptValue());
    Mapping::const_iterator itCategoryOnly = m_mapping.find(MethodErrorID(method, errorIDCategoryOnly));
    if (itCategoryOnly != m_mapping.end())
    {
        return itCategoryOnly->second;
    }

    Mapping::const_iterator itMethodOnly = m_mapping.find(MethodErrorID(method, OptErrorID()));
    if (itMethodOnly != m_mapping.end())
    {
        return itMethodOnly->second;
    }

    return types::RESPONSE_CODE_SERVER_ERROR;
}

//-------------------------------------------------------------------------------------------------
void ErrorsMapping::ActionMapping::addMapping(const std::list<std::string> &methods, const ErrorsMapping::OptErrorID &errorID, unsigned status)
{
    for (const std::string &method : methods)
    {
        m_mapping[MethodErrorID(method, errorID)] = status;
    }
}

//-------------------------------------------------------------------------------------------------
void ErrorsMapping::ActionMapping::addMapping(const std::string &method, const ErrorsMapping::OptErrorID &errorID, unsigned status)
{
    m_mapping[MethodErrorID(method, errorID)] = status;
}

//-------------------------------------------------------------------------------------------------
void ErrorsMapping::ActionMapping::setDisallowedMethods(const std::list<std::string> &methods)
{
    for (const std::string &method : methods)
    {
        m_mapping[MethodErrorID(method, OptErrorID())] = types::RESPONSE_CODE_METHOD_NOT_ALLOWED;
    }
}

//-------------------------------------------------------------------------------------------------
ErrorsMapping::ActionMapping::MethodHelper ErrorsMapping::ActionMapping::method(const std::string &method)
{
    return MethodHelper(*this, method);
}