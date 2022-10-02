#ifndef THREADTOOLS_H
#define THREADTOOLS_H

inline void ThreadSleep(unsigned nMilliseconds)
{
#ifdef _WIN32

#ifdef PLATFORM_WINDOWS_PC // This is performed by the game module!
	//static bool bInitialized = false;
	//if (!bInitialized)
	//{
	//	bInitialized = true;
	//	// Set the timer resolution to 1 ms (default is 10.0, 15.6, 2.5, 1.0 or
	//	// some other value depending on hardware and software) so that we can
	//	// use Sleep( 1 ) to avoid wasting CPU time without missing our frame
	//	// rate.
	//	timeBeginPeriod(1);
	//}
#endif

	Sleep(nMilliseconds);
#elif PLATFORM_PS3
	if (nMilliseconds == 0)
	{
		// sys_ppu_thread_yield doesn't seem to function properly, so sleep instead.
		sys_timer_usleep(60);
	}
	else
	{
		sys_timer_usleep(nMilliseconds * 1000);
	}
#elif defined(POSIX)
	usleep(nMilliseconds * 1000);
#endif
}
inline void ThreadPause()
{
#if defined( COMPILER_PS3 )
	__db16cyc();
#elif defined( COMPILER_GCC )
	__asm __volatile("pause");
#elif defined ( COMPILER_MSVC64 )
	_mm_pause();
#elif defined( COMPILER_MSVC32 )
	__asm pause;
#elif defined( COMPILER_MSVCX360 )
	YieldProcessor();
	__asm { or r0, r0, r0 }
	YieldProcessor();
	__asm { or r1, r1, r1 }
#else
#error "implement me"
#endif
}

bool ThreadInMainThread();
bool ThreadInRenderThread();
bool ThreadInServerFrameThread();
ThreadId_t ThreadGetCurrentId();

//-----------------------------------------------------------------------------
//
// Interlock methods. These perform very fast atomic thread
// safe operations. These are especially relevant in a multi-core setting.
//
//-----------------------------------------------------------------------------

LONG ThreadInterlockedCompareExchange64(LONG volatile* pDest, int64 value, int64 comperand);
bool ThreadInterlockedAssignIf(LONG volatile* p, int32 value, int32 comperand);
int64 ThreadInterlockedCompareExchange64(int64 volatile* pDest, int64 value, int64 comperand);
bool ThreadInterlockedAssignIf64(int64 volatile* pDest, int64 value, int64 comperand);

#ifdef _WIN32
#define NOINLINE
#elif defined( _PS3 )
#define NOINLINE __attribute__ ((noinline))
#elif defined(POSIX)
#define NOINLINE __attribute__ ((noinline))
#endif

#if defined( _X360 ) || defined( _PS3 )
#define ThreadMemoryBarrier() __lwsync()
#elif defined(COMPILER_MSVC)
// Prevent compiler reordering across this barrier. This is
// sufficient for most purposes on x86/x64.
#define ThreadMemoryBarrier() _ReadWriteBarrier()
#elif defined(COMPILER_GCC)
// Prevent compiler reordering across this barrier. This is
// sufficient for most purposes on x86/x64.
// http://preshing.com/20120625/memory-ordering-at-compile-time
#define ThreadMemoryBarrier() asm volatile("" ::: "memory")
#else
#error Every platform needs to define ThreadMemoryBarrier to at least prevent compiler reordering
#endif

//-----------------------------------------------------------------------------
//
// A super-fast thread-safe integer A simple class encapsulating the notion of an 
// atomic integer used across threads that uses the built in and faster 
// "interlocked" functionality rather than a full-blown mutex. Useful for simple 
// things like reference counts, etc.
//
//-----------------------------------------------------------------------------

template <typename T>
class CInterlockedIntT
{
public:
	CInterlockedIntT() : m_value(0) { static_assert((sizeof(T) == sizeof(int32)) || (sizeof(T) == sizeof(int64))); }

	CInterlockedIntT(T value) : m_value(value) {}

