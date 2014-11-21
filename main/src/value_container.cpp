#include <value_container.h>
#include <QStringList>
#include <QDebug>

#include <pugixml.hpp>
#include <jsoncpp.h>
#include <yaml.h>

using namespace hfsmexec;

template<typename T>
struct ArbitraryValueTypeContainer;

template<>
struct ArbitraryValueTypeContainer<Undefined>
{
    static const ArbitraryValueType type = TYPE_UNDEFINED;
};

template<>
struct ArbitraryValueTypeContainer<Null>
{
    static const ArbitraryValueType type = TYPE_NULL;
};

template<>
struct ArbitraryValueTypeContainer<Boolean>
{
    static const ArbitraryValueType type = TYPE_BOOLEAN;
};

template<>
struct ArbitraryValueTypeContainer<Integer>
{
    static const ArbitraryValueType type = TYPE_INTEGER;
};

template<>
struct ArbitraryValueTypeContainer<Float>
{
    static const ArbitraryValueType type = TYPE_FLOAT;
};

template<>
struct ArbitraryValueTypeContainer<String>
{
    static const ArbitraryValueType type = TYPE_STRING;
};

template<>
struct ArbitraryValueTypeContainer<Array>
{
    static const ArbitraryValueType type = TYPE_ARRAY;
};

template<>
struct ArbitraryValueTypeContainer<Object>
{
    static const ArbitraryValueType type = TYPE_OBJECT;
};

/*
 * ArbitraryValueException
 */
ArbitraryValueException::ArbitraryValueException() :
    message("ArbitraryValueException")
{

}

ArbitraryValueException::ArbitraryValueException(const QString& message):
    message(message)
{

}

ArbitraryValueException::~ArbitraryValueException() throw()
{

}

const char* ArbitraryValueException::what() const throw()
{
    return message.toStdString().c_str();
}

/*
 * ArbitraryValue
 */
ArbitraryValue::ArbitraryValue()
{
    create(TYPE_UNDEFINED);
}


ArbitraryValue::ArbitraryValue(ArbitraryValue const &other)
{
    create(other.type, other.data);
}

template<typename T>
ArbitraryValue::ArbitraryValue(T const &v)
{
    create<T>(v);
}

ArbitraryValue::~ArbitraryValue()
{
    destroy();
}

const ArbitraryValueType& ArbitraryValue::getType() const
{
    return type;
}

void* ArbitraryValue::ptr()
{
    return static_cast<void*>(&data);
}

void const* ArbitraryValue::ptr() const
{
    return static_cast<void const*>(&data);
}

template<typename T>
T& ArbitraryValue::get()
{
    ArbitraryValueType expected = ArbitraryValueTypeContainer<T>::type;
    if(expected != type)
    {
        throw ArbitraryValueException("invalid type");
    }

    switch(type)
    {
        case TYPE_UNDEFINED:
        case TYPE_NULL:
            throw ArbitraryValueException("non-fetchable type");
        default:
            return *static_cast<T*>(ptr());
    }
}

template<typename T>
T const& ArbitraryValue::get() const
{
    ArbitraryValueType expected = ArbitraryValueTypeContainer<T>::type;
    if(expected != type)
    {
        throw ArbitraryValueException("invalid type");
    }

    switch(type)
    {
        case TYPE_UNDEFINED:
        case TYPE_NULL:
            throw ArbitraryValueException("non-fetchable type");
        default:
            return *static_cast<T const*>(ptr());
    }
}

template<typename T>
void ArbitraryValue::set(T const& other)
{
    destroy();
    create<T>(other);
}

void ArbitraryValue::set(ArbitraryValue const& other)
{
    if(this != &other)
    {
        destroy();
        create(other.type, other.data);
    }
}

