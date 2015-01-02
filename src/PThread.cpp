///////////////////////////////////////////////////////////////////////////////
// File:       PThread.cpp
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Implements wrapper class above pthread library.
///////////////////////////////////////////////////////////////////////////////

/**
 * @file PThread.cpp
 *
 * @brief Implements wrapper class above pthread library.
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#include "PThread.h"

using namespace std;

/**
 * Static generator for unique numbers of threads.
 */
UniqueIDGenerator PThread::idGenerator;

/**
 * Constructor.
 */
PThread::PThread() :
		threadInitialized(false), threadRunning(false), retCode(0) {
	id = idGenerator.generate() + 1;

	if (pthread_create(&thread, NULL, &threadInitPrivate,
			reinterpret_cast<void *>(this)) != 0) {
		throw PThreadCreate();
	}
}

/**
 * Destructor.
 */
PThread::~PThread() {
	cancel();
	pthread_detach(thread);
}

/**
 * Method which is called for initialization of the new thread.
 * @param _obj Object of the PThread which belongs this thread.
 */
void *PThread::threadInitPrivate(void *_obj) {
	PThread *obj = reinterpret_cast<PThread *>(_obj);

	/* Setting up finalization callback & disable assynchronous cancellation. */

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	pthread_cleanup_push(obj->onFinishPrivate, _obj);

	/* Now we are going to wait until start() is called. */

	obj->initMonitor.enter();
	obj->threadInitialized = true;
	obj->initMonitor.signal();
	obj->initMonitor.exit();

	obj->runningMonitor.enter();
	while (!obj->threadRunning) { // Cycle until start() is not called
		obj->runningMonitor.wait();
	}
	obj->runningMonitor.exit();

	/* Start running of the code of this thread. */

	obj->onStart();
	obj->retCode = obj->run();

	/* Running ended, finalize this thread. */

	onFinishPrivate(obj);
	pthread_cleanup_pop(0);
	pthread_exit(NULL);
}

/**
 * Starts running of this thread.
 */
void PThread::start() {
	initMonitor.enter();
	if (!threadInitialized) {
		initMonitor.wait();
	}
	initMonitor.exit();

	runningMonitor.enter();
	threadRunning = true;
	runningMonitor.signal();
	runningMonitor.exit();
}

/**
 * Waits until thread is finished.
 */
void PThread::join() {
	pthread_join(thread, NULL);
}

/**
 * Sends cancellation demand to this thread.
 * This request will be served on the next cancelation point.
 */
void PThread::cancel() {
	int _running;

	runningMonitor.enter();
	_running = threadRunning;
	runningMonitor.exit();

	if (_running) {
		pthread_cancel(thread);
		join();
	}
}

/**
 * Returns thread structure to this thread.
 * @return Thread structure to this thread.
 */
pthread_t PThread::getThread() {
	return thread;
}

/**
 * Returns ID of this thread.
 * @return ID of this thread.
 */
int PThread::getID() {
	return id;
}

/**
 * Retrieves code by which has been canceled this thread.
 * @return Return code of the termination of this thread.
 */
int PThread::getRetCode() {
	return retCode;
}

/**
 * Test whether has not been demanded cancellation of this thread.
 */
void PThread::cancelPoint() {
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_testcancel();
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
}

/**
 * Fires callback functions which signals the end of this threads.
 */
void PThread::fireFinishCallbacks() {
	set<OnFinishCallback>::iterator clbk_iter;
	for (clbk_iter = onFinishCallbacks.begin();
			clbk_iter != onFinishCallbacks.end(); ++clbk_iter) {
		(**clbk_iter)();
	}
}

/**
 * Adds new callback for signaling finish of this thread.
 */
void PThread::addOnFinishCallback(OnFinishCallback callback) {
	onFinishCallbacks.insert(callback);
}

/**
 * Removes callback for signaling finish of this thread.
 */
void PThread::removeOnFinishCallback(OnFinishCallback callback) {
	onFinishCallbacks.erase(callback);
}

/**
 * Method is called for finalization of thread's run.
 */
void PThread::onFinishPrivate(void *_obj) {
	PThread *obj = reinterpret_cast<PThread *>(_obj);

	obj->runningMonitor.enter();
	obj->threadRunning = false;
	obj->runningMonitor.exit();

	obj->initMonitor.exit();

	obj->onFinish();
	obj->fireFinishCallbacks();
}

/**
 * Constructor of the new monitor.
 */
PThreadMonitor::PThreadMonitor() {
	pthread_mutex_init(&monMutex, NULL);
	pthread_cond_init(&monCond, NULL);
}

/**
 * Destructor of the monitor.
 */
PThreadMonitor::~PThreadMonitor() {
	pthread_mutex_destroy(&monMutex);
	pthread_cond_destroy(&monCond);
}

/**
 * Entering into critical section.
 */
void PThreadMonitor::enter() {
	pthread_mutex_lock(&monMutex);
}

/**
 * Exiting the critic section.
 */
void PThreadMonitor::exit() {
	pthread_mutex_unlock(&monMutex);
}

/**
 * Signaling all waiting threads.
 */
void PThreadMonitor::signal() {
	pthread_cond_broadcast(&monCond);
}

/**
 * Waiting for signal.
 */
void PThreadMonitor::wait() {
	pthread_cond_wait(&monCond, &monMutex);
}