	T operator()(void) const { return m_value; }
	operator T() const { return m_value; }

	bool operator!() const { return (m_value == 0); }
	bool operator==(T rhs) const { return (m_value == rhs); }
	bool operator!=(T rhs) const { return (m_value != rhs); }

	T operator++() {
		if (sizeof(T) == sizeof(int32))
			return (T)ThreadInterlockedIncrement((int32*)&m_value);
		else
			return (T)ThreadInterlockedIncrement64((int64*)&m_value);
	}
	T operator++(int) { return operator++() - 1; }

	T operator--() {
		if (sizeof(T) == sizeof(int32))
			return (T)ThreadInterlockedDecrement((int32*)&m_value);
		else
			return (T)ThreadInterlockedDecrement64((int64*)&m_value);
	}

	T operator--(int) { return operator--() + 1; }

	bool AssignIf(T conditionValue, T newValue)
	{
		if (sizeof(T) == sizeof(int32))
			return ThreadInterlockedAssignIf((LONG*)&m_value, (int32)newValue, (int32)conditionValue);
		else
			return ThreadInterlockedAssignIf64((int64*)&m_value, (int64)newValue, (int64)conditionValue);
	}


	T operator=(T newValue) {
		if (sizeof(T) == sizeof(int32))
			ThreadInterlockedExchange((int32*)&m_value, newValue);
		else
			ThreadInterlockedExchange64((int64*)&m_value, newValue);
		return m_value;
	}

	// Atomic add is like += except it returns the previous value as its return value
	T AtomicAdd(T add) {
		if (sizeof(T) == sizeof(int32))
			return (T)ThreadInterlockedExchangeAdd((int32*)&m_value, (int32)add);
		else
			return (T)ThreadInterlockedExchangeAdd64((int64*)&m_value, (int64)add);
	}


	void operator+=(T add) {
		if (sizeof(T) == sizeof(int32))
			ThreadInterlockedExchangeAdd((int32*)&m_value, (int32)add);
		else
			ThreadInterlockedExchangeAdd64((int64*)&m_value, (int64)add);
	}

	void operator-=(T subtract) { operator+=(-subtract); }
	void operator*=(T multiplier) {
		T original, result;
		do
		{
			original = m_value;
			result = original * multiplier;
		} while (!AssignIf(original, result));
	}
	void operator/=(T divisor) {
		T original, result;
		do
		{
			original = m_value;
			result = original / divisor;
		} while (!AssignIf(original, result));
	}

	T operator+(T rhs) const { return m_value + rhs; }
	T operator-(T rhs) const { return m_value - rhs; }

	T InterlockedExchange(T newValue) {
		if (sizeof(T) == sizeof(int32))
			return (T)ThreadInterlockedExchange((int32*)&m_value, newValue);
		else
			return (T)ThreadInterlockedExchange64((int64*)&m_value, newValue);
	}

private:
	volatile T m_value;
};

typedef CInterlockedIntT<int> CInterlockedInt;
typedef CInterlockedIntT<unsigned> CInterlockedUInt;

//=============================================================================
class CThreadFastMutex;

inline CMemory p_MutexInternal_WaitForLock;
inline auto v_MutexInternal_WaitForLock = p_MutexInternal_WaitForLock.RCast<int (*)(CThreadFastMutex* mutex)>();

inline CMemory p_MutexInternal_ReleaseWaiter;
inline auto v_MutexInternal_ReleaseWaiter = p_MutexInternal_ReleaseWaiter.RCast<int (*)(CThreadFastMutex* mutex)>();

inline CMemory p_DeclareCurrentThreadIsMainThread;
inline auto v_DeclareCurrentThreadIsMainThread = p_DeclareCurrentThreadIsMainThread.RCast<ThreadId_t (*)(void)>();

inline ThreadId_t* g_ThreadMainThreadID = nullptr;
inline ThreadId_t g_ThreadRenderThreadID = NULL;
inline ThreadId_t* g_ThreadServerFrameThreadID = nullptr;