bool ArbitraryValue::operator==(ArbitraryValue const& other) const
{
    if(type != other.type)
    {
        return false;
    }

    switch(type)
    {
        case TYPE_BOOLEAN:
            return get<Boolean>() == other.get<Boolean>();
        case TYPE_INTEGER:
            return get<Integer>() == other.get<Integer>();
        case TYPE_FLOAT:
            return get<Float>() == other.get<Float>();
        case TYPE_STRING:
            return get<String>() == other.get<String>();
        case TYPE_OBJECT:
            return get<Object>() == other.get<Object>();
        case TYPE_ARRAY:
            return get<Array>() == other.get<Array>();
        default:
            return true;
    }
}

template<typename T>
void ArbitraryValue::create(T const &v)
{
    void* p = ptr();
    type = ArbitraryValueTypeContainer<T>::type;
    switch(type)
    {
        case TYPE_UNDEFINED:
        case TYPE_NULL:
        case TYPE_BOOLEAN:
        case TYPE_INTEGER:
        case TYPE_FLOAT:
            memcpy(&data, &v, sizeof(T));
            break;
        case TYPE_STRING:
            new(p) String(reinterpret_cast<String const&>(v));
            break;
        case TYPE_OBJECT:
            new(p) Object(reinterpret_cast<Object const&>(v));
            break;
        case TYPE_ARRAY:
            new(p) Array(reinterpret_cast<Array const&>(v));
            break;
    }
}

void ArbitraryValue::create(ArbitraryValueType t)
{
    type = t;
    memset(ptr(), 0, sizeof(data));
    switch(type)
    {
        case TYPE_UNDEFINED:
        case TYPE_NULL:
        case TYPE_BOOLEAN:
        case TYPE_INTEGER:
        case TYPE_FLOAT:
            break;
        case TYPE_STRING:
            new(ptr()) String();
            break;
        case TYPE_OBJECT:
            new(ptr()) Object();
            break;
        case TYPE_ARRAY:
            new(ptr()) Array();
            break;
    }
}

void ArbitraryValue::create(ArbitraryValueType t, DataUnion const& other)
{
    void* p = &data;
    type = t;
    switch(t)
    {
        case TYPE_UNDEFINED:
        case TYPE_NULL:
        case TYPE_BOOLEAN:
        case TYPE_INTEGER:
        case TYPE_FLOAT:
            memcpy(&data, &other, sizeof(data));
            break;
        case TYPE_STRING:
            new(p) String(reinterpret_cast<String const&>(other));
            break;
        case TYPE_OBJECT:
            new(p) Object(reinterpret_cast<Object const&>(other));
            break;
        case TYPE_ARRAY:
            new(p) Array(reinterpret_cast<Array const&>(other));
            break;
    }
}

void ArbitraryValue::destroy()
{
    switch(type)
    {
        case TYPE_UNDEFINED:
        case TYPE_NULL:
        case TYPE_BOOLEAN:
        case TYPE_INTEGER:
        case TYPE_FLOAT:
            break;
        case TYPE_STRING:
            static_cast<String*>(ptr())->~String();
            break;
        case TYPE_OBJECT:
            static_cast<Object*>(ptr())->~Object();
            break;
        case TYPE_ARRAY:
            static_cast<Array*>(ptr())->~Array();
            break;
    }

    memset(&data, 0, sizeof(data));
}

/*
 * ValueContainer
 */
ValueContainer::ValueContainer()
{

}

ValueContainer::ValueContainer(const ValueContainer& value) :
    value(value.value)
{

}

template<typename T>
ValueContainer::ValueContainer(const T& value)
{
    setValue<T>(value);
}

ValueContainer::~ValueContainer()
{

}

const ArbitraryValueType& ValueContainer::getType() const
{
    return value.getType();
}

void ValueContainer::undefined()
{
    value.set<Undefined>(Undefined());
}

void ValueContainer::null()
{
    value.set<Null>(Null());
}

bool ValueContainer::get(Boolean& value, Boolean defaultValue) const
{
    return getValue<Boolean>(value, defaultValue);
}

bool ValueContainer::get(Integer& value, Integer defaultValue) const
{
    return getValue<Integer>(value, defaultValue);
}

bool ValueContainer::get(Float& value, Float defaultValue) const
{
    return getValue<Float>(value, defaultValue);
}

