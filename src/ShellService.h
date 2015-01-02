///////////////////////////////////////////////////////////////////////////////
// Project:    Shell
// Course:     POS (Advanced Operating Systems)
// File:       ShellService.h
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Header file to service which takes control over simple shell.
///////////////////////////////////////////////////////////////////////////////

/**
 * @file ShellService.h
 *
 * @brief Header file to service which takes contrl over simple shell.
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#ifndef SHELLSERVICE_H_INCLUDED
#define SHELLSERVICE_H_INCLUDED

#include <set>

#include "ReadPThread.h"
#include "ExecutePThread.h"

using namespace std;

/*
 * Singleton class which starts or stops service of the shell.
 */
class ShellService {
public:
	typedef void(*OnFinishCallback)(int);

	static ShellService &getInstance();

	bool start();
	void stop();

	void addOnFinishCallback(OnFinishCallback callback);
	void removeOnFinishCallback(OnFinishCallback callback);

private:
	ShellService();
	static ShellService instance;

	static const int BUFFER_SIZE = 513;

	bool initFailed;
	vector<char> buffer;
	PThreadMonitor bufferMonitor;
	ReadPThread readThread;
	ExecutePThread executeThread;
	set<OnFinishCallback> onFinishCallbacks;
	bool finished;
	sigset_t orig_sigmask;

	void fireFinishCallbacks(int code);

	static void readThreadFinished();
	static void executeThreadFinished();
};

#endif // SHELLSERVICE_H_INCLUDED
