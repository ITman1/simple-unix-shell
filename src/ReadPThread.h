///////////////////////////////////////////////////////////////////////////////
// Project:    Shell
// Course:     POS (Advanced Operating Systems)
// File:       ReadPThread.h
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Header file which defines the thread which reads from the stdin 
///////////////////////////////////////////////////////////////////////////////

/**
 * @file ReadPThread.h
 *
 * @brief Header file which defines the thread which reads from the stdin 
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#ifndef READPTHREAD_H_INCLUDED
#define READPTHREAD_H_INCLUDED

#include <vector>

#include "PThread.h"

using namespace std;

/**
 * Class of the thread which reads text from the stdin & cooperates with consumer.
 */
class ReadPThread: public PThread {
public:
	ReadPThread(vector<char> &buffer, PThreadMonitor &bufferMonitor) :
			buffer(buffer), bufferMonitor(bufferMonitor) {
	}
	virtual ~ReadPThread() {
	}
	virtual int run();
	virtual void cancel();
	void startReading();
private:
	vector<char> &buffer;
	PThreadMonitor &bufferMonitor;
	int exitFile;
	char exitFileName[32];

};

#endif // READPTHREAD_H_INCLUDED