bool ValueContainer::get(String& value, String defaultValue) const
{
    return getValue<String>(value, defaultValue);
}

bool ValueContainer::get(Array& value, Array defaultValue) const
{
    return getValue<Array>(value, defaultValue);
}

bool ValueContainer::get(Object& value, Object defaultValue) const
{
    return getValue<Object>(value, defaultValue);
}

bool ValueContainer::get(const QString& path, Boolean& value, Boolean defaultValue) const
{
    return getValue<Boolean>(path, value, defaultValue);
}

bool ValueContainer::get(const QString& path, Integer& value, Integer defaultValue) const
{
    return getValue<Integer>(path, value, defaultValue);
}

bool ValueContainer::get(const QString& path, Float& value, Float defaultValue) const
{
    return getValue<Float>(path, value, defaultValue);
}

bool ValueContainer::get(const QString& path, String& value, String defaultValue) const
{
    return getValue<String>(path, value, defaultValue);
}

bool ValueContainer::get(const QString& path, Array& value, Array defaultValue) const
{
    return getValue<Array>(path, value, defaultValue);
}

bool ValueContainer::get(const QString& path, Object& value, Object defaultValue) const
{
    return getValue<Object>(path, value, defaultValue);
}

bool ValueContainer::get(const QString& path, ValueContainer& value, ValueContainer defaultValue) const
{
    const ValueContainer* v = this;
    if (findValue(path, v))
    {
        value = *v;

        return true;
    }

    value = defaultValue;

    return false;
}

void ValueContainer::set(const Boolean& value)
{
    setValue<Boolean>(value);
}

void ValueContainer::set(const Integer& value)
{
    setValue<Integer>(value);
}

void ValueContainer::set(const Float& value)
{
    setValue<Float>(value);
}

void ValueContainer::set(const char* value)
{
    set(String(value));
}

void ValueContainer::set(const String& value)
{
    setValue<String>(value);
}

void ValueContainer::set(const Array& value)
{
    setValue<Array>(value);
}

void ValueContainer::set(const Object& value)
{
    setValue<Object>(value);
}

void ValueContainer::set(const ValueContainer& value)
{
    *this = value;
}

void ValueContainer::set(const QString& path, const Boolean& value)
{
    setValue<Boolean>(path, value);
}

void ValueContainer::set(const QString& path, const Integer& value)
{
    setValue<Integer>(path, value);
}

void ValueContainer::set(const QString& path, const Float& value)
{
    setValue<Float>(path, value);
}

void ValueContainer::set(const QString& path, const char* value)
{
    set(path, String(value));
}

void ValueContainer::set(const QString& path, const String& value)
{
    setValue<String>(path, value);
}

void ValueContainer::set(const QString& path, const Array& value)
{
    setValue<Array>(path, value);
}

void ValueContainer::set(const QString& path, const Object& value)
{
    setValue<Object>(path, value);
}

void ValueContainer::set(const QString& path, const ValueContainer& value)
{
    ValueContainer* v = this;
    if (findValue(path, v))
    {
        *v = value;
    }
}

void ValueContainer::remove(const QString& path)
{
    //TODO
}

bool ValueContainer::isUndefined() const
{
    return getType() == TYPE_UNDEFINED;
}

bool ValueContainer::isNull() const
{
    return getType() == TYPE_NULL;
}

bool ValueContainer::isBoolean() const
{
    return getType() == TYPE_BOOLEAN;
}

bool ValueContainer::isInteger() const
{
    return getType() == TYPE_INTEGER;
}

bool ValueContainer::isFloat() const
{
    return getType() == TYPE_FLOAT;
}

bool ValueContainer::isString() const
{
    return getType() == TYPE_STRING;
}

bool ValueContainer::isArray() const
{
    return getType() == TYPE_ARRAY;
}

bool ValueContainer::isObject() const
{
    return getType() == TYPE_OBJECT;
}

