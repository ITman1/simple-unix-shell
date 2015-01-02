///////////////////////////////////////////////////////////////////////////////
// Project:    Shell
// Course:     POS (Advanced Operating Systems)
// File:       ShellService.cpp
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Source file which implements contrl over simple shell.
///////////////////////////////////////////////////////////////////////////////

/**
 * @file ShellService.cpp
 *
 * @brief Source file which implements contrl over simple shell.
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#include <cstdio>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "ShellService.h"

ShellService ShellService::instance;

/**
 * Constructor.
 */
ShellService::ShellService() :
		initFailed(false), buffer(vector<char>(BUFFER_SIZE)), bufferMonitor(
				PThreadMonitor()), readThread(
				ReadPThread(buffer, bufferMonitor)), executeThread(
				ExecutePThread(buffer, bufferMonitor)) {

}

/**
 * Callback function which captures end of the read thread.
 */
void ShellService::readThreadFinished() {
	instance.executeThread.cancel();
	instance.fireFinishCallbacks(instance.readThread.getRetCode());
}

/**
 * Callback function which captures end of the execute thread.
 */
void ShellService::executeThreadFinished() {
	instance.readThread.cancel();
	instance.fireFinishCallbacks(instance.executeThread.getRetCode());
}

/**
 * Fires callbacks to signal exit of the shell service.
 * @param code Exit code of the service.
 */
void ShellService::fireFinishCallbacks(int code) {
	set<OnFinishCallback>::iterator clbk_iter;
	for (clbk_iter = onFinishCallbacks.begin();
			clbk_iter != onFinishCallbacks.end(); ++clbk_iter) {
		(**clbk_iter)(code);
	}
}

/**
 * Registers new finish callback function.
 * @param callback Function to be registered.
 */
void ShellService::addOnFinishCallback(OnFinishCallback callback) {
	onFinishCallbacks.insert(callback);
}

/**
 * Unregisters finish callback function.
 * @param callback Function to be unregistered.
 */
void ShellService::removeOnFinishCallback(OnFinishCallback callback) {
	onFinishCallbacks.erase(callback);
}

/**
 * Returns singleton instance of this service.
 * @return Instance of this service object.
 */
ShellService &ShellService::getInstance() {
	return instance;
}

/**
 * Starts shell service.
 * @return True on succes, otherwise false.
 */
bool ShellService::start() {
	if (!initFailed) {
		//  stop();

		/*readThread = ReadPThread(buffer, bufferMonitor),
		 executeThread = ExecutePThread(buffer, bufferMonitor);    */

		readThread.addOnFinishCallback(readThreadFinished);
		executeThread.addOnFinishCallback(executeThreadFinished);

		executeThread.start();
		readThread.startReading();
	}

	return !initFailed;
}

/**
 * Stops shell service.
 */
void ShellService::stop() {
	executeThread.cancel();
	readThread.cancel();
}
