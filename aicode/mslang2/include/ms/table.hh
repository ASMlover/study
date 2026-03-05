#pragma once

#include "ms/value.hh"
#include "ms/object.hh"

namespace ms {

class Table {
public:
    Table();
    ~Table();

    void adjustCapacity(int capacity);
    bool set(ObjString* key, Value value);
    bool get(ObjString* key, Value& value);
    bool remove(ObjString* key);
    void addAll(Table* from);
    ObjString* findString(std::string_view chars);
    void removeMarked();
    void mark();
    void clear();

private:
    struct Entry;

    static Entry* findEntry(Entry* entries, int capacity, ObjString* key);

    Entry* entries;
    int count;
    int capacity;
};

struct Table::Entry {
    ObjString* key;
    Value value;
    Entry() : key(nullptr), value(NIL_VAL) {}
};

}