bool ValueContainer::toXml(QString& xml) const
{
    pugi::xml_document doc;
    pugi::xml_node root = doc.root();
    if (!buildToXml(this, &root))
    {
        qWarning() <<"couldn't build xml from value container";

        return false;
    }

    std::stringstream stream;
    pugi::xml_writer_stream writer(stream);
    doc.save(writer);

    xml = stream.str().c_str();

    return true;
}

bool ValueContainer::toJson(QString& json) const
{
    Json::Value root;
    if (!buildToJson(this, &root))
    {
        qWarning() <<"couldn't build json from value container";

        return false;
    }

    Json::StyledWriter writer;
    json = writer.write(root).c_str();

    return true;
}

bool ValueContainer::toYaml(QString& yaml) const
{
    YAML::Node root;
    if (!buildToYaml(this, &root))
    {
        qWarning() <<"couldn't build yaml from value container";

        return false;
    }

    YAML::Emitter writer;
    writer <<root;
    yaml = writer.c_str();

    return true;
}

bool ValueContainer::fromXml(const QString& xml)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(xml.toStdString().c_str(), xml.size());
    if (result.status != pugi::status_ok)
    {
        qWarning() <<"couldn't set value container from xml";

        return false;
    }

    pugi::xml_node root = doc.root();
    if (!buildFromXml(this, &root))
    {
        qWarning() <<"couldn't set value container from xml";

        return false;
    }

    return true;
}

bool ValueContainer::fromJson(const QString& json)
{
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json.toStdString(), root))
    {
        qWarning() <<"couldn't set value container from json:" <<reader.getFormatedErrorMessages().c_str();

        return false;
    }

    if (!buildFromJson(this, &root))
    {
        qWarning() <<"couldn't set value container from json";

        return false;
    }

    return true;
}

bool ValueContainer::fromYaml(const QString& yaml)
{
    try
    {
        YAML::Node root = YAML::Load(yaml.toStdString());

        if (!buildFromYaml(this, &root))
        {
            qWarning() <<"couldn't set value container from yaml";

            return false;
        }

        return true;
    }
    catch (YAML::Exception e)
    {
        qWarning() <<"couldn't set value container from yaml:" <<e.msg.c_str();
    }

    return false;
}

const ValueContainer& ValueContainer::operator=(const ValueContainer& other)
{
    this->value.set(other.value);

    return *this;
}

bool ValueContainer::operator==(const ValueContainer& other) const
{
    return value == other.value;
}

ValueContainer& ValueContainer::operator[](const QString& name)
{
    if (getType() != TYPE_OBJECT)
    {
        set(Object());
    }

    Object& object = value.get<Object>();
    Object::iterator i = object.find(name.toStdString().c_str());
    if (i == object.end())
    {
        return object.insert(name.toStdString().c_str(), ValueContainer()).value();
    }

    return i.value();
}

const ValueContainer& ValueContainer::operator[](const QString& name) const
{
    if (getType() != TYPE_OBJECT)
    {
        throw ArbitraryValueException("value ist not of type object");
    }

    const Object& object = value.get<Object>();
    Object::const_iterator i = object.find(name);
    if (i == object.end())
    {
        throw ArbitraryValueException("member " + name + " not found");
    }

    return i.value();
}

ValueContainer& ValueContainer::operator[](int i)
{
    if (getType() != TYPE_ARRAY)
    {
        set(Array());
    }

    Array& array = value.get<Array>();
    for (int j = array.size() - i - 1; j < 0; j++)
    {
        array.append(Null());
    }

    return array[i];
}

const ValueContainer& ValueContainer::operator[](int i) const
{
    if (getType() != TYPE_ARRAY)
    {
        throw ArbitraryValueException("value ist not of type array");
    }

    const Array& array = value.get<Array>();
    if (i >= array.size())
    {
        throw ArbitraryValueException("index out of bound");
    }

    return array[i];
}

