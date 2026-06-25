#ifndef CB_CRITICAL_SECTION
#define CB_CRITICAL_SECTION

// Portable recursive critical section. Originally three branches (Win32
// CRITICAL_SECTION / pthread / a silent no-op stub); replaced 2026-06-25 with a
// single std::recursive_mutex during the macOS port. C++17 (the build already
// requires cxx_std_17), recursive to match the old Win32/pthread behaviour, and
// never a silent no-op lock. Keep the Enter()/Leave() API the callers expect.
#include <mutex>

class CriticalSection
{
private:
    std::recursive_mutex _mutex;

public:
    CriticalSection() = default;
    ~CriticalSection() = default;

    // Non-copyable / non-movable (std::recursive_mutex is neither).
    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;

    void Enter() { _mutex.lock(); }
    void Leave() { _mutex.unlock(); }
};

class CriticalSectionLock
{
private:
    CriticalSection* _criticalSection;
    bool _lock;

public:
    CriticalSectionLock(CriticalSection& criticalSection, bool lock = true) 
        : _criticalSection(&criticalSection)
        , _lock(lock)
    {
        if (_lock) _criticalSection->Enter();
    }
    ~CriticalSectionLock() 
    {
        if (_lock) _criticalSection->Leave();
    }
};

#endif
