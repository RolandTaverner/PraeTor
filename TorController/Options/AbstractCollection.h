#pragma once

#include <boost/assert.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>

template<typename ValueType>
class AbstractCollectionConstIterator;

template<typename ValueType>
class AbstractCollection
{
public:
    typedef ValueType CollectionValueType;
    typedef typename AbstractCollection<ValueType> CollectionType;
    typedef typename AbstractCollectionConstIterator<ValueType> IteratorType;
    typedef typename boost::iterator_range<IteratorType> RangeType;

    virtual ~AbstractCollection() {}

    RangeType getRange() const
    {
        return RangeType(AbstractCollectionConstIterator(begin()), AbstractCollectionConstIterator(end()));
    }

protected:
    class Element
    {
    public:

        explicit Element(const AbstractCollection<ValueType> *collection) :
            m_collection(collection)
        {
        }
        
        virtual ~Element() {}

        Element *next() const
        {
            BOOST_ASSERT(m_collection != NULL);
            return getCollection()->next(this);
        }

        const ValueType &dereference() const
        {
            BOOST_ASSERT(m_collection != NULL);
            return getCollection()->dereference(this);
        }
    
        const AbstractCollection<ValueType> *getCollection() const
        {
            return m_collection;
        }

        virtual Element *clone() const = 0;
        
        virtual bool equals(const Element *rhs) const = 0;

    private:
        const AbstractCollection<ValueType> *m_collection;
    };

private:
    virtual Element *begin() const = 0;
    virtual Element *end() const = 0;
    virtual Element *next(const Element *current) const = 0;
    virtual const ValueType &dereference(const Element *current) const = 0;

    friend class Element;
};

template<typename ValueType>
class AbstractCollectionConstIterator
    : public boost::iterator_facade<
                typename AbstractCollectionConstIterator<ValueType>, 
                const ValueType,
                boost::forward_traversal_tag>
{
public:
    typedef AbstractCollection<ValueType> CollectionType;
    typedef typename CollectionType::Element ElementType;

    AbstractCollectionConstIterator()
        : m_element(NULL)
    {
    }

    explicit AbstractCollectionConstIterator(ElementType *p)
        : m_element(p)
    {
        BOOST_ASSERT(m_element != NULL);
    }

    AbstractCollectionConstIterator(const AbstractCollectionConstIterator &rhs)
        : iterator_facade_(rhs), m_element(rhs.m_element->clone())
    {
    }

    AbstractCollectionConstIterator &operator=(const AbstractCollectionConstIterator &rhs)
    {
        if (this != &rhs)
        {
            if (m_element)
            {
                delete m_element;
            }
            iterator_facade_::operator=(rhs);
            m_element = rhs.m_element ? rhs.m_element->clone() : NULL;
        }

        return *this;
    }

    ~AbstractCollectionConstIterator()
    {
        if (m_element)
        {
            delete m_element;
        }
    }

private:
    friend class boost::iterator_core_access;

    void increment() 
    { 
        BOOST_ASSERT(m_element != NULL);

        const ElementType *nextElement = m_element->next();
        if (m_element)
        {
            delete m_element;
        }
        m_element = nextElement;
    }

    bool equal(const AbstractCollectionConstIterator &other) const
    {
        if (this->m_element == NULL && other.m_element == NULL)
        {
            return true;
        }
        else if ((this->m_element == NULL) != (other.m_element == NULL))
        {
            return false;
        }
        return this->m_element->equals(other.m_element);
    }

    const ValueType &dereference() const 
    { 
        BOOST_ASSERT(m_element != NULL);

        return m_element->dereference(); 
    }

    const ElementType *m_element;
};