bool ValueContainer::findValue(const QString& path, const ValueContainer*& value) const
{
    QString replacePath = path.trimmed().replace("[", "/[");
    QStringList splitPath = replacePath.split("/", QString::SkipEmptyParts);

    try
    {
        for (int i = 0; i < splitPath.size(); i++)
        {
            //access array value
            if (splitPath[i].at(0) == '[')
            {
                int index = splitPath[i].mid(1, 1).toInt();
                value = &(*value)[index];
            }
            //access other value
            else
            {
                QString name = splitPath[i];
                value = &(*value)[name];
            }
        }

        return true;
    }
    catch (ArbitraryValueException e)
    {
    }

    return false;
}

bool ValueContainer::findValue(const QString& path, ValueContainer*& value)
{
    QString replacePath = path.trimmed().replace("[", "/[");
    QStringList splitPath = replacePath.split("/", QString::SkipEmptyParts);

    for (int i = 0; i < splitPath.size(); i++)
    {
        //access array value
        if (splitPath[i].at(0) == '[')
        {
            int index = splitPath[i].mid(1, 1).toInt();
            value = &(*value)[index];
        }
        //access other value
        else
        {
            QString name = splitPath[i];
            value = &(*value)[name];
        }
    }

    return true;
}

template<typename T>
bool ValueContainer::getValue(T& value, T defaultValue) const
{
    try
    {
        value = this->value.get<T>();

        return true;
    }
    catch (ArbitraryValueException e)
    {
    }

    value = defaultValue;

    return false;
}

template<typename T>
bool ValueContainer::getValue(const QString& path, T& value, T defaultValue) const
{
    const ValueContainer* v = this;
    if (findValue(path, v))
    {
        return v->get(value, defaultValue);
    }

    return false;
}

template<typename T>
void ValueContainer::setValue(const T& value)
{
    this->value.set<T>(value);
}

template<typename T>
void ValueContainer::setValue(const QString& path, const T& value)
{
    ValueContainer* v = this;
    if (findValue(path, v))
    {
        v->set(value);
    }
}

bool ValueContainer::buildToXml(const ValueContainer* value, void* data) const
{
    pugi::xml_node* xmlValue = static_cast<pugi::xml_node*>(data);

    if (value->isUndefined())
    {
        return false;
    }
    else if (value->isNull())
    {

    }
    else if (value->isBoolean())
    {
        Boolean v;
        value->get(v);
        xmlValue->text().set(v);
    }
    else if (value->isInteger())
    {
        Integer v;
        value->get(v);
        xmlValue->text().set(QString::number(v).toStdString().c_str());
    }
    else if (value->isFloat())
    {
        Float v;
        value->get(v);
        xmlValue->text().set(QString::number(v).toStdString().c_str());
    }
    else if (value->isString())
    {
        String v;
        value->get(v);
        xmlValue->text().set(v.toStdString().c_str());
    }
    else if (value->isArray())
    {
        Array v;
        value->get(v);
        for (int i = 0; i < v.size(); i++)
        {
            pugi::xml_node dataChild = xmlValue->append_child("item");
            if (!buildToXml(&v[i], &dataChild))
            {
                return false;
            }
        }
    }
    else if (value->isObject())
    {
        Object v;
        value->get(v);
        for (Object::const_iterator it = v.begin(); it != v.end(); it++)
        {
            pugi::xml_node dataChild = xmlValue->append_child(it.key().toStdString().c_str());
            if (!buildToXml(&it.value(), &dataChild))
            {
                return false;
            }
        }
    }

    return true;
}

