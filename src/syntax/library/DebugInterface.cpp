#include "DebugInterface.h"

#include <deque>
#include <functional>
#include <iostream>

void TVPEnsureDataPathDirectory() {
}

void tTJSNC_Debug::tTVPLogStreamHolder::Open(const wchar_t* const& mode) {
    if (OpenFailed) return; // no more try

    try {
        wchar_t filename[260];
        if (TVPLogLocation.GetLen() == 0) {
            Stream = nullptr;
            OpenFailed = true;
        } else {
            // no log location specified
            TJS_strcpy(filename, TVPNativeLogLocation);
            TJS_strcat(filename, TJS_W("./krkr.console.log"));
            TVPEnsureDataPathDirectory();
            Stream = _wfopen(filename, mode);
            if (!Stream) OpenFailed = true;
        }

        if (Stream) {
            fseek(Stream, 0, SEEK_END);
            if (ftell(Stream) == 0) {
                // write BOM
                // TODO: 32-bit unicode support
                fwrite(TJS_N("\xff\xfe"), 1, 2, Stream); // indicate unicode text
            }

            ttstr separator{TJS_W("\r")};
            Log(separator);

            static tjs_char timebuf[80];

            tm struct_tm{};
            time_t timer;
            timer = time(&timer);

            localtime_s(&struct_tm, &timer);
            TJS_strftime(timebuf, 79, TJS_W("%#c"), &struct_tm);

            Log(ttstr(TJS_W("Logging to ")) + ttstr(filename) + TJS_W(" started on ") + timebuf);
        }
    } catch (...) {
        OpenFailed = true;
    }
}

//---------------------------------------------------------------------------
void tTJSNC_Debug::tTVPLogStreamHolder::Clear() {
    // clear log text
    if (Stream) fclose(Stream);

    Open(TJS_W("wb"));
}

/**
 * 写入日志到文件
 * @param text 文本
 */
void tTJSNC_Debug::tTVPLogStreamHolder::Log(const ttstr& text) {
    if (!Stream) Open(TJS_W("ab"));

    try {
        if (Stream) {
            size_t len = text.GetLen() * sizeof(tjs_char);
            if (len != fwrite(text.c_str(), 1, len, Stream)) {
                // cannot write
                fclose(Stream);
                OpenFailed = true;
                return;
            }
            fwrite(TJS_W("\n"), 1, 1 * sizeof(tjs_char), Stream);

            // flush
            fflush(Stream);
        }
    } catch (...) {
        try {
            if (Stream) fclose(Stream);
        } catch (...) {
        }

        OpenFailed = true;
    }
}

void tTJSNC_Debug::TVPSetOnLog(const TVPLog& func) {
    TVPOnLog = func;
}

void tTJSNC_Debug::TVPCleanupLoggingHandlerVector() {
    // eliminate empty
    for (auto i = TVPLoggingHandlerVector.begin();
         i != TVPLoggingHandlerVector.end();) {
        if (!i->Object) {
            i->Release();
            i = TVPLoggingHandlerVector.erase(i);
        } else {
            ++i;
        }
    }
}

static bool TVPInDeliverLoggingEvent = false;

void tTJSNC_Debug::_TVPDeliverLoggingEvent(const ttstr& line) {
    if (!TVPInDeliverLoggingEvent) {
        TVPInDeliverLoggingEvent = true;
        try {
            if (TVPLoggingHandlerVector.size()) {
                bool emptyflag = false;
                tTJSVariant vline(line);
                tTJSVariant* pvline[] = {&vline};
                for (auto& i : TVPLoggingHandlerVector) {
                    if (i.Object) {
                        tjs_error er;
                        try {
                            er = i.FuncCall(0, nullptr,
                                            nullptr, nullptr,
                                            1, pvline, nullptr);
                        } catch (...) {
                            // failed
                            i.Release();
                            i.Object = i.ObjThis = nullptr;
                            throw;
                        }
                        if (TJS_FAILED(er)) {
                            // failed
                            i.Release();
                            i.Object = i.ObjThis = nullptr;
                            emptyflag = true;
                        }
                    } else {
                        emptyflag = true;
                    }
                }

                if (emptyflag) {
                    // the array has empty cell
                    TVPCleanupLoggingHandlerVector();
                }
            }

            if (!TVPLoggingHandlerVector.size()) {
                TVPSetOnLog(nullptr);
            }
        } catch (...) {
            TVPInDeliverLoggingEvent = false;
            throw;
        }
        TVPInDeliverLoggingEvent = false;
    }
}

