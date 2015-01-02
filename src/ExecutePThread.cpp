///////////////////////////////////////////////////////////////////////////////
// Project:    Shell
// Course:     POS (Advanced Operating Systems)
// File:       ExecutePThread.cpp
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Source file which implements thread that executes commands 
//             parsed from the buffer.
///////////////////////////////////////////////////////////////////////////////

/**
 * @file ExecutePThread.cpp
 *
 * @brief Source file which implements thread that executes commands 
 *        parsed from the buffer.
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#include <iostream>
#include <algorithm>
#include <sstream>

#include <cstdio>

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <wordexp.h>

#include "RegExp.h"
#include "ExecutePThread.h"

using namespace std;
using namespace regexp;

/**
 * Parses whole command line.
 */
string ExecutePThread::REGEX_CMDLINE_TEST =
		"^[[:space:]]*([^&><\t\r\n ]+)(([[:space:]]+([^&><\t\r\n ]+))*)(([[:space:]]+([><])[[:space:]]*([^&><\t\r\n ]+))*)[[:space:]]*([[:space:]]+(&))?[[:space:]]*$";

/**
 * Parses argument segment only.
 */
string ExecutePThread::REGEX_ARG_PARSE = "^(([[:space:]]+([^&><\t\r\n ]+))+)$";

/**
 * Parses redirect segment only.
 */
string ExecutePThread::REGEX_REDIRECT_PARSE =
		"^(([[:space:]]+([><])[[:space:]]*([^&><\t\r\n ]+))+)$";

/**
 * Matches white spaces.
 */
string ExecutePThread::REGEX_SPACES = "^[[:space:]]+$";

/**
 * Matches exit command.
 */
string ExecutePThread::REGEX_EXIT = "^[[:space:]]*exit[[:space:]]*$";

int ExecutePThread::stdin_dup = -1; /**< clonned FD of the STDIN */
int ExecutePThread::stdout_dup = -1; /**< clonned FD of the STDOUT */
int ExecutePThread::stderr_dup = -1; /**< clonned FD of the STDERR */
int ExecutePThread::devnull_fd = -1; /**< clonned FD of the /dev/null */
bool ExecutePThread::child_exited = false; /**< Determines whether child currently exited */

/**
 * Cancels execute thread.
 */
void ExecutePThread::cancel() {
	bufferMonitor.signal();
	PThread::cancel();
}

/**
 * Starts new process and calls its handler function.
 * 
 * @param label Name of the process.
 * @param processHandler Handler function of the process 
 * @return Return code from the process. 
 */
int ExecutePThread::startProcess(int(*processHandler)(void *arg), void *arg) {
	errno = 0;
	int pid = fork();
	int retError = errno;

	if (pid == 0) { // Created - child process
		exit((*processHandler)(arg)); // Call handler function of th eprocess
	} else if (pid == -1) { // An error
		perror("Failed to create a new process - fork()");
		return (retError != 0) ? -errno : -EXIT_FAILURE;
	}

	return pid;
}

/**
 * Callback function which is called when this thread is going to start.
 */
void ExecutePThread::onStart() {
	stdin_dup = dup(STDIN_FILENO);
	stdout_dup = dup(STDOUT_FILENO);
	//stderr_dup = dup(STDERR_FILENO);
	devnull_fd = open("/dev/null", O_RDWR);

	struct sigaction sa;
	sa.sa_handler = child_exited_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGCHLD);
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("Failed - sigaction(SIGCHLD)");
	}
}

/**
 * Callback function which is called when this thread is going to finish.
 */
void ExecutePThread::onFinish() {
	close(stdin_dup);
	close(stdout_dup);
	close(stderr_dup);
	close(devnull_fd);
}

/**
 * Serves SIGCHLG signals.
 * @param signo Number of signal which entranced into this handler.
 */
void ExecutePThread::child_exited_handler(int signo) {
	signo = signo;
	pid_t pid;
	int status;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		child_exited = true;
	}
}

/**
 * Parses redirect records into information structure from the array of matches.
 * @param cmdInfo Information about parsed line - will be filled.
 * @param matches Array of matches.
 */
void ExecutePThread::parseRedirects(CommandInfo &cmdInfo,
		const vector<string> &matches) {
	RegExp cmdExpr(REGEX_REDIRECT_PARSE, REG_EXTENDED);
	vector < string > redirectsMatches;
	string redirs = matches[5];

	while (!(redirectsMatches = cmdExpr.exec(redirs)).empty()) {
		RedirectInfo redirInfo;
		redirInfo.fileName = redirectsMatches[4];
		redirInfo.isOut = !redirectsMatches[3].compare(">");
		cmdInfo.redirects.push_back(redirInfo);
		redirs.erase(redirs.size() - redirectsMatches[2].size(),
				redirectsMatches[2].size());
	}

	reverse(cmdInfo.redirects.begin(), cmdInfo.redirects.end());
}

