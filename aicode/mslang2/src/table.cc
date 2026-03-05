#include "ms/table.hh"

#include <cstring>

namespace ms {

static uint32_t hashString(const char* key, size_t length) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; i++) {
        hash ^= static_cast<uint32_t>(key[i]);
        hash *= 16777619u;
    }
    return hash;
}

Table::Entry* Table::findEntry(Entry* entries, int capacity, ObjString* key) {
    uint32_t index = key->hash % capacity;
    Entry* tombstone = nullptr;
    while (true) {
        Entry* entry = &entries[index];
        if (entry->key == nullptr) {
            if (entry->value.isNil()) {
                return tombstone != nullptr ? tombstone : entry;
            }
            if (tombstone == nullptr) {
                tombstone = entry;
            }
        } else if (entry->key == key) {
            return entry;
        }
        index = (index + 1) % capacity;
    }
}

Table::Table() : entries(nullptr), count(0), capacity(0) {}

Table::~Table() {
    delete[] entries;
}

void Table::adjustCapacity(int newCapacity) {
    auto* newEntries = new Entry[newCapacity];
    for (int i = 0; i < newCapacity; i++) {
        newEntries[i] = Entry();
    }

    for (int i = 0; i < capacity; i++) {
        Entry* entry = &entries[i];
        if (entry->key == nullptr) {
            continue;
        }
        int index = entry->key->hash % newCapacity;
        while (newEntries[index].key != nullptr) {
            index = (index + 1) % newCapacity;
        }
        newEntries[index] = *entry;
    }

    delete[] entries;
    entries = newEntries;
    capacity = newCapacity;
}

bool Table::set(ObjString* key, Value value) {
    if (count + 1 > capacity * 0.75) {
        int newCapacity = capacity < 8 ? 8 : capacity * 2;
        adjustCapacity(newCapacity);
    }

    Entry* entry = findEntry(entries, capacity, key);
    bool isNewKey = entry->key == nullptr;
    if (isNewKey && entry->value.isNil()) {
        count++;
    }
    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool Table::get(ObjString* key, Value& value) {
    if (count == 0) {
        return false;
    }
    Entry* entry = findEntry(entries, capacity, key);
    if (entry->key == nullptr) {
        return false;
    }
    value = entry->value;
    return true;
}

bool Table::remove(ObjString* key) {
    if (count == 0) {
        return false;
    }
    Entry* entry = findEntry(entries, capacity, key);
    if (entry->key == nullptr) {
        return false;
    }
    entry->key = nullptr;
    entry->value = NIL_VAL;
    count--;
    return true;
}

void Table::addAll(Table* from) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if (entry->key != nullptr) {
            set(entry->key, entry->value);
        }
    }
}

ObjString* Table::findString(std::string_view chars) {
    if (count == 0) {
        return nullptr;
    }
    uint32_t hash = hashString(chars.data(), chars.size());
    int index = hash % capacity;
    while (true) {
        Entry* entry = &entries[index];
        if (entry->key == nullptr) {
            return nullptr;
        }
        if (entry->key->hash == hash && entry->key->str.size() == chars.size() &&
            std::memcmp(entry->key->str.data(), chars.data(), chars.size()) == 0) {
            return entry->key;
        }
        index = (index + 1) % capacity;
    }
}

void Table::removeMarked() {
    if (count == 0) {
        return;
    }
    for (int i = 0; i < capacity; i++) {
        Entry* entry = &entries[i];
        if (entry->key != nullptr && entry->key->isMarked) {
            entry->key = nullptr;
            entry->value = NIL_VAL;
            count--;
        }
    }
}

void Table::mark() {
    for (int i = 0; i < capacity; i++) {
        Entry* entry = &entries[i];
        if (entry->key != nullptr) {
            entry->key->isMarked = true;
            if (!entry->value.isNil()) {
                entry->value.asObj()->isMarked = true;
            }
        }
    }
}

void Table::clear() {
    delete[] entries;
    entries = nullptr;
    count = 0;
    capacity = 0;
}

}