bool ValueContainer::buildToJson(const ValueContainer* value, void* data) const
{
    Json::Value* jsonValue = static_cast<Json::Value*>(data);

    if (value->isUndefined())
    {
        return false;
    }
    else if (value->isNull())
    {

    }
    else if (value->isBoolean())
    {
        Boolean v;
        value->get(v);
        *jsonValue = v;
    }
    else if (value->isInteger())
    {
        Integer v;
        value->get(v);
        *jsonValue = v;
    }
    else if (value->isFloat())
    {
        Float v;
        value->get(v);
        *jsonValue = v;
    }
    else if (value->isString())
    {
        String v;
        value->get(v);
        *jsonValue = v.toStdString();
    }
    else if (value->isArray())
    {
        Array v;
        value->get(v);
        *jsonValue = Json::Value(Json::arrayValue);
        for (int i = 0; i < v.size(); i++)
        {
            Json::Value dataChild;
            if (!buildToJson(&v[i], &dataChild))
            {
                return false;
            }
            (*jsonValue)[i] = dataChild;
        }
    }
    else if (value->isObject())
    {
        Object v;
        value->get(v);
        *jsonValue = Json::Value(Json::objectValue);
        for (Object::const_iterator it = v.begin(); it != v.end(); it++)
        {
            Json::Value dataChild;
            if (!buildToJson(&it.value(), &dataChild))
            {
                return false;
            }
            (*jsonValue)[it.key().toStdString()] = dataChild;
        }
    }

    return true;
}

bool ValueContainer::buildToYaml(const ValueContainer* value, void* data) const
{
    YAML::Node* yamlValue = static_cast<YAML::Node*>(data);

    if (value->isUndefined())
    {
        return false;
    }
    else if (value->isNull())
    {

    }
    else if (value->isBoolean())
    {
        Boolean v;
        value->get(v);
        *yamlValue = v;
    }
    else if (value->isInteger())
    {
        Integer v;
        value->get(v);
        *yamlValue = v;
    }
    else if (value->isFloat())
    {
        Float v;
        value->get(v);
        *yamlValue = v;
    }
    else if (value->isString())
    {
        String v;
        value->get(v);
        *yamlValue = v.toStdString();
    }
    else if (value->isArray())
    {
        Array v;
        value->get(v);
        for (int i = 0; i < v.size(); i++)
        {
            YAML::Node dataChild;
            if (!buildToYaml(&v[i], &dataChild))
            {
                return false;
            }
            yamlValue->push_back(dataChild);
        }
    }
    else if (value->isObject())
    {
        Object v;
        value->get(v);
        for (Object::const_iterator it = v.begin(); it != v.end(); it++)
        {
            YAML::Node dataChild;
            if (!buildToYaml(&it.value(), &dataChild))
            {
                return false;
            }
            (*yamlValue)[it.key().toStdString()] = dataChild;
        }
    }

    return true;
}

bool ValueContainer::buildFromXml(ValueContainer* value, void* data)
{
    pugi::xml_node* xmlValue = static_cast<pugi::xml_node*>(data);
    pugi::xml_text textContent = xmlValue->text();
    if (textContent)
    {
        std::string text = textContent.get();

        if (text.empty())
        {
            value->null();
        }
        else if (text == "true" || text == "false")
        {
            value->set(textContent.as_bool());
        }
        else if (strtod(text.c_str(), NULL) != 0.0 || text == "0" || text == "0.0")
        {
            if (text.find_first_of(".") == std::string::npos)
            {
                value->set(textContent.as_int());
            }
            else
            {
                value->set(textContent.as_double());
            }
        }
        else
        {
            value->set(text.c_str());
        }
    }
    else if (xmlValue->child("item"))   //TODO object or array?
    {
        Array array;
        for (pugi::xml_node_iterator i = xmlValue->begin(); i != xmlValue->end(); i++)
        {
            ValueContainer v;
            if (!buildFromXml(&v, &i))
            {
                return false;
            }
            array.append(v);
        }
        value->set(array);
    }
    else
    {
        Object object;
        for (pugi::xml_node_iterator i = xmlValue->begin(); i != xmlValue->end(); i++)
        {
            ValueContainer v;
            if (!buildFromXml(&v, &i))
            {
                return false;
            }

            object.insert(i->name(), v);
        }
        value->set(object);
    }

    return true;
}

