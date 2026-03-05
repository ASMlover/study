#include "ms/object.hh"
#include "ms/value.hh"

#include <cstring>
#include <sstream>
#include <iomanip>
#include <functional>

namespace ms {

size_t ValueHash::operator()(const Value& value) const {
    switch (value.type) {
        case Type::Nil:
            return 0;
        case Type::Bool:
            return std::hash<bool>{}(value.asBool());
        case Type::Number:
            return std::hash<double>{}(value.asNumber());
        case Type::Obj:
            if (value.asObj()->type == ObjType::OBJ_STRING) {
                auto* str = static_cast<ObjString*>(value.asObj());
                return std::hash<std::string>{}(str->str);
            }
            return std::hash<void*>{}(static_cast<void*>(value.asObj()));
    }
    return 0;
}

bool ValueEqual::operator()(const Value& a, const Value& b) const {
    return valuesEqual(a, b);
}

static ObjString* stringList = nullptr;
static ObjFunction* functionList = nullptr;
static ObjNative* nativeList = nullptr;
static ObjClosure* closureList = nullptr;
static ObjUpvalue* upvalueList = nullptr;
static ObjClass* classList = nullptr;
static ObjInstance* instanceList = nullptr;
static ObjMethod* methodList = nullptr;
static ObjBoundMethod* boundMethodList = nullptr;
static ObjList* listList = nullptr;
static ObjMap* mapList = nullptr;

std::string Obj::toString() const {
    switch (type) {
        case ObjType::OBJ_STRING:
            return static_cast<const ObjString*>(this)->toString();
        case ObjType::OBJ_FUNCTION:
            return static_cast<const ObjFunction*>(this)->toString();
        case ObjType::OBJ_NATIVE:
            return static_cast<const ObjNative*>(this)->toString();
        case ObjType::OBJ_CLOSURE:
            return static_cast<const ObjClosure*>(this)->toString();
        case ObjType::OBJ_UPVALUE:
            return static_cast<const ObjUpvalue*>(this)->toString();
        case ObjType::OBJ_CLASS:
            return static_cast<const ObjClass*>(this)->toString();
        case ObjType::OBJ_INSTANCE:
            return static_cast<const ObjInstance*>(this)->toString();
        case ObjType::OBJ_METHOD:
            return static_cast<const ObjMethod*>(this)->toString();
        case ObjType::OBJ_BOUND_METHOD:
            return static_cast<const ObjBoundMethod*>(this)->toString();
        case ObjType::OBJ_LIST:
            return static_cast<const ObjList*>(this)->toString();
        case ObjType::OBJ_MAP:
            return static_cast<const ObjMap*>(this)->toString();
    }
    return "<unknown>";
}

ObjString* takeString(std::string* s) {
    auto* obj = new ObjString(std::move(*s));
    obj->next = stringList;
    stringList = obj;
    delete s;
    return obj;
}

ObjString* copyString(const char* s, size_t length) {
    char* heapChars = new char[length + 1];
    memcpy(heapChars, s, length);
    heapChars[length] = '\0';
    auto* obj = new ObjString(heapChars);
    obj->next = stringList;
    stringList = obj;
    return obj;
}

ObjFunction* newFunction() {
    auto* obj = new ObjFunction();
    obj->next = functionList;
    functionList = obj;
    return obj;
}

ObjNative* newNative(NativeFn function) {
    auto* obj = new ObjNative(function);
    obj->next = nativeList;
    nativeList = obj;
    return obj;
}

ObjClosure* newClosure(ObjFunction* function) {
    auto* obj = new ObjClosure(function);
    obj->next = closureList;
    closureList = obj;
    return obj;
}

ObjUpvalue* newUpvalue(Value* location) {
    auto* obj = new ObjUpvalue(location);
    obj->next = upvalueList;
    upvalueList = obj;
    return obj;
}

ObjClass* newClass(ObjString* name) {
    auto* obj = new ObjClass(name);
    obj->next = classList;
    classList = obj;
    return obj;
}

ObjInstance* newInstance(ObjClass* klass) {
    auto* obj = new ObjInstance(klass);
    obj->next = instanceList;
    instanceList = obj;
    return obj;
}

ObjMethod* newMethod(ObjClosure* closure) {
    auto* obj = new ObjMethod(closure);
    obj->next = methodList;
    methodList = obj;
    return obj;
}

ObjBoundMethod* newBoundMethod(Value* receiver, ObjClosure* method) {
    auto* obj = new ObjBoundMethod(receiver, method);
    obj->next = boundMethodList;
    boundMethodList = obj;
    return obj;
}

ObjList* newList() {
    auto* obj = new ObjList();
    obj->next = listList;
    listList = obj;
    return obj;
}

ObjMap* newMap() {
    auto* obj = new ObjMap();
    obj->next = mapList;
    mapList = obj;
    return obj;
}

std::string ObjFunction::toString() const {
    if (name == nullptr) {
        return "<script>";
    }
    std::ostringstream oss;
    oss << "<fn " << name->str << ">";
    return oss.str();
}

std::string ObjClosure::toString() const {
    if (function->name == nullptr) {
        return "<script>";
    }
    std::ostringstream oss;
    oss << "<fn " << function->name->str << ">";
    return oss.str();
}

std::string ObjClass::toString() const {
    if (name == nullptr) {
        return "<class>";
    }
    std::ostringstream oss;
    oss << "<class " << name->str << ">";
    return oss.str();
}

std::string ObjInstance::toString() const {
    if (klass == nullptr || klass->name == nullptr) {
        return "<instance>";
    }
    std::ostringstream oss;
    oss << "<" << klass->name->str << " instance>";
    return oss.str();
}

std::string ObjList::toString() const {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) {
            oss << ", ";
        }
        oss << elements[i].toString();
    }
    oss << "]";
    return oss.str();
}

