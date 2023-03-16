#pragma once

/*
* This header only library was taken from
* https://github.com/DefinitelyNotAnmol/VisualBenchmarking/blob/master/benchmark.h
* And customized
* Customized version : https://github.com/mhdshameel/VisualBenchmarking
* */

#define ON 1
#define OFF 0

#ifndef BENCHMARKING
#define BENCHMARKING ON
#endif // BENCHMARKING

#include <string>
#include <chrono>
#include <fstream>

struct ProfileResult {
    std::string Name;
    long long Start, End;
    ProfileResult(const char* name, long long start, long long end)
        : Name(name), Start(start), End(end) {}
};

struct InstrumentationSession {
    std::string Name;
};

class IInstrumentor
{
    static std::once_flag _creation_once_flag;
public:
    IInstrumentor() : m_CurrentSession(nullptr) {}
    InstrumentationSession* m_CurrentSession;

    virtual ~IInstrumentor() {}
    virtual void EndSession() = 0;
    virtual void BeginSession(const std::string& name) = 0;
    virtual void WriteProfile(const ProfileResult& result) = 0;

    static IInstrumentor& Get(bool output_to_console = false);

    void SetCurrentSessionName(const std::string& name)
    {
        m_CurrentSession = new InstrumentationSession;
        m_CurrentSession->Name = name;
    }
};

class FilestreamInstrumentor : public IInstrumentor {
    std::ofstream m_OutputStream;
    int m_ProfileCount;
    const std::string filepath = "results.json";
    std::mutex write_mx;

public:

    virtual void BeginSession(const std::string& name) {
        m_OutputStream.open(filepath);
        WriteHeader();
        __super::SetCurrentSessionName(name);
    }

    virtual void EndSession() {
        WriteFooter();
        m_OutputStream.close();
        delete m_CurrentSession;
        m_CurrentSession = nullptr;
        m_ProfileCount = 0;
    }

    virtual void WriteProfile(const ProfileResult& result) {
        std::unique_lock lk(write_mx);
        if (m_ProfileCount++ > 0)
            m_OutputStream << ",";

        std::string name = result.Name;
        std::replace(name.begin(), name.end(), '"', '\'');

        m_OutputStream << "{";
        m_OutputStream << "\"cat\":\"function\",";
        m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
        m_OutputStream << "\"name\":\"" << name << "\",";
        m_OutputStream << "\"ph\":\"X\",";
        m_OutputStream << "\"pid\":0,";
        m_OutputStream << "\"tid\":0,";
        m_OutputStream << "\"ts\":" << result.Start;
        m_OutputStream << "}";

        m_OutputStream.flush();
    }

    void WriteHeader() {
        m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
        m_OutputStream.flush();
    }

    void WriteFooter() {
        m_OutputStream << "]}";
        m_OutputStream.flush();
    }
};

class ConsoleInstrumentator : public IInstrumentor {
    std::mutex cout_mx;
public:
    ConsoleInstrumentator() {}

    virtual void BeginSession(const std::string& name) {
        SetCurrentSessionName(name);
    }

    virtual void EndSession() {
        delete m_CurrentSession;
        m_CurrentSession = nullptr;
    }

    virtual void WriteProfile(const ProfileResult& result) {
        std::string name = result.Name;
        std::replace(name.begin(), name.end(), '"', '\'');
        auto lk = std::unique_lock(cout_mx);
        std::cout << "name: " << name << ", ";
        std::cout << "dur: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::microseconds(result.End - result.Start)) << ", ";
        std::cout << "ts: " << result.Start;
        std::cout << std::endl;
    }
};

std::once_flag IInstrumentor::_creation_once_flag = std::once_flag();

IInstrumentor& IInstrumentor::Get(bool output_to_console) {
    static IInstrumentor* instance = nullptr;
    std::call_once(_creation_once_flag,
        [output_to_console]() {
            if (output_to_console)
                instance = new ConsoleInstrumentator();
            else
                instance = new FilestreamInstrumentor();
        }
    ); //thread safe initialization
    return *instance;
}

class InstrumentationTimer {
    const char* m_Name;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
    bool m_Stopped;
public:
    InstrumentationTimer(const char* name)
        : m_Name(name), m_Stopped(false) {

        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }

    ~InstrumentationTimer() {
        if (!m_Stopped)
            Stop();
    }

    void Stop() {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

        IInstrumentor::Get().WriteProfile(ProfileResult(m_Name, start, end));

        m_Stopped = true;
    }
};

#if BENCHMARKING
#define PROFILE_SCOPE(name) InstrumentationTimer timer(name)
#define START_SESSION(name) IInstrumentor::Get().BeginSession(name)
#define START_CONSOLE_SESSION(name) IInstrumentor::Get(true).BeginSession(name)
#define END_SESSION() IInstrumentor::Get().EndSession()
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)
#define PROFILE_FUNCTION_DETAILED() PROFILE_SCOPE(__PRETTY_FUNCTION__)
#else
#define START_SESSION(name)
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION() 
#define PROFILE_FUNCTION_DETAILED()
#define START_CONSOLE_SESSION(name)
#define END_SESSION()
#endif

