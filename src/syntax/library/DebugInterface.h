#pragma once

#include <deque>

#include "tjsNative.h"

namespace TJS {
    class tTJSNC_Debug final : public tTJSNativeClass {
        struct tTVPLogItem {
            ttstr Log;
            ttstr Time;

            tTVPLogItem(const ttstr& log, const ttstr& time):
                Log(log), Time(time) {
            }
        };

        static constexpr tjs_uint TVPLogMaxLines{2048};
        static inline ttstr TVPLogLocation{};
        static inline wchar_t TVPNativeLogLocation[260]{};

        std::vector<tTJSVariantClosure> TVPLoggingHandlerVector{};
        std::deque<tTVPLogItem>* TVPLogDeque{};
        ttstr* TVPImportantLogs{nullptr};
        bool TVPLogObjectsInitialized{false};
        bool TVPLoggingToFile{true};

        typedef std::function<void(const ttstr&)> TVPLog;
        TVPLog TVPOnLog{};

        // log stream holder
        class tTVPLogStreamHolder {
            FILE* Stream;
            bool Alive;
            bool OpenFailed;

        public:
            tTVPLogStreamHolder() {
                Stream = nullptr;
                Alive = true;
                OpenFailed = false;
            }

            ~tTVPLogStreamHolder() {
                if (Stream) fclose(Stream);
                Alive = false;
            }

            void Clear(); // clear log stream
            void Log(const ttstr& text); // log given text

            void Reopen() {
                if (Stream) fclose(Stream);
                Stream = nullptr;
                Alive = false;
                OpenFailed = false;
            } // reopen log stream

            void Open(const wchar_t* const& mode);
        } TVPLogStreamHolder{};

        void TVPAddLog(const ttstr&, bool = false);
        void TVPEnsureLogObjects();
        void TVPSetOnLog(const TVPLog& func);
        void TVPAddLoggingHandler(tTJSVariantClosure);
        void TVPCleanupLoggingHandlerVector();
        void _TVPDeliverLoggingEvent(const ttstr&);

    public:
        tjs_uint32 ClassID = -1;
        tTJSNC_Debug();
    };

    static tTJSNativeClass* TVPCreateNativeClass_Debug() {
        return new tTJSNC_Debug();
    }
}