///////////////////////////////////////////////////////////////////////////////
class CThreadFastMutex
{
public:
	int WaitForLock(void) {
		return v_MutexInternal_WaitForLock(this);
	}
	int ReleaseWaiter(void) {
		return v_MutexInternal_ReleaseWaiter(this);
	}

	inline uint32  GetOwnerId(void)   const { return m_nOwnerID; }
	inline int     GetDepth(void)     const { return m_nDepth; }
	inline int     GetAddend(void)    const { return m_lAddend; }
	inline HANDLE  GetSemaphore(void) const { return m_hSemaphore; }

private:
	volatile uint32_t m_nOwnerID;
	int               m_nDepth;
	volatile int      m_lAddend;
	HANDLE            m_hSemaphore;
};

///////////////////////////////////////////////////////////////////////////////
class VThreadTools : public IDetour
{
	virtual void GetAdr(void) const
	{
		spdlog::debug("| FUN: CThreadFastMutex::WaitForLock        : {:#18x} |\n", p_MutexInternal_WaitForLock.GetPtr());
		spdlog::debug("| FUN: CThreadFastMutex::ReleaseWaiter      : {:#18x} |\n", p_MutexInternal_ReleaseWaiter.GetPtr());
		spdlog::debug("| FUN: DeclareCurrentThreadIsMainThread     : {:#18x} |\n", p_DeclareCurrentThreadIsMainThread.GetPtr());
		spdlog::debug("| VAR: g_ThreadMainThreadID                 : {:#18x} |\n", reinterpret_cast<uintptr_t>(g_ThreadMainThreadID));
		spdlog::debug("| VAR: g_ThreadServerFrameThreadID          : {:#18x} |\n", reinterpret_cast<uintptr_t>(g_ThreadServerFrameThreadID));
		spdlog::debug("+----------------------------------------------------------------+\n");
	}
	virtual void GetFun(void) const
	{
		p_MutexInternal_WaitForLock   = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xD9\xFF\x15\x00\x00\x00\x00"), "xxxx?xxxx?xxxxxxxxxx????");
		p_MutexInternal_ReleaseWaiter = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x40\x53\x48\x83\xEC\x20\x8B\x41\x04\x48\x8B\xD9\x83\xE8\x01"), "xxxxxxxxxxxxxxx");
		p_DeclareCurrentThreadIsMainThread = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x83\xEC\x28\xFF\x15\x00\x00\x00\x00\x89\x05\x00\x00\x00\x00\x48\x83\xC4\x28"), "xxxxxx????xx????xxxx");

		v_MutexInternal_WaitForLock   = p_MutexInternal_WaitForLock.RCast<int (*)(CThreadFastMutex*)>();      /*48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC 20 48 8B D9 FF 15 ?? ?? ?? ??*/
		v_MutexInternal_ReleaseWaiter = p_MutexInternal_ReleaseWaiter.RCast<int (*)(CThreadFastMutex*)>();    /*40 53 48 83 EC 20 8B 41 04 48 8B D9 83 E8 01*/
		v_DeclareCurrentThreadIsMainThread = p_DeclareCurrentThreadIsMainThread.RCast<ThreadId_t(*)(void)>(); /*48 83 EC 28 FF 15 ?? ?? ?? ?? 89 05 ?? ?? ?? ?? 48 83 C4 28 */
	}
	virtual void GetVar(void) const
	{
		g_ThreadMainThreadID = p_DeclareCurrentThreadIsMainThread.FindPattern("89 05").ResolveRelativeAddressSelf(0x2, 0x6).RCast<ThreadId_t*>();
		g_ThreadServerFrameThreadID = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x83\x79\x00\x00\x75\x28\x8B"), "xx?xxxx")
			.FindPatternSelf("8B 05").ResolveRelativeAddressSelf(0x2, 0x6).RCast<ThreadId_t*>();
	}
	virtual void GetCon(void) const { }
	virtual void Attach(void) const { }
	virtual void Detach(void) const { }
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(VThreadTools);

#endif // THREADTOOLS_H
