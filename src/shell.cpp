///////////////////////////////////////////////////////////////////////////////
// Project:    Shell
// Course:     POS (Advanced Operating Systems)
// File:       shell.cpp
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Represents main executable application - runs simple shell service.
///////////////////////////////////////////////////////////////////////////////

/**
 * @file shell.cpp
 *
 * @brief Runs service of a simple shell.
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include <cstdio>
#include <signal.h>

using namespace std;

#include "ShellService.h"

/**
 * Instance of a service of shell.
 */
ShellService &shell = ShellService::getInstance();

/*
 * Service ended, exit application.
 * @param code Exit code of the application.
 */
void shellServiceFinished(int code) {
	exit(code);
}

/**
 * Signal handler which cleans up application and exists.
 * @param signo Number of signal which was delivered to this handler.
 */
void shell_exit(int signo) {
	signo = signo;
	shell.stop();
}

int main(int argc, char *argv[]) {
	argc = argc;
	argv = argv;

	/* Properly handle SIGTERM and SIGQUIT signals. */
	struct sigaction sa;
	sa.sa_handler = shell_exit;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGQUIT);
	sigaddset(&sa.sa_mask, SIGTERM);

	if (sigaction(SIGQUIT, &sa, NULL) == -1) {
		perror("Failed - sigaction(SIGQUIT)");
		return EXIT_FAILURE;
	}

	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		perror("Failed - sigaction(SIGTERM)");
		return EXIT_FAILURE;
	}

	/* Ignore SIGINT */

	sa.sa_flags = 0;
	sa.sa_handler = SIG_IGN;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGINT);

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("Failed - sigaction(SIGINT)");
		return EXIT_FAILURE;
	}
	//pthread_sigmask(SIG_BLOCK, &sa.sa_mask, NULL);

	/* Run shell service */

	shell.addOnFinishCallback(shellServiceFinished);
	if (shell.start()) {
		while (1)
			pause();
		return EXIT_SUCCESS;
	} else {
		cerr << "Unable to start shell service!" << endl;
		return EXIT_FAILURE;
	}

}

