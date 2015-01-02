///////////////////////////////////////////////////////////////////////////////
// File:       PThread.h
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Defines class of the wrapper above pthread library.
///////////////////////////////////////////////////////////////////////////////

/**
 * @file PThread.h
 *
 * @brief Defines class of the wrapper above pthread library.
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#ifndef PTHREAD_H_INCLUDED
#define PTHREAD_H_INCLUDED

#include <pthread.h>

#include <stdexcept>
#include <set>

#include "UniqueIDGenerator.h"

using namespace std;

class PThreadCreate: public runtime_error {
public:
	PThreadCreate() :
			runtime_error("Error during creating a new thread!") {
	}
};

class PThreadMonitor {
public:
	PThreadMonitor();
	~PThreadMonitor();
	void enter();
	void exit();
	void signal();
	void wait();
private:
	pthread_mutex_t monMutex;
	pthread_cond_t monCond;
};

class PThread {
public:
	typedef void(*OnFinishCallback)();

	PThread();
	virtual ~PThread();

	virtual void start();
	virtual void cancel();
	pthread_t getThread();
	int getID();
	int getRetCode();
	void join();
	void addOnFinishCallback(OnFinishCallback callback);
	void removeOnFinishCallback(OnFinishCallback callback);

	virtual int run() = 0;

protected:
	virtual void onStart() {
	}
	virtual void onFinish() {
	}

	void cancelPoint();

	volatile bool threadInitialized;
	volatile bool threadRunning;
	volatile int id;
	int retCode;
	pthread_t thread;

	PThreadMonitor initMonitor;
	PThreadMonitor runningMonitor;

	set<OnFinishCallback> onFinishCallbacks;

private:
	static UniqueIDGenerator idGenerator;

	static void *threadInitPrivate(void *thread);
	static void onFinishPrivate(void *thread);

	void fireFinishCallbacks();
};

#endif // PTHREAD_H_INCLUDED
