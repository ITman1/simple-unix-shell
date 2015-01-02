///////////////////////////////////////////////////////////////////////////////
// Project:    Shell
// Course:     POS (Advanced Operating Systems)
// File:       ReadPThread.cpp
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Implements thread which reads input from stdin and stores it 
//             into buffer. 
///////////////////////////////////////////////////////////////////////////////

/**
 * @file ReadPThread.cpp
 *
 * @brief Implements thread which reads input from stdin and stores it into buffer.
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <iostream>

#include <signal.h>
#include <errno.h>    

#include "ReadPThread.h"

using namespace std;

/**
 * Cancels this thread.
 */
void ReadPThread::cancel() {
	bufferMonitor.signal();
	PThread::cancel();
}

/**
 * Starts reading from the stdin.
 */
void ReadPThread::startReading() {
	start();
}

/**
 * Main body of read thread.
 * @return Exit code how exited/returned this thread.
 */
int ReadPThread::run() {
	bool lastCommandFinished = true;
	// Set stdin into file set - only stdin we will control and read
	fd_set read_set;
	FD_ZERO(&read_set);
	FD_SET(STDIN_FILENO, &read_set);

	// Block until new data are available
	int ret = EXIT_SUCCESS;
	while (1) {
		int retError = 0;
		errno = 0;

		while (1) { // Wait for data to read or cancel request to break this thread.
			cancelPoint();
			struct timeval tv = { 1, 0 };
			ret = select(STDIN_FILENO + 1, &read_set, NULL, NULL, &tv);
			cancelPoint();

			if (ret != -1) {
				break;
			}
		}

		if (FD_ISSET(STDIN_FILENO, &read_set)) { // New data on stdin

			bufferMonitor.enter();

			/* Buffer is not empty, consumer has not processed it yet. */
			while (buffer[0] != '\0') {
				cancelPoint();
				bufferMonitor.wait(); // Wait & enable buffer to consumer
			}

			errno = 0;
			ssize_t readBytes = read(STDIN_FILENO, &buffer[0],
					sizeof(char) * buffer.size());

			if (readBytes < 0) { // exit with error
				bufferMonitor.exit();
				retError = errno;
				perror("Failed reading of stdin - read()");
				ret = (retError != 0) ? errno : EXIT_FAILURE;
				break;
			} else if (readBytes == 0 || buffer[0] == '\0') { // Nothing has been read
				buffer[0] = '\0';
				bufferMonitor.exit();
				continue;
			} else if (readBytes > 0 && readBytes < (int) buffer.size()) { // Normal reading, apend \0
				buffer[readBytes] = '\0';
			}

			// Buffer overflow, print error
			if (readBytes >= (int) buffer.size() && lastCommandFinished) {
				cerr << "Too long input command!" << endl;
				buffer[0] = '\0';
			}
			// Input text in limits, everything OK, signal the consumer
			else if (readBytes > 0 && readBytes < (int) buffer.size()
					&& lastCommandFinished) {
				bufferMonitor.signal();
			}
			// There was probably some error, do not put data to consumer and delete the data from buffer
			else {
				buffer[0] = '\0';
			}

			bool prevLastCommandFinished = lastCommandFinished;
			lastCommandFinished = buffer[readBytes - 1] == '\n';

			/* After error is printed, send to consumer empty line - this will cause printing the $ */
			if (!prevLastCommandFinished && lastCommandFinished && readBytes > 0
					&& readBytes < (int) buffer.size()) {
				buffer[0] = ' ';
				buffer[1] = '\0';
				bufferMonitor.signal();
			}

			bufferMonitor.exit();
		}

		// Reset for sure - but with one set file descriptor is not necessary
		FD_ZERO(&read_set);
		FD_SET(STDIN_FILENO, &read_set);
	}

	return ret;
}