/**
 * Parses arguments into information structure from the array of matches.
 * @param cmdInfo Information about parsed line - will be filled.
 * @param matches Array of matches.
 */
void ExecutePThread::parseArguments(CommandInfo &cmdInfo,
		const vector<string> &matches) {
	RegExp cmdExpr(REGEX_ARG_PARSE, REG_EXTENDED);
	vector < string > argsMatches;
	string args = matches[2];

	while (!(argsMatches = cmdExpr.exec(args)).empty()) {
		cmdInfo.arguments.push_back(argsMatches[3]);
		args.erase(args.size() - argsMatches[2].size(), argsMatches[2].size());
	}

	reverse(cmdInfo.arguments.begin(), cmdInfo.arguments.end());
}

/**
 * Parses program name, argumetns & files where to redirect stdin & stdout if are specified.
 * @param cmdInfo Information about parsed line - will be filled.
 * @param matches Array of matches.
 */
void ExecutePThread::processCommand(CommandInfo &cmdInfo,
		const vector<string> &matches) {
	// Program name
	cmdInfo.programName = matches[1];
	cmdInfo.programNameArgs = cmdInfo.programName;

	// Getting arguments 
	if (!matches[2].empty()) {
		parseArguments(cmdInfo, matches);
		cmdInfo.programNameArgs += matches[2];
	}

	// Getting redirects 
	if (!matches[5].empty()) {
		parseRedirects(cmdInfo, matches);
	}

	// Run command on background
	cmdInfo.runOnBackground = !matches[10].empty();
}

/**
 * Executes comand which is retrieved from the cmdInfo.
 * 
 * @param cmdInfo Information about parsed line.
 * @return True on success, false on failure.
 */
bool ExecutePThread::executeCommand(CommandInfo &cmdInfo) {
	child_exited = false;
	int cmdPID = startProcess(executionHandler, &cmdInfo);

	/* Close stdin/stdout because it is not demanded to 
	 * recieve data from the child process */

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	//close(STDERR_FILENO);

	if (cmdPID > 0) {
		if (!cmdInfo.runOnBackground) {
			sigset_t newmask, oldmask;
			sigemptyset(&newmask);
			sigaddset(&newmask, SIGCHLD);

			pthread_sigmask(SIG_BLOCK, &newmask, &oldmask);

			/* Wait until child has exited. */
			while (!child_exited) {
				sigsuspend(&oldmask);
			}

			pthread_sigmask(SIG_UNBLOCK, &newmask, NULL);
		} else {

		}
	}

	/* Restore std file descriptors */

	dup2(stdin_dup, STDIN_FILENO);
	dup2(stdout_dup, STDOUT_FILENO);
	//dup2(stderr_dup, STDERR_FILENO);

	return true;

}

/**
 * Redirects stdin and stdout.
 * @param cmdInfo Information about parsed line.
 * @param outRedirected Is set whether some stdout redirection was proceeded.
 * @param inRedirected Is set whether some stdin redirection was proceeded.
 * @return Code which signals sucess or failure. 0 is returned on success.
 */
int ExecutePThread::redirectStdInOut(CommandInfo &cmdInfo, bool &outRedirected,
		bool &inRedirected) {
	int retError = 0;
	errno = 0;

	outRedirected = false;
	inRedirected = false;

	int ret = EXIT_SUCCESS;
	for (vector<RedirectInfo>::iterator it = cmdInfo.redirects.begin();
			it != cmdInfo.redirects.end(); it++) {
		RedirectInfo &info = *it;

		int redir_file, file_no;

		if (info.isOut) {
			outRedirected = true;
			file_no = STDOUT_FILENO;
			if ((redir_file = open(&info.fileName[0], O_CREAT | O_WRONLY,
					S_IRUSR | S_IRGRP | S_IROTH)) < 0) {
				retError = errno;
				perror(
						"Failed to open/create file to which should be stdout redirected - open()");
				ret = (retError != 0) ? errno : EXIT_FAILURE;
				break;
			}
		} else {
			inRedirected = true;
			file_no = STDIN_FILENO;
			if ((redir_file = open(&info.fileName[0], O_RDONLY)) < 0) {
				retError = errno;
				perror(
						"Failed to open file from which should be stdin taken - open()");
				ret = (retError != 0) ? errno : EXIT_FAILURE;
				break;
			}
		}

		cout << flush;

		if (dup2(redir_file, file_no) < 0) {
			retError = errno;
			perror(
					"Failed to establish redirecting of stdin or stdout - dup2()");
			close(redir_file);
			ret = (retError != 0) ? errno : EXIT_FAILURE;
			break;
		}
		close(redir_file);

	}

	return ret;
}