void tTJSNC_Debug::TVPAddLoggingHandler(tTJSVariantClosure clo) {
    if (const auto i =
            std::find(TVPLoggingHandlerVector.begin(),
                      TVPLoggingHandlerVector.end(), clo);
        i == TVPLoggingHandlerVector.end()
    ) {
        clo.AddRef();
        TVPLoggingHandlerVector.push_back(clo);
        TVPSetOnLog([this](const ttstr& line) {
            _TVPDeliverLoggingEvent(line);
        });
    }
}


// TVPAddLog
void tTJSNC_Debug::TVPEnsureLogObjects() {
    if (TVPLogObjectsInitialized) return;
    TVPLogObjectsInitialized = true;

    TVPLogDeque = new std::deque<tTVPLogItem>();
    TVPImportantLogs = new ttstr();
}

void tTJSNC_Debug::TVPAddLog(const ttstr& line, bool appendtoimportant) {
    // add a line to the log.
    // exceeded lines over TVPLogMaxLines are eliminated.
    // this function is not thread-safe ...

    TVPEnsureLogObjects();
    if (!TVPLogDeque) return; // log system is shuttingdown
    if (!TVPImportantLogs) return; // log system is shuttingdown

    static time_t prevlogtime = 0;
    static ttstr prevtimebuf;
    static tjs_char timebuf[40];

    time_t timer;
    timer = time(&timer);

    if (prevlogtime != timer) {
        tm struct_tm{};
        localtime_s(&struct_tm, &timer);
        TJS_strftime(timebuf, 39, TJS_W("%H:%M:%S"), &struct_tm);
        prevlogtime = timer;
        prevtimebuf = timebuf;
    }

    TVPLogDeque->emplace_back(line, prevtimebuf);

    if (appendtoimportant) {
        *TVPImportantLogs += ttstr(timebuf) + TJS_W(" ! ") + line + TJS_W("\n");
    }

    while (TVPLogDeque->size() >= TVPLogMaxLines + 100) {
        auto i = TVPLogDeque->begin();
        TVPLogDeque->erase(i, i + 100);
    }

    const auto timebuflen = static_cast<tjs_int>(wcslen(timebuf));
    ttstr buf(static_cast<tTJSStringBufferLength>(timebuflen + 1 + line.GetLen()));
    tjs_char* p = buf.Independ();
    TJS_strcpy(p, timebuf);
    p += timebuflen;
    *p = TJS_W(' ');
    p++;
    TJS_strcpy(p, line.c_str());
    if (TVPOnLog) TVPOnLog(buf);
    std::wcout << buf.c_str();

    if (TVPLoggingToFile) TVPLogStreamHolder.Log(buf);
}

tTJSNC_Debug::tTJSNC_Debug() : tTJSNativeClass(TJS_W("Debug")) {
    // registration of native members
    TJSNativeClassRegisterNCM(
        this, TJS_W("finalize"),
        TJSCreateNativeClassMethod([] {
            return TJS_S_OK;
        }), ClassName, nitMethod);

    TJSNativeClassRegisterNCM(
        this, TJS_W("Debug"),
        TJSCreateNativeClassConstructor([] {
            return TJS_S_OK;
        }), ClassName,
        nitMethod);

    TJSNativeClassRegisterNCM(
        this, TJS_W("message"),
        TJSCreateNativeClassMethod([this](
            tTJSVariant*, const tjs_int numparams,
            tTJSVariant** param, iTJSDispatch2*
        ) {
                if (numparams < 1) return TJS_E_BADPARAMCOUNT;

                if (numparams == 1) {
                    TVPAddLog(*param[0]);
                } else {
                    // display the arguments separated with ", "
                    ttstr args;
                    for (int i = 0; i < numparams; i++) {
                        if (i != 0) args += TJS_W(", ");
                        args += ttstr(*param[i]);
                    }
                    TVPAddLog(args);
                }

                return TJS_S_OK;
            }), ClassName,
        nitMethod, TJS_STATICMEMBER);
}
