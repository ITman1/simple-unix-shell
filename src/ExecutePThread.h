///////////////////////////////////////////////////////////////////////////////
// Project:    Shell
// Course:     POS (Advanced Operating Systems)
// File:       ExecutePThread.h
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Header file which defines the thread which executes commands 
//             from the buffer. 
///////////////////////////////////////////////////////////////////////////////

/**
 * @file ExecutePThread.h
 *
 * @brief Header file which defines the thread which executes commands from the buffer. 
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#ifndef EXECUTEPTHREAD_H_INCLUDED
#define EXECUTEPTHREAD_H_INCLUDED

#include <vector>
#include <string>

#include "PThread.h"

using namespace std;

/**
 * Thread class which executes commands fromt he buffer shared with read thread.
 */
class ExecutePThread: public PThread {
public:
	ExecutePThread(vector<char> &buffer, PThreadMonitor &bufferMonitor) :
			buffer(buffer), bufferMonitor(bufferMonitor) {
	}
	virtual ~ExecutePThread() {
	}
	virtual int run();
	virtual void cancel();
private:

	/**
	 * Keeps informations about redirect files.
	 */
	typedef struct {
		bool isOut;
		string fileName;
	} RedirectInfo;

	/**
	 * Structure which hold informations about parsed command line.
	 */
	typedef struct {
		string programName;
		string programNameArgs;
		vector<string> arguments;
		vector<RedirectInfo> redirects;
		bool runOnBackground;
	} CommandInfo;

	static string REGEX_CMDLINE_TEST;
	static string REGEX_ARG_PARSE;
	static string REGEX_REDIRECT_PARSE;
	static string REGEX_SPACES;
	static string REGEX_EXIT;

	vector<char> &buffer;
	PThreadMonitor &bufferMonitor;

	static bool child_exited;
	static int stdin_dup;
	static int stdout_dup;
	static int stderr_dup;
	static int devnull_fd;

	void parseRedirects(CommandInfo &cmdInfo, const vector<string> &matches);
	void parseArguments(CommandInfo &cmdInfo, const vector<string> &matches);
	void processCommand(CommandInfo &cmdInfo, const vector<string> &matches);

	bool executeCommand(CommandInfo &cmdInfo);
	int startProcess(int(*processHandler)(void *arg), void *arg);

	void onStart();
	void onFinish();

	static int executionHandler(void *arg);
	static int redirectStdInOut(CommandInfo &cmdInfo, bool &outRedirected,
			bool &inRedirected);
	static int detachFD(int fd);
	static void child_exited_handler(int signo);
};

#endif // EXECUTEPTHREAD_H_INCLUDED
