#pragma once

#include <list>
#include <map>
#include <string>

#include <boost/optional.hpp>

#include "Error.h"

class ErrorsMapping
{
public:
    ErrorsMapping();
    ~ErrorsMapping();

    unsigned getHttpStatusCode(const std::string &action, const std::string &method, const ErrorCode &error) const;

private:
    typedef boost::optional<std::string> OptCategory;
    typedef boost::optional<int> OptValue;
    typedef std::pair<OptCategory, OptValue> ErrorID;
    typedef boost::optional<ErrorID> OptErrorID;

    class ActionMapping
    {
    public:
        unsigned getHttpStatusCode(const std::string &method, const ErrorCode &error) const;
        void addMapping(const std::list<std::string> &methods, const OptErrorID &error, unsigned status);
        void addMapping(const std::string &method, const OptErrorID &error, unsigned status);

        void setDisallowedMethods(const std::list<std::string> &methods);


        class CategoryHelper {
        public:
            CategoryHelper(ActionMapping &parent, const std::string &method, const std::string &category) :
                m_parent(parent), m_method(method), m_category(category)
            {
            }

            CategoryHelper &setStatus(const OptValue &errorCode, unsigned status)
            {
                m_parent.addMapping(m_method, ErrorID(m_category, errorCode), status);
                return *this;
            }

        private:
            ActionMapping &m_parent;
            std::string m_method;
            std::string m_category;
        };

        class MethodHelper {
        public:
            explicit MethodHelper(ActionMapping &parent, const std::string &method) :
                m_parent(parent), m_method(method)
            {
            }

            CategoryHelper category(const std::string &category)
            {
                return CategoryHelper(m_parent, m_method, category);
            }

            void all(unsigned status)
            {
                m_parent.addMapping(m_method, ErrorID(), status);
            }

        private:
            ActionMapping &m_parent;
            std::string m_method;
        };

        MethodHelper method(const std::string &method);

    private:
        typedef std::pair<std::string, OptErrorID> MethodErrorID;

        typedef std::map<MethodErrorID, unsigned> Mapping;
        Mapping m_mapping;
    };
    
    typedef std::map<std::string, ActionMapping> ActionMap;
    ActionMap m_actionMap;
};

