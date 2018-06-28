﻿#ifndef EYEGLE_COMMON_SPIN_LOCK_H_  
#define EYEGLE_COMMON_SPIN_LOCK_H_  

#include <assert.h>  

#if defined(WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>  
#else  
#include <pthread.h>  
#endif  

#include "common/Global.h"



//不允许重复锁.
class COMMON_API SpinLock  
{  
#if defined(WIN32)  
	CRITICAL_SECTION m_critical_section;  
#else  
	pthread_spinlock_t m_spinlock;  
#endif  

public:  
	SpinLock(int spin_count = 10000);

	~SpinLock(); 

#if defined(WIN32)  
	inline CRITICAL_SECTION* InnerMutex() { return &m_critical_section; }  
#else  
	inline pthread_spinlock_t* InnerMutex() { return &m_spinlock; }  
#endif  

	inline void Lock();

	inline bool Trylock();

	inline void Unlock(); 
};

inline void SpinLock::Lock()  
{  
#if defined(WIN32)  
	EnterCriticalSection(&m_critical_section);  
	//assert(m_critical_section.RecursionCount == 1);
#else  
	int rs = ::pthread_spin_lock(&m_spinlock);  
	assert(0 == rs);  
#endif  
}  

inline bool SpinLock::Trylock() 
{  
#if defined(WIN32)  
	if(!TryEnterCriticalSection(&m_critical_section)) { return false; }

	if(m_critical_section.RecursionCount > 1) {
		LeaveCriticalSection(&m_critical_section);
		throw ("Thread locked expcetion! LeaveCriticalSection");
	}
	return true;
#else  
	return 0 == ::pthread_spin_trylock(&m_spinlock);  
#endif  
}  

inline void SpinLock::Unlock()  
{  
#if defined(WIN32)  
	//assert(m_critical_section.RecursionCount == 1);
	LeaveCriticalSection(&m_critical_section);  
#else  
	int rs = ::pthread_spin_unlock(&m_spinlock);  
	assert(0 == rs);  
#endif  
}  

class Locker {
public:
	Locker(SpinLock* spin_lock) : m_spin_lock(spin_lock) {
		m_spin_lock->Lock();
	}

	~Locker() {
		m_spin_lock->Unlock();
	}

private:
	SpinLock* m_spin_lock;
};



#endif