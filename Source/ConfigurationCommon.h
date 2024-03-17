#pragma once
#define PROPERTY( name, type, defaultValue ) \
    public: \
        type name() { return _##name; } \
        void name(type value) { _##name = value; } \
    private: \
        type _##name = defaultValue;

#define PROPERTY_CONST( name, type, defaultValue ) \
    public: \
        type name() { return _##name; } \
        void name(type const& value) { _##name = value; } \
    private: \
        type _##name = defaultValue;
