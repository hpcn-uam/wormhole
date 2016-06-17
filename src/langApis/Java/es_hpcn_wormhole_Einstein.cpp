#include "es_hpcn_wormhole_Einstein.h"

#include <einstein/einstein.hpp>
using namespace einstein;

Einstein *_JWH_einstein;

/*
 * Class:     es_hpcn_wormhole_Einstein
 * Method:    init
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_es_hpcn_wormhole_Einstein_init
(JNIEnv *env, jobject obj, jstring configFileName, jstring listenIp, jint listenPort, jboolean autoDeployWorms, jobjectArray RunParams)
{
	UNUSED(obj);
	UNUSED(RunParams); //TODO remove this parameter

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