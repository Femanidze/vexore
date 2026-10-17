#ifndef VEXORE_JSONPARSER_H
#define VEXORE_JSONPARSER_H

#include "../core/Types.h"

struct JsonValue {
    int type;
    union {
        int boolVal;
        long long intVal;
        double numVal;
        char* strVal;
    };
    struct JsonValue* next;
    char* key;
};

struct JsonObject {
    struct JsonValue* head;
    int count;
};

int JsonParseObject(const char* text, long long len, struct JsonObject* out);
void JsonFreeObject(struct JsonObject* obj);
struct JsonValue* JsonFindKey(const struct JsonObject* obj, const char* key);
int JsonGetInt(const struct JsonObject* obj, const char* key, int fallback);
float JsonGetFloat(const struct JsonObject* obj, const char* key, float fallback);
int JsonGetBool(const struct JsonObject* obj, const char* key, int fallback);
const char* JsonGetString(const struct JsonObject* obj, const char* key, const char* fallback);
int JsonGetColor(const struct JsonObject* obj, const char* key, struct ColorRgba* out);

struct JsonWriter {
    char* buf;
    long long len;
    long long cap;
    int needComma;
};

int JsonWriterInit(struct JsonWriter* w, long long cap);
void JsonWriterFree(struct JsonWriter* w);
int JsonWriterBeginObject(struct JsonWriter* w);
int JsonWriterEndObject(struct JsonWriter* w);
int JsonWriterKey(struct JsonWriter* w, const char* key);
int JsonWriterInt(struct JsonWriter* w, long long v);
int JsonWriterFloat(struct JsonWriter* w, float v);
int JsonWriterBool(struct JsonWriter* w, int v);
int JsonWriterString(struct JsonWriter* w, const char* s);
int JsonWriterColor(struct JsonWriter* w, const struct ColorRgba* c);

#endif
