///////////////////////////////////////////////////////////////////////////////
// File:       UniqueIDGenerator.h
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Header to simple generator that ensures always unique numbers 
//             to be generated.
///////////////////////////////////////////////////////////////////////////////

/**
 * @file UniqueIDGenerator.h
 *
 * @brief Header to simple generator that ensures always unique numbers
 *        to be generated. 
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#ifndef UNIQUEIDGENERATOR_H_INCLUDED
#define UNIQUEIDGENERATOR_H_INCLUDED

#include <pthread.h>

using namespace std;

/**
 * Class implementing unique number generator.
 */
class UniqueIDGenerator {
public:
	UniqueIDGenerator();
	virtual ~UniqueIDGenerator();

	int generate();
private:
	volatile int id;
	pthread_mutex_t idMutex;
};

#endif // UNIQUEIDGENERATOR_H_INCLUDED
