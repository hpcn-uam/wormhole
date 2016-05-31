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

	const char *c_configFileName = env->GetStringUTFChars(configFileName, NULL);
	const char *c_listenIp       = env->GetStringUTFChars(listenIp, NULL);

	vector<string> runparams;

	if (env->GetArrayLength(RunParams) > 0) {
		for (int i = 0; i < env->GetArrayLength(RunParams); i++) {
			jobject obj = env->GetObjectArrayElement(RunParams, i);
			jstring str = (jstring) obj;
			const char *strchars = env->GetStringUTFChars(str, NULL);
			runparams.push_back(string(strchars));
			env->ReleaseStringUTFChars(str, strchars);
		}
	}

	try {
		_JWH_einstein = new Einstein(c_configFileName, c_listenIp, listenPort, autoDeployWorms == JNI_TRUE, runparams);
		_JWH_einstein->openHoles();

	} catch (std::runtime_error) {
		delete _JWH_einstein;
	}

	env->ReleaseStringUTFChars(configFileName, c_configFileName);
	env->ReleaseStringUTFChars(configFileName, c_listenIp);

	return 0;
}