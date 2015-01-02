///////////////////////////////////////////////////////////////////////////////
// File:       UniqueIDGenerator.cpp
// Date:       February 2013
// Author:     Radim Loskot
// E-mail:     xlosko01(at)stud.fit.vutbr.cz
//
// Brief:      Implemnets simple generator that ensures always unique numbers 
//             to be generated.
///////////////////////////////////////////////////////////////////////////////

/**
 * @file UniqueIDGenerator.cpp
 *
 * @brief Implemnets simple generator that ensures always unique numbers
 *        to be generated. 
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#include "UniqueIDGenerator.h"

UniqueIDGenerator::UniqueIDGenerator() :
		id(0) {
	pthread_mutex_init(&idMutex, NULL);
}

UniqueIDGenerator::~UniqueIDGenerator() {
	pthread_mutex_destroy(&idMutex);
}

/**
 * Generates unique number.
 * 
 * @return Unique number. 
 */
int UniqueIDGenerator::generate() {
	pthread_mutex_lock(&idMutex);
	int generatedID = id++;
	pthread_mutex_unlock(&idMutex);
	return generatedID;
}
