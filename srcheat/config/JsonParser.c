#include "JsonParser.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum JsonTypeEnum {
    JsonNull = 0,
    JsonBool = 1,
    JsonInt = 2,
    JsonFloat = 3,
    JsonStr = 4,
    JsonArr = 5,
    JsonObj = 6
};

static void SkipWs(const char** p, const char* end)
{
    while (*p < end && (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n'))
        (*p)++;
}

static int MatchLit(const char** p, const char* end, const char* lit, int n)
{
    if (end - *p < n)
        return 0;
    if (memcmp(*p, lit, (size_t)n) != 0)
        return 0;
    *p += n;
    return 1;
}

static char* DupRange(const char* s, long long n)
{
    char* o;
    if (n < 0)
        n = 0;
    o = (char*)malloc((size_t)n + 1);
    if (!o)
        return NULL;
    if (n)
        memcpy(o, s, (size_t)n);
    o[n] = 0;
    return o;
}

static int ParseStringRaw(const char** p, const char* end, char** out)
{
    char* buf;
    long long cap, len;
    if (*p >= end || **p != '"')
        return 0;
    (*p)++;
    cap = 32;
    len = 0;
    buf = (char*)malloc((size_t)cap);
    if (!buf)
        return 0;
    while (*p < end && **p != '"') {
        char c = **p;
        if (c == '\\') {
            (*p)++;
            if (*p >= end)
                break;
            c = **p;
            switch (c) {
            case 'n': c = '\n'; break;
            case 't': c = '\t'; break;
            case 'r': c = '\r'; break;
            case 'b': c = '\b'; break;
            case 'f': c = '\f'; break;
            case 'u': {
                int i;
                unsigned code = 0;
                for (i = 0; i < 4 && *p + 1 < end; i++) {
                    char h = *(*p + 1);
                    code <<= 4;
                    if (h >= '0' && h <= '9')
                        code |= (unsigned)(h - '0');
                    else if (h >= 'a' && h <= 'f')
                        code |= (unsigned)(h - 'a' + 10);
                    else if (h >= 'A' && h <= 'F')
                        code |= (unsigned)(h - 'A' + 10);
                    else
                        break;
                    (*p)++;
                }
                if (code < 0x80)
                    c = (char)code;
                else
                    c = '?';
                break;
            }
            default:
                break;
            }
            (*p)++;
        } else {
            (*p)++;
        }
        if (len + 1 >= cap) {
            char* nb;
            cap *= 2;
            nb = (char*)realloc(buf, (size_t)cap);
            if (!nb) {
                free(buf);
                return 0;
            }
            buf = nb;
        }
        buf[len++] = c;
    }
    if (*p >= end || **p != '"') {
        free(buf);
        return 0;
    }
    (*p)++;
    buf[len] = 0;
    *out = buf;
    return 1;
}

static struct JsonValue* ParseValue(const char** p, const char* end, char** errKey);

static int ParseObjectBody(const char** p, const char* end, struct JsonObject* out, char** errKey)
{
    struct JsonValue* tail = NULL;
    SkipWs(p, end);
    if (*p < end && **p == '}') {
        (*p)++;
        return 1;
    }
    for (;;) {
        struct JsonValue* v;
        char* key = NULL;
        SkipWs(p, end);
        if (!ParseStringRaw(p, end, &key)) {
            *errKey = DupRange("object key", 11);
            return 0;
        }
        SkipWs(p, end);
        if (*p >= end || **p != ':') {
            free(key);
            *errKey = DupRange("object separator", 16);
            return 0;
        }
        (*p)++;
        v = ParseValue(p, end, errKey);
        if (!v) {
            free(key);
            return 0;
        }
        v->key = key;
        if (tail)
            tail->next = v;
        else
            out->head = v;
        tail = v;
        out->count++;
        SkipWs(p, end);
        if (*p < end && **p == ',') {
            (*p)++;
            continue;
        }
        if (*p < end && **p == '}') {
            (*p)++;
            return 1;
        }
        *errKey = DupRange("object", 6);
        return 0;
    }
}

static int ParseArrayBody(const char** p, const char* end, struct JsonValue** headOut, char** errKey)
{
    struct JsonValue* head = NULL;
    struct JsonValue* tail = NULL;
    SkipWs(p, end);
    if (*p < end && **p == ']') {
        (*p)++;
        *headOut = NULL;
        return 1;
    }
    for (;;) {
        struct JsonValue* v = ParseValue(p, end, errKey);
        if (!v)
            return 0;
        v->key = NULL;
        if (tail)
            tail->next = v;
        else
            head = v;
        tail = v;
        SkipWs(p, end);
        if (*p < end && **p == ',') {
            (*p)++;
            continue;
        }
        if (*p < end && **p == ']') {
            (*p)++;
            *headOut = head;
            return 1;
        }
        *errKey = DupRange("array", 5);
        {
            struct JsonValue* it = head;
            while (it) {
                struct JsonValue* nx = it->next;
                free(it->strVal);
                free(it);
                it = nx;
            }
        }
        return 0;
    }
}

static struct JsonValue* ParseValue(const char** p, const char* end, char** errKey)
{
    struct JsonValue* v;
    SkipWs(p, end);
    if (*p >= end) {
        *errKey = DupRange("value", 5);
        return NULL;
    }
    v = (struct JsonValue*)calloc(1, sizeof(*v));
    if (!v)
        return NULL;
    if (**p == '{') {
        struct JsonObject nested;
        (*p)++;
        nested.head = NULL;
        nested.count = 0;
        if (!ParseObjectBody(p, end, &nested, errKey)) {
            free(v);
            JsonFreeObject(&nested);
            return NULL;
        }
        v->type = JsonObj;
        v->strVal = NULL;
        v->intVal = 0;
        v->next = nested.head;
        return v;
    }
    if (**p == '[') {
        struct JsonValue* arr = NULL;
        (*p)++;
        if (!ParseArrayBody(p, end, &arr, errKey)) {
            free(v);
            return NULL;
        }
        v->type = JsonArr;
        v->next = arr;
        return v;
    }
    if (**p == '"') {
        char* s = NULL;
        if (!ParseStringRaw(p, end, &s)) {
            free(v);
            *errKey = DupRange("value", 5);
            return NULL;
        }
        v->type = JsonStr;
        v->strVal = s;
        return v;
    }
    if (MatchLit(p, end, "true", 4)) {
        v->type = JsonBool;
        v->boolVal = 1;
        return v;
    }
    if (MatchLit(p, end, "false", 5)) {
        v->type = JsonBool;
        v->boolVal = 0;
        return v;
    }
    if (MatchLit(p, end, "null", 4)) {
        v->type = JsonNull;
        return v;
    }
    {
        const char* start = *p;
        int isFloat = 0;
        if (*p < end && (**p == '-' || **p == '+'))
            (*p)++;
        while (*p < end && ((**p >= '0' && **p <= '9') || **p == '.' || **p == 'e' || **p == 'E' || **p == '-' || **p == '+')) {
            if (**p == '.' || **p == 'e' || **p == 'E')
                isFloat = 1;
            (*p)++;
        }
        if (*p == start) {
            free(v);
            *errKey = DupRange("value", 5);
            return NULL;
        }
        if (isFloat) {
            v->type = JsonFloat;
            v->numVal = strtod(start, NULL);
        } else {
            v->type = JsonInt;
            v->intVal = strtoll(start, NULL, 10);
        }
        return v;
    }
}

int JsonParseObject(const char* text, long long len, struct JsonObject* out)
{
    const char* p;
    const char* end;
    char* errKey = NULL;
    if (!text || !out)
        return 0;
    out->head = NULL;
    out->count = 0;
    p = text;
    end = text + len;
    SkipWs(&p, end);
    if (p >= end || *p != '{') {
        if (p < end && *p == 0)
            return 0;
        return 0;
    }
    p++;
    if (!ParseObjectBody(&p, end, out, &errKey)) {
        JsonFreeObject(out);
        free(errKey);
        return 0;
    }
    SkipWs(&p, end);
    if (p < end && *p != 0 && p != end) {
        JsonFreeObject(out);
        return 0;
    }
    return 1;
}

static void FreeValueDeep(struct JsonValue* v)
{
    while (v) {
        struct JsonValue* nx = v->next;
        if (v->type == JsonStr || v->type == JsonArr || v->type == JsonObj || v->strVal) {
            if (v->type == JsonArr || v->type == JsonObj)
                FreeValueDeep(v->next);
            else
                free(v->strVal);
        }
        free(v->key);
        free(v);
        v = nx;
    }
}

void JsonFreeObject(struct JsonObject* obj)
{
    if (!obj)
        return;
    FreeValueDeep(obj->head);
    obj->head = NULL;
    obj->count = 0;
}

struct JsonValue* JsonFindKey(const struct JsonObject* obj, const char* key)
{
    struct JsonValue* it;
    if (!obj || !key)
        return NULL;
    it = obj->head;
    while (it) {
        if (it->key && strcmp(it->key, key) == 0)
            return it;
        it = it->next;
    }
    return NULL;
}

int JsonGetInt(const struct JsonObject* obj, const char* key, int fallback)
{
    struct JsonValue* v = JsonFindKey(obj, key);
    if (!v)
        return fallback;
    if (v->type == JsonInt || v->type == JsonBool)
        return (int)v->intVal;
    if (v->type == JsonFloat)
        return (int)v->numVal;
    return fallback;
}

float JsonGetFloat(const struct JsonObject* obj, const char* key, float fallback)
{
    struct JsonValue* v = JsonFindKey(obj, key);
    if (!v)
        return fallback;
    if (v->type == JsonFloat)
        return (float)v->numVal;
    if (v->type == JsonInt || v->type == JsonBool)
        return (float)v->intVal;
    return fallback;
}

int JsonGetBool(const struct JsonObject* obj, const char* key, int fallback)
{
    struct JsonValue* v = JsonFindKey(obj, key);
    if (!v)
        return fallback;
    if (v->type == JsonBool || v->type == JsonInt)
        return v->intVal ? 1 : 0;
    if (v->type == JsonFloat)
        return v->numVal != 0.0;
    return fallback;
}

const char* JsonGetString(const struct JsonObject* obj, const char* key, const char* fallback)
{
    struct JsonValue* v = JsonFindKey(obj, key);
    if (!v || v->type != JsonStr || !v->strVal)
        return fallback;
    return v->strVal;
}

int JsonGetColor(const struct JsonObject* obj, const char* key, struct ColorRgba* out)
{
    struct JsonValue* v;
    if (!out)
        return 0;
    v = JsonFindKey(obj, key);
    if (!v)
        return 0;
    if (v->type == JsonStr && v->strVal) {
        float r = 0, g = 0, b = 0, a = 1;
        int n = sscanf(v->strVal, "%f,%f,%f,%f", &r, &g, &b, &a);
        if (n >= 3) {
            out->r = r;
            out->g = g;
            out->b = b;
            out->a = (n >= 4) ? a : 1.0f;
            return 1;
        }
        n = sscanf(v->strVal, "%f %f %f %f", &r, &g, &b, &a);
        if (n >= 3) {
            out->r = r;
            out->g = g;
            out->b = b;
            out->a = (n >= 4) ? a : 1.0f;
            return 1;
        }
    }
    if (v->type == JsonArr) {
        struct JsonValue* e = v->next;
        float f[4];
        int i;
        for (i = 0; i < 4 && e; i++, e = e->next) {
            if (e->type == JsonFloat)
                f[i] = (float)e->numVal;
            else if (e->type == JsonInt)
                f[i] = (float)e->intVal;
            else
                f[i] = 0.0f;
        }
        if (i >= 3) {
            out->r = f[0];
            out->g = f[1];
            out->b = f[2];
            out->a = (i >= 4) ? f[3] : 1.0f;
            return 1;
        }
    }
    return 0;
}

static int WriterGrow(struct JsonWriter* w, long long need)
{
    long long nc;
    char* nb;
    if (w->len + need + 1 <= w->cap)
        return 1;
    nc = w->cap ? w->cap : 256;
    while (w->len + need + 1 > nc)
        nc *= 2;
    nb = (char*)realloc(w->buf, (size_t)nc);
    if (!nb)
        return 0;
    w->buf = nb;
    w->cap = nc;
    return 1;
}

static int WriterAppend(struct JsonWriter* w, const char* s, long long n)
{
    if (!WriterGrow(w, n))
        return 0;
    memcpy(w->buf + w->len, s, (size_t)n);
    w->len += n;
    w->buf[w->len] = 0;
    return 1;
}

int JsonWriterInit(struct JsonWriter* w, long long cap)
{
    if (!w || cap <= 0)
        return 0;
    w->buf = (char*)malloc((size_t)cap);
    if (!w->buf)
        return 0;
    w->buf[0] = 0;
    w->len = 0;
    w->cap = cap;
    w->needComma = 0;
    return 1;
}

void JsonWriterFree(struct JsonWriter* w)
{
    if (!w)
        return;
    free(w->buf);
    w->buf = NULL;
    w->len = 0;
    w->cap = 0;
    w->needComma = 0;
}

static int WriterMaybeComma(struct JsonWriter* w)
{
    if (w->needComma)
        return WriterAppend(w, ",", 1);
    return 1;
}

int JsonWriterBeginObject(struct JsonWriter* w)
{
    if (!w)
        return 0;
    if (!WriterMaybeComma(w))
        return 0;
    w->needComma = 0;
    return WriterAppend(w, "{", 1);
}

int JsonWriterEndObject(struct JsonWriter* w)
{
    if (!w)
        return 0;
    w->needComma = 1;
    return WriterAppend(w, "}", 1);
}

int JsonWriterKey(struct JsonWriter* w, const char* key)
{
    long long n;
    if (!w || !key)
        return 0;
    if (!WriterMaybeComma(w))
        return 0;
    n = (long long)strlen(key);
    if (!WriterAppend(w, "\"", 1))
        return 0;
    if (!WriterAppend(w, key, n))
        return 0;
    if (!WriterAppend(w, "\":", 2))
        return 0;
    w->needComma = 0;
    return 1;
}

int JsonWriterInt(struct JsonWriter* w, long long v)
{
    char tmp[32];
    int n;
    if (!w)
        return 0;
    if (!WriterMaybeComma(w))
        return 0;
    n = snprintf(tmp, sizeof(tmp), "%lld", v);
    if (n <= 0)
        return 0;
    w->needComma = 1;
    return WriterAppend(w, tmp, n);
}

int JsonWriterFloat(struct JsonWriter* w, float v)
{
    char tmp[48];
    int n;
    if (!w)
        return 0;
    if (!WriterMaybeComma(w))
        return 0;
    n = snprintf(tmp, sizeof(tmp), "%.6g", (double)v);
    if (n <= 0)
        return 0;
    w->needComma = 1;
    return WriterAppend(w, tmp, n);
}

int JsonWriterBool(struct JsonWriter* w, int v)
{
    if (!w)
        return 0;
    if (!WriterMaybeComma(w))
        return 0;
    w->needComma = 1;
    if (v)
        return WriterAppend(w, "true", 4);
    return WriterAppend(w, "false", 5);
}

int JsonWriterString(struct JsonWriter* w, const char* s)
{
    if (!w)
        return 0;
    if (!WriterMaybeComma(w))
        return 0;
    if (!WriterAppend(w, "\"", 1))
        return 0;
    if (s) {
        long long n = (long long)strlen(s);
        if (!WriterAppend(w, s, n))
            return 0;
    }
    w->needComma = 1;
    return WriterAppend(w, "\"", 1);
}

int JsonWriterColor(struct JsonWriter* w, const struct ColorRgba* c)
{
    char tmp[96];
    int n;
    if (!w || !c)
        return 0;
    if (!WriterMaybeComma(w))
        return 0;
    n = snprintf(tmp, sizeof(tmp), "\"%.4f,%.4f,%.4f,%.4f\"",
        (double)c->r, (double)c->g, (double)c->b, (double)c->a);
    if (n <= 0)
        return 0;
    w->needComma = 1;
    return WriterAppend(w, tmp, n);
}