std::string ObjMap::toString() const {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& [key, value] : map) {
        if (!first) {
            oss << ", ";
        }
        first = false;
        oss << key.toString() << ": " << value.toString();
    }
    oss << "}";
    return oss.str();
}

ObjString::ObjString(std::string&& s) : Obj(ObjType::OBJ_STRING), str(std::move(s)), hash(0) {}

ObjString::ObjString(char* chars) : Obj(ObjType::OBJ_STRING), str(chars), hash(0) {}

ObjFunction::ObjFunction() : Obj(ObjType::OBJ_FUNCTION), arity(0), upvalueCount(0), chunk(nullptr), name(nullptr) {}

ObjClosure::ObjClosure(ObjFunction* fn) : Obj(ObjType::OBJ_CLOSURE), function(fn), upvalues(nullptr), upvalueCount(0) {}

ObjClosure::~ObjClosure() {
    delete[] upvalues;
}

ObjUpvalue::ObjUpvalue(Value* loc) : Obj(ObjType::OBJ_UPVALUE), location(loc), closed(nullptr), next(nullptr) {}

ObjClass::ObjClass(ObjString* n) : Obj(ObjType::OBJ_CLASS), name(n), methods(nullptr) {}

ObjInstance::ObjInstance(ObjClass* k) : Obj(ObjType::OBJ_INSTANCE), klass(k), fields(nullptr) {}

ObjMethod::ObjMethod(ObjClosure* c) : Obj(ObjType::OBJ_METHOD), closure(c) {}

ObjBoundMethod::ObjBoundMethod(Value* rec, ObjClosure* m) : Obj(ObjType::OBJ_BOUND_METHOD), receiver(rec), method(m) {}

ObjList::ObjList() : Obj(ObjType::OBJ_LIST), elements() {}

ObjMap::ObjMap() : Obj(ObjType::OBJ_MAP), map() {}

std::string ObjString::toString() const {
    return str;
}

std::string ObjNative::toString() const {
    return "<native fn>";
}

std::string ObjUpvalue::toString() const {
    return "<upvalue>";
}

std::string ObjMethod::toString() const {
    return "<method>";
}

std::string ObjBoundMethod::toString() const {
    return "<bound method>";
}

}