bool ValueContainer::buildFromJson(ValueContainer* value, void* data)
{
    Json::Value* jsonValue = static_cast<Json::Value*>(data);

    if (jsonValue->isNull())
    {
        value->null();
    }
    else if (jsonValue->isBool())
    {
        value->set(jsonValue->asBool());
    }
    else if (jsonValue->isInt() || jsonValue->isUInt())
    {
        value->set(jsonValue->asInt());
    }
    else if (jsonValue->isDouble())
    {
        value->set(jsonValue->asDouble());
    }
    else if (jsonValue->isString())
    {
        value->set(jsonValue->asString().c_str());
    }
    else if (jsonValue->isArray())
    {
        Array array;
        for (unsigned int i = 0; i < jsonValue->size(); i++)
        {
            ValueContainer v;
            if (!buildFromJson(&v, &jsonValue[i]))
            {
                return false;
            }
            array.append(v);
        }
        value->set(array);
    }
    else if (jsonValue->isObject())
    {
        Object object;
        for (Json::ValueIterator i = jsonValue->begin(); i != jsonValue->end(); i++)
        {
            ValueContainer v;
            if (!buildFromJson(&v, &*i))
            {
                return false;
            }
            object.insert(i.memberName(), v);
        }
        value->set(object);
    }

    return true;
}

bool ValueContainer::buildFromYaml(ValueContainer* value, void* data)
{
    YAML::Node* yamlValue = static_cast<YAML::Node*>(data);

    if (yamlValue->IsNull())
    {
        value->null();
    }
    else if (yamlValue->IsScalar())
    {
        try
        {
            value->set(yamlValue->as<Boolean>());
        }
        catch (const YAML::BadConversion& e)
        {
            try
            {
                value->set(yamlValue->as<Integer>());
            }
            catch (const YAML::BadConversion& e)
            {
                try
                {
                    value->set(yamlValue->as<Float>());
                }
                catch (const YAML::BadConversion& e)
                {
                    std::string s = yamlValue->as<std::string>();
                    value->set(s.c_str());
                }
            }
        }
    }
    else if (yamlValue->IsSequence())
    {
        Array array;
        for (unsigned int i = 0; i < yamlValue->size(); i++)
        {
            ValueContainer v;
            YAML::Node n = (*yamlValue)[i];
            if (!buildFromYaml(&v, &n))
            {
                return false;
            }
            array.append(v);
        }
        value->set(array);
    }
    else if (yamlValue->IsMap())
    {
        Object object;
        for (YAML::iterator i = yamlValue->begin(); i != yamlValue->end(); i++)
        {
            ValueContainer v;
            if (!buildFromYaml(&v, &i->second))
            {
                return false;
            }
            object.insert(i->first.as<std::string>().c_str(), v);
        }
        value->set(object);
    }

    return true;
}

/*
 * ValueContainerTest
 */
#include <QFile>
ValueContainerTest::ValueContainerTest()
{
    QString out;

    ValueContainer c;

    qDebug() <<"===================================";
    c.fromJson("{ \"some\" : { \"test\" : { \"x\" : 5 } }, \"test\" : 1234 }");
    c.set("/other/test/x", -50);
    c.set("/array_test[5]/obj[2]", "foo");

    ValueContainer sub;
    c.get("/other/test/x", sub);
    sub.null();

    out = "";
    c.toXml(out);
    qDebug() <<out;
    c.toJson(out);
    qDebug() <<out;
    c.toYaml(out);
    qDebug() <<out;

    qDebug() <<"===================================";
    c.fromXml("<test><from_xml><x>1</x><y>2</y><z>3</z></from_xml><arr><item>5</item><item>6</item><item>7</item></arr></test>");

    out = "";
    c.toXml(out);
    qDebug() <<out;
    c.toJson(out);
    qDebug() <<out;
    c.toYaml(out);
    qDebug() <<out;

    qDebug() <<"===================================";
    QFile file("/home/marcel/Programming/hfsm-exec/test.yaml");
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qWarning() <<"couldn't open file";

        return;
    }
    QTextStream stream(&file);
    QString data = stream.readAll();

    c.fromYaml(data.toStdString().c_str());

    out = "";
    c.toXml(out);
    qDebug() <<out;
    c.toJson(out);
    qDebug() <<out;
    c.toYaml(out);
    qDebug() <<out;
}