/**
 * Redirects passed file descriptor to /dev/null.
 * @param fd File descriptor which should be redirected do /dev/null.
 * @return 0 on success, othewise error code
 */
int ExecutePThread::detachFD(int fd) {
	int retError = 0;
	errno = 0;

	if (dup2(devnull_fd, fd) < 0) {
		retError = errno;
		stringstream ss;
		ss << "Failed to detach file descriptor " << fd
				<< " for background process to /dev/null  - dup2()";
		string msg = ss.str();
		perror(msg.c_str());
		return (retError != 0) ? errno : EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Handle function for new forked process.
 * Executes parsed command from command line.
 * @param arg Informations about parsed line.
 * @return Exit code of the new process.
 */
int ExecutePThread::executionHandler(void *arg) {
	CommandInfo &cmdInfo = *static_cast<CommandInfo *>(arg);

	int retError = 0;
	errno = 0;

	bool outRedirected, inRedirected;

	/* Set up redirections */

	if ((retError = redirectStdInOut(cmdInfo, outRedirected, inRedirected))
			!= EXIT_SUCCESS) {
		return retError;
	}

	/* If running of command on background is demanded then redirect stdin/stdout/stderr to /dev/null
	 * Shell does not recieves any feedback from the process on the background 
	 */
	if (cmdInfo.runOnBackground) {

		if (!inRedirected
				&& ((retError = detachFD(STDIN_FILENO)) != EXIT_SUCCESS)) {
			return retError;
		}

		/*if ((retError = detachFD(STDERR_FILENO)) != EXIT_SUCCESS) {
		 return retError; 
		 }*/

		if (!outRedirected
				&& ((retError = detachFD(STDOUT_FILENO)) != EXIT_SUCCESS)) {
			return retError;
		}
	}

	/* Setup behaviour on SIGINT command */

	struct sigaction sa;
	sa.sa_flags = (cmdInfo.runOnBackground) ? 0 : SA_RESTART | SA_SIGINFO;
	sa.sa_handler = (cmdInfo.runOnBackground) ? SIG_IGN : SIG_DFL;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGINT);

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		retError = errno;
		perror("Failed - sigaction(SIGINT)");
		return (retError != 0) ? errno : EXIT_FAILURE;
	}

	/*vector<char *> argv(cmdInfo.arguments.size() + 2);

	 argv[0] = &cmdInfo.programName[0];
	 int i = 1;
	 for (vector<string>::iterator it = cmdInfo.arguments.begin(); it != cmdInfo.arguments.end(); it++) {
	 argv[i++] = &(*it)[0];
	 }
	 argv[argv.size() - 1] = NULL;          */

	/* Executes command with the argumetns parsed by wordexp */

	wordexp_t res;
	if (wordexp(cmdInfo.programNameArgs.c_str(), &res, 0) != 0) {
		return EXIT_FAILURE;
	}
	execvp(res.we_wordv[0], res.we_wordv);

	/*execvp(&cmdInfo.programName[0], &argv[0]);*/
	retError = errno;

	/*if (dup2(stderr_dup, STDERR_FILENO) < 0) {
	 retError = errno;
	 perror("Failed execv() and to restore stderr for background process - dup2()");
	 return (retError != 0)? errno : EXIT_FAILURE;  
	 }*/

	perror("Failed - execv()");
	return (retError != 0) ? errno : EXIT_FAILURE;

}

/**
 * Main function where execute thread runs.
 * @return Exit code of this thread.
 */
int ExecutePThread::run() {

	while (1) {
		cout << "$ " << flush;

		cancelPoint();
		bufferMonitor.enter();

		/* Reading command from stdin */
		while (buffer[0] == '\0') {
			cancelPoint();
			bufferMonitor.wait();
		}

		string command(&buffer[0]);
		buffer[0] = '\0';
		bufferMonitor.signal();
		bufferMonitor.exit();

		/* Checking command syntax */
		RegExp spacesExpr(REGEX_SPACES, REG_EXTENDED);
		RegExp exitExpr(REGEX_EXIT, REG_EXTENDED);

		if (spacesExpr.test(command)) {
			continue;
		} else if (exitExpr.test(command)) {
			break;
		}

		RegExp cmdExpr(REGEX_CMDLINE_TEST, REG_EXTENDED);
		vector < string > matches;

		matches = cmdExpr.exec(command);
		if (matches.empty()) {
			cerr << "Typed invalid command!" << endl;
			continue;
		}

		/* Getting command - parsing it */

		CommandInfo cmdInfo;
		processCommand(cmdInfo, matches);

		/* Executing command */
		executeCommand(cmdInfo);
	}

	return 0;
}

