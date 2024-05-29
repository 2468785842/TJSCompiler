//---------------------------------------------------------------------------
/*
	TJS2 Script Engine
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// string heap management used by tTJSVariant and tTJSString
//---------------------------------------------------------------------------
#ifndef tjsVariantStringH
#define tjsVariantStringH

#include <cstring>
#include <memory>

#include "tjsConfig.h"


namespace TJS {
    class tTJSVariant;
    //---------------------------------------------------------------------------
    // tTJSVariantString stuff
    //---------------------------------------------------------------------------
#define TJS_VS_SHORT_LEN 21

    class tTJSVariantString;

    tjs_int TJSGetShorterStrLen(const tjs_char *str, tjs_int max);

    tTJSVariantString *TJSAllocStringHeap();

    void TJSDeallocStringHeap(tTJSVariantString *vs);

    void TJSThrowStringAllocError();

    void TJSThrowNarrowToWideConversionError();

    void TJSCompactStringHeap();

#ifdef TJS_DEBUG_DUMP_STRING
    extern void TJSDumpStringHeap(void);
#endif
    //---------------------------------------------------------------------------


    //---------------------------------------------------------------------------
    // base memory allocation functions for long string
    //---------------------------------------------------------------------------
    tjs_char *TJSVS_malloc(tjs_uint len);

    tjs_char *TJSVS_realloc(tjs_char *buf, tjs_uint len);

    void TJSVS_free(const tjs_char *buf);


    // tTJSVariantString
    struct tTJSVariantString_S {
        tjs_int RefCount; // reference count - 1
        tjs_char *LongString;
        tjs_char ShortString[TJS_VS_SHORT_LEN + 1];
        tjs_int Length; // string length
        tjs_uint32 HeapFlag;
        tjs_uint32 Hint;
    };

    /*start-of-tTJSVariantString*/
    class tTJSVariantString : public tTJSVariantString_S {
    public:
        TJS_METHOD_DEF(void, AddRef, ()) {
            RefCount++;
        }

        TJS_METHOD_DEF(void, Release, ());

        void SetString(const tjs_char *ref, const tjs_int maxlen = -1) {
            if (LongString) {
                TJSVS_free(LongString);
            }
            size_t len;
            if (maxlen != -1)
                len = TJSGetShorterStrLen(ref, maxlen);
            else
                len = wcslen(ref);

            if (len > TJS_VS_SHORT_LEN) {
                LongString = TJSVS_malloc(len + 1);
                wcsncpy_s(LongString, len + 1, ref, len);
            } else {
                wcsncpy_s(ShortString, TJS_VS_SHORT_LEN + 1, ref, len);
            }
            Length = static_cast<tjs_int>(len);
        }

        void SetString(const tjs_nchar *ref) {
            if (LongString) {
                TJSVS_free(LongString);
            }
            const int len = static_cast<tjs_int>(strlen(ref));
            if (len == -1) TJSThrowNarrowToWideConversionError();

            size_t numCvt{};
            if (len > TJS_VS_SHORT_LEN) {
                LongString = TJSVS_malloc(len + 1);
            }

            mbsrtowcs_s(&numCvt, len > TJS_VS_SHORT_LEN ? LongString : ShortString, len + 1, &ref, len, nullptr);
            Length = static_cast<tjs_int>(numCvt);
        }

        void AllocBuffer(const tjs_int len) {
            /* note that you must call FixLength if you allocate larger than the
                actual string size */

            if (LongString) {
                TJSVS_free(LongString);
            }

            Length = len;
            if (len > TJS_VS_SHORT_LEN) {
                LongString = TJSVS_malloc(len + 1);
                LongString[len] = 0;
            } else {
                ShortString[len] = 0;
            }
        }

        TJS_METHOD_DEF(void, ResetString, (const tjs_char *ref)) {
            if (LongString) {
                TJSVS_free(LongString);
            }
            SetString(ref);
        }


        TJS_METHOD_DEF(void, AppendBuffer, (const tjs_int applen)) {
            /* note that you must call FixLength if you allocate larger than the
                actual string size */

            // assume this != NULL
            const tjs_int newlen = Length += applen;
            if (LongString) {
                // still long string
                LongString = TJSVS_realloc(LongString, newlen + 1);
                LongString[newlen] = 0;
                return;
            }
            if (newlen <= TJS_VS_SHORT_LEN) {
                // still short string
                ShortString[newlen] = 0;
                return;
            }
            // becomes a long string
            tjs_char *newbuf = TJSVS_malloc(newlen + 1);
            TJS_strcpy(newbuf, ShortString);
            LongString = newbuf;
            LongString[newlen] = 0;
        }

        TJS_METHOD_DEF(void, Append, (const tjs_char *str)) {
            // assume this != NULL
            Append(str, static_cast<tjs_int>(wcslen(str)));
        }

        TJS_METHOD_DEF(void, Append, (const tjs_char *str, const tjs_int applen)) {
            // assume this != NULL
            const tjs_int orglen = Length;
            const tjs_int newlen = Length += applen;
            if (LongString) {
                // still long string
                LongString = TJSVS_realloc(LongString, newlen + 1);
                TJS_strcpy(LongString + orglen, str);
                return;
            }
            if (newlen <= TJS_VS_SHORT_LEN) {
                // still short string
                TJS_strcpy(ShortString + orglen, str);
                return;
            }
            // becomes a long string
            tjs_char *newbuf = TJSVS_malloc(newlen + 1);
            TJS_strcpy(newbuf, ShortString);
            TJS_strcpy(newbuf + orglen, str);
            LongString = newbuf;
        }

        TJS_CONST_METHOD_DEF(TJS_METHOD_RET(const tjs_char *), operator const tjs_char *, ()) {
            return LongString ? LongString : ShortString;
        }

        TJS_CONST_METHOD_DEF(tjs_int, GetLength, ()) {
            return Length;
        }

        TJS_METHOD_DEF(tTJSVariantString *, FixLength, ());

        TJS_METHOD_DEF(tjs_uint32 *, GetHint, ()) { return &Hint; }

        TJS_CONST_METHOD_DEF(tTVInteger, ToInteger, ());

        TJS_CONST_METHOD_DEF(tTVReal, ToReal, ());

        TJS_CONST_METHOD_DEF(void, ToNumber, (tTJSVariant & dest));

        TJS_CONST_METHOD_DEF(tjs_int, GetRefCount, ()) {
            return RefCount;
        }

        [[nodiscard]] size_t QueryPersistSize() const {
            return sizeof(tjs_uint) + GetLength() * sizeof(tjs_char);
        }

        void Persist(tjs_uint8 *dest) const {
            tjs_uint size;
            const tjs_char *ptr = LongString ? LongString : ShortString;
            *reinterpret_cast<tjs_uint *>(dest) = size = GetLength();
            dest += sizeof(tjs_uint);
            while (size--) {
                *reinterpret_cast<tjs_char *>(dest) = *ptr;
                dest += sizeof(tjs_char);
                ptr++;
            }
        }
    };

    /*end-of-tTJSVariantString*/
    //---------------------------------------------------------------------------


    //---------------------------------------------------------------------------
    TJS_EXP_FUNC_DEF(tTJSVariantString *, TJSAllocVariantString, (const tjs_char *ref1,
            const tjs_char *ref2));

    TJS_EXP_FUNC_DEF(tTJSVariantString *, TJSAllocVariantString, (const tjs_char *ref, tjs_int n));

    TJS_EXP_FUNC_DEF(tTJSVariantString *, TJSAllocVariantString, (const tjs_char *ref));

    TJS_EXP_FUNC_DEF(tTJSVariantString *, TJSAllocVariantString, (const tjs_nchar *ref));

    TJS_EXP_FUNC_DEF(tTJSVariantString *, TJSAllocVariantString, (const tjs_uint8 **src));

    TJS_EXP_FUNC_DEF(tTJSVariantString *, TJSAllocVariantStringBuffer, (tjs_uint len));

    TJS_EXP_FUNC_DEF(tTJSVariantString *, TJSAppendVariantString, (tTJSVariantString * str,
            const tjs_char *app));

    TJS_EXP_FUNC_DEF(tTJSVariantString *, TJSAppendVariantString, (tTJSVariantString * str,
            const tTJSVariantString *app));

    TJS_EXP_FUNC_DEF(tTJSVariantString *, TJSFormatString, (const tjs_char *format, tjs_uint numparams,
            tTJSVariant **params));

    //---------------------------------------------------------------------------
} // namespace TJS
#endif
