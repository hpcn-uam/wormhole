/* Copyright (c) 2015-2018 Rafael Leira
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "es_hpcn_wormhole_Einstein.h"

#include <einstein/einstein.hpp>
using namespace einstein;

Einstein *_JWH_einstein;

/*
 * Class:     es_hpcn_wormhole_Einstein
 * Method:    init
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_es_hpcn_wormhole_Einstein_init(JNIEnv *env,
                                                           jobject obj,
                                                           jstring configFileName,
                                                           jstring listenIp,
                                                           jint listenPort,
                                                           jboolean autoDeployWorms,
                                                           jobjectArray RunParams)
{
	UNUSED(obj);
	UNUSED(RunParams);  // TODO remove this parameter

	const char *c_configFileName = env->GetStringUTFChars(configFileName, NULL);
	const char *c_listenIp       = env->GetStringUTFChars(listenIp, NULL);

	vector<string> runparams;

	try {
		_JWH_einstein = new Einstein(c_configFileName, c_listenIp, listenPort, autoDeployWorms == JNI_TRUE);
		_JWH_einstein->openHoles();

	} catch (std::runtime_error) {
		delete _JWH_einstein;
	}

	env->ReleaseStringUTFChars(configFileName, c_configFileName);
	env->ReleaseStringUTFChars(configFileName, c_listenIp);

	return 0;
}