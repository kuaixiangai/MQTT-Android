/*
Copyright (c) 2009-2014 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License v1.0
and Eclipse Distribution License v1.0 which accompany this distribution.
 
The Eclipse Public License is available at
   http://www.eclipse.org/legal/epl-v10.html
and the Eclipse Distribution License is available at
  http://www.eclipse.org/org/documents/edl-v10.php.
 
Contributors:
   Roger Light - initial implementation and documentation.
*/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <arpa/inet.h> 
#include <asm/io.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#include <winsock2.h>
#define snprintf sprintf_s
#endif

#include <time.h>
#include <android/log.h>
#include "jni.h"
#include "mosquitto_internal.h"
#include "time_mosq.h"
#include <mosquitto.h>
#include <pthread.h>
#include <android/log.h>
#include "client_shared.h"
#include <uuid.h>

bool process_messages = true;
int msg_count = 0;
struct mosq_config* g_sub_config = NULL;
struct mosquitto *g_sub_mosq = NULL;

char change_topic[128] = {'\0'};
int change_qos = 0;

int sub_set = 0;

pthread_mutex_t mutex; 

static int write_file(const struct mosquitto_message *message, const struct mosq_config *getcfg, char *sender_id, char file_path[]) {
	//Ê¹ÓÃuuidÉú³ÉÎ¨Ò»±êÊ¶Âë
	uuid_t uuid;
	char str[48];
	uuid_generate(uuid);
	uuid_unparse(uuid, str);

	/*
	time_t t;
	int j = 0;
	j = time(&t);
	*/

	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "xxx1----:%s!\n", getcfg->id);
	#endif
	
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "str:%s!\n", str);
	//char file_path_name[255] = {'\0'};
	FILE * fptr = NULL;
	sprintf(file_path, "%s/%s.amr", getcfg->file_input, str);
	fptr = fopen(file_path, "wb+");

#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "xxx2----!\n");
#endif

	if(NULL == fptr) {
//		#ifdef PRINT_ANDROID_INFO
			__android_log_print(ANDROID_LOG_INFO, "JNITag", ":%s ,cant not open!\n", file_path);
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "cant not open!\n");
//		#endif
		return -1;
	}else {
		fwrite(message->payload+sizeof(struct sendMessBody), 1, message->payloadlen -sizeof(struct sendMessBody), fptr);
		fclose(fptr);
		fptr = NULL;
	}

	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "xxx3----!\n");
	#endif

	return 0;
}

static int get_text_msg(const struct mosquitto_message *message, struct mosq_config *getcfg, char msg_content[]) {
	int sendMsgLen = sizeof(struct sendMessBody);
	int msg_len = message->payloadlen - sendMsgLen; //

	memcpy(msg_content, message->payload + sendMsgLen, msg_len);

	return 0;
}

static int send_file_msg_tojava(struct sendMessBody *reve, struct mosq_config *getcfg, char *filepath) {
	//1.ÃÃˆÂ¶Ã”Ã€Ã ÃÃÂ½Ã¸ÃÃÃ—ÂªÂ»Â»
	char tem_audioPlayTime[8]={'\0'};
	char tem_senderType[8]={'\0'};

	sprintf(tem_audioPlayTime, "%d", reve->audio_play_time);
	sprintf(tem_senderType, "%d", reve->sender_type);
	
	//2.Â·Â¢Ã‹ÃÃŽÃ„Â¼Ã¾ÃÃ»ÃÂ¢
	JNIEnv *sub_env = getcfg->sub_env;
	jobject sub_obj = getcfg->sub_obj;
	jclass sub_class=(*sub_env)->GetObjectClass(sub_env, sub_obj);
	if ((*sub_env)->ExceptionCheck(sub_env)) {
   		return -2;
	}
	if((sub_env == NULL) || (sub_obj == NULL) || (sub_class == NULL))
	{
		return -1;
	}

	jstring jstrFilePath = NULL;
	jstring jstrUserName = NULL;
	jstring jstrPortraitPath = NULL;
	jstring jstraudio_play_time = NULL;
	jstring jstrSenderType = NULL;

	jmethodID sound_methodID = (*sub_env)->GetMethodID(sub_env, sub_class, "SendCliSounds", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	if(sound_methodID) {
		jstrFilePath = (*sub_env) -> NewStringUTF(sub_env, filepath);
		jstrUserName = (*sub_env) -> NewStringUTF(sub_env, reve->user_name);
		jstrPortraitPath     = (*sub_env) -> NewStringUTF(sub_env, reve->portrait_path);
		jstraudio_play_time  = (*sub_env) -> NewStringUTF(sub_env, tem_audioPlayTime);
		jstrSenderType       = (*sub_env) -> NewStringUTF(sub_env, tem_senderType);
	}

	if((jstrFilePath == NULL) || (jstrUserName == NULL) ||
		(jstrPortraitPath == NULL) || (jstraudio_play_time == NULL) ||
		(jstrSenderType == NULL)) {
		goto PROCESS_MESSAGE_SOUND_ERR;
	}

	//Ã•Ã¦Ã•Ã½ÂµÃ·Ã“ÃƒÂºÂ¯ÃŠÃ½
	(*sub_env)->CallVoidMethod(sub_env, sub_obj, sound_methodID, jstrFilePath, jstrUserName, jstrPortraitPath, jstraudio_play_time, jstrSenderType);


/* ÃŠÃÂ·Ã…jniÃ—ÃŠÃ”Â´ */
PROCESS_MESSAGE_SOUND_ERR:
	if(jstrFilePath != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrFilePath);
	if(jstrUserName != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrUserName);
	if(jstrPortraitPath != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrPortraitPath);
	if(jstraudio_play_time != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstraudio_play_time);
	if(jstrSenderType != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrSenderType);

	(*sub_env)->DeleteLocalRef(sub_env, sub_class);

	return 0;
}

static int send_text_msg_tojava(struct sendMessBody *reve, struct mosq_config *getcfg, char *msg) {
	//1.ÃÃˆÂ¶Ã”Ã€Ã ÃÃÂ½Ã¸ÃÃÃ—ÂªÂ»Â»
	char tem_senderType[8]={'\0'};

	sprintf(tem_senderType, "%d", reve->sender_type);
	
	//2.Â·Â¢Ã‹ÃÃŽÃ„Â¼Ã¾ÃÃ»ÃÂ¢
	JNIEnv *sub_env = getcfg->sub_env;
	jobject sub_obj = getcfg->sub_obj;
	jclass sub_class=(*sub_env)->GetObjectClass(sub_env, sub_obj);
	if ((*sub_env)->ExceptionCheck(sub_env)) {
   		return -2;
	}
	if((sub_env == NULL) || (sub_obj == NULL) || (sub_class == NULL))
	{
		return -1;
	}

	jstring jstrTextMsg  = NULL;
	jstring jstrUserName = NULL;
	jstring jstrPortraitPath = NULL;
	jstring jstrSenderType = NULL;

	jmethodID sound_methodID = (*sub_env)->GetMethodID(sub_env, sub_class, "SendCliTextMsg", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	if(sound_methodID) {
		jstrTextMsg = (*sub_env) -> NewStringUTF(sub_env, msg);
		jstrUserName = (*sub_env) -> NewStringUTF(sub_env, reve->user_name);
		jstrPortraitPath     = (*sub_env) -> NewStringUTF(sub_env, reve->portrait_path);
		jstrSenderType       = (*sub_env) -> NewStringUTF(sub_env, tem_senderType);
	}

	if((jstrTextMsg == NULL) || (jstrUserName == NULL) ||
		(jstrPortraitPath == NULL) || (jstrSenderType == NULL)) {
		goto PROCESS_MESSAGE_SOUND_ERR;
	}

	//Ã•Ã¦Ã•Ã½ÂµÃ·Ã“ÃƒÂºÂ¯ÃŠÃ½
	(*sub_env)->CallVoidMethod(sub_env, sub_obj, sound_methodID, jstrTextMsg, jstrUserName, jstrPortraitPath, jstrSenderType);

	return 0;
/* ÃŠÃÂ·Ã…jniÃ—ÃŠÃ”Â´ */
PROCESS_MESSAGE_SOUND_ERR:
	if(jstrTextMsg != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrTextMsg);
	if(jstrUserName != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrUserName);
	if(jstrPortraitPath != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrPortraitPath);
	if(jstrSenderType != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrSenderType);
	
	(*sub_env)->DeleteLocalRef(sub_env, sub_class);
	return 0;
}

static send_cmd_tojava(struct sendMessBody *reve, struct mosq_config *getcfg) {
	int rc = 0;
	char java_exc_class[] = "java/lang/IllegalArgumentException";
	
	//1.ÃÃˆÂ¶Ã”Ã€Ã ÃÃÂ½Ã¸ÃÃÃ—ÂªÂ»Â»
	char tem_senderType[8]={'\0'};
	char tem_cmd[8] = {'\0'};

	sprintf(tem_senderType, "%d", reve->sender_type);
	sprintf(tem_cmd, "%d", reve->trans_mode);
	
	//2.Â·Â¢Ã‹ÃÃŽÃ„Â¼Ã¾ÃÃ»ÃÂ¢
	JNIEnv *sub_env = getcfg->sub_env;
	jobject sub_obj = getcfg->sub_obj;
	jclass sub_class=(*sub_env)->GetObjectClass(sub_env, sub_obj);
	if ((*sub_env)->ExceptionCheck(sub_env)) {
   		return -2;
	}
	if((sub_env == NULL) || (sub_obj == NULL) || (sub_class == NULL))
	{
		return -1;
	}

	jstring jstrTransMode  = NULL;
	jstring jstrUserName = NULL;
	jstring jstrPortraitPath = NULL;
	jstring jstrSenderType = NULL;

#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "test1\n");
#endif
	jmethodID sound_methodID = (*sub_env)->GetMethodID(sub_env, sub_class, "SendCliCmd", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	if ((*sub_env)->ExceptionCheck(sub_env)) {
   		return -2;
	}
	printf("THe sound_mothodId:%d\n", sound_methodID);
	if(sound_methodID) {
		jstrTransMode = (*sub_env) -> NewStringUTF(sub_env, tem_cmd);
		jstrUserName = (*sub_env) -> NewStringUTF(sub_env, reve->user_name);
		jstrPortraitPath     = (*sub_env) -> NewStringUTF(sub_env, reve->portrait_path);
		jstrSenderType       = (*sub_env) -> NewStringUTF(sub_env, tem_senderType);
	}
	
#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "test2\n");
#endif

	if((jstrTransMode == NULL) || (jstrUserName == NULL) ||
		(jstrPortraitPath == NULL) || (jstrSenderType == NULL)) {
		goto PROCESS_MESSAGE_SOUND_ERR;
	}

	//Ã•Ã¦Ã•Ã½ÂµÃ·Ã“ÃƒÂºÂ¯ÃŠÃ½
	(*sub_env)->CallVoidMethod(sub_env, sub_obj, sound_methodID, jstrTransMode, jstrUserName, jstrPortraitPath, jstrSenderType);

/* ÃŠÃÂ·Ã…jniÃ—ÃŠÃ”Â´ */
PROCESS_MESSAGE_SOUND_ERR:
	if(jstrTransMode != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrTransMode);
	if(jstrUserName != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrUserName);
	if(jstrPortraitPath != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrPortraitPath);
	if(jstrSenderType != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrSenderType);
	
	//(*sub_env)->DeleteLocalRef(sub_env, sub_class);
	return 0;
}

int send_msg_toJava(struct sendMessBody *reve, struct mosq_config *getcfg, char *pass_msg) {	
	char tem_senderType[8]={'\0'};
	sprintf(tem_senderType, "%d", reve->sender_type);
	char tem_cmd[8] = {'\0'};
	sprintf(tem_cmd, "%d", reve->trans_mode);

	char tem_chatMode[8] = {'\0'};
	sprintf(tem_chatMode, "%d", reve->chat_mode);

	//char tem_fromUserID[48] = {'\0'};
	//if(reve->id != NULL) {
	//	sprintf(tem_fromUserID, "%s", reve->id);
	//}

	char tem_toUserID[48] = {'\0'};
	if(reve->chat_mode == 3) {
		if(reve->toUserID != NULL) {
			sprintf(tem_toUserID, "%s", reve->toUserID);
		}
	}
	
	JNIEnv *sub_env = getcfg->sub_env;
	jobject sub_obj = getcfg->sub_obj;
	jclass sub_class=(*sub_env)->GetObjectClass(sub_env, sub_obj);

	jstring jstrFromUserID = NULL;
	jstring jstrToUserID   = NULL;
	jstring jstrChatMode   = NULL;
	jstring jstrTransMode  = NULL;
	jstring jstrUserName   = NULL;
	jstring jstrPortraitPath = NULL;
	jstring jstrSenderType   = NULL;

	//·¢ËÍÎÄ¼þÌØÓÐ
	jstring jstrFilePath = NULL;
	jstring jstraudio_play_time = NULL;

	//·¢ËÍÎÄ±¾ÏûÏ¢ÌØÓÐ
	jstring jstrTextMsg  = NULL;
	
	if((sub_env == NULL) || (sub_obj == NULL) || (sub_class == NULL))
	{
		return -1;
	}
	jmethodID sound_methodID = (*sub_env)->GetMethodID(sub_env, sub_class, "SendCliAllMsg", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");	
	if ((*sub_env)->ExceptionCheck(sub_env)) {
   		return -2;
	}

#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "tranmode:%d\n", reve->trans_mode);
#endif
	if((reve->trans_mode == SEND_SOUNDS_TRANS_MODE)) {
		char tem_audioPlayTime[8]={'\0'};
		sprintf(tem_audioPlayTime, "%d", reve->audio_play_time);
		
		if(sound_methodID) {
			jstrFromUserID = (*sub_env) -> NewStringUTF(sub_env, reve->id); 
			jstrChatMode   = (*sub_env) -> NewStringUTF(sub_env, tem_chatMode); 
			jstrTransMode  = (*sub_env) -> NewStringUTF(sub_env, tem_cmd); 
			jstrSenderType = (*sub_env) -> NewStringUTF(sub_env, tem_senderType);
			jstrFilePath   = (*sub_env) -> NewStringUTF(sub_env, pass_msg);
			jstrUserName   = (*sub_env) -> NewStringUTF(sub_env, reve->user_name);
			jstrPortraitPath	 = (*sub_env) -> NewStringUTF(sub_env, reve->portrait_path);
			jstraudio_play_time  = (*sub_env) -> NewStringUTF(sub_env, tem_audioPlayTime);			
		}
		
		if((jstrChatMode == NULL) || (jstrTransMode == NULL) || (jstrFilePath == NULL) || (jstrUserName == NULL) ||
			(jstrPortraitPath == NULL) || (jstraudio_play_time == NULL) ||
			(jstrSenderType == NULL)) {
			goto PROCESS_MESSAGE_SOUND_ERR;
		}

		if(reve->chat_mode == 3) {
			jstrToUserID = (*sub_env) -> NewStringUTF(sub_env, tem_toUserID); 
			(*sub_env)->CallVoidMethod(sub_env, sub_obj, sound_methodID, jstrChatMode, jstrTransMode, jstrFilePath, jstrUserName, jstrPortraitPath, jstraudio_play_time, jstrSenderType, jstrToUserID, jstrFromUserID);
		}else {
			(*sub_env)->CallVoidMethod(sub_env, sub_obj, sound_methodID, jstrChatMode, jstrTransMode, jstrFilePath, jstrUserName, jstrPortraitPath, jstraudio_play_time, jstrSenderType, NULL, jstrFromUserID);
		}
		//Ã•Ã¦Ã•Ã½ÂµÃ·Ã“ÃƒÂºÂ¯ÃŠÃ½

		
	}else if(reve->trans_mode == SEND_WORDS_TRANS_MODE) {
		/* ´«ÊäÎÄ×Ö */	
		if(sound_methodID) {
			jstrFromUserID = (*sub_env) -> NewStringUTF(sub_env, reve->id); 
			jstrChatMode  = (*sub_env) -> NewStringUTF(sub_env, tem_chatMode);
			jstrTransMode = (*sub_env) -> NewStringUTF(sub_env, tem_cmd);
			jstrSenderType		 = (*sub_env) -> NewStringUTF(sub_env, tem_senderType);
			jstrTextMsg = (*sub_env) -> NewStringUTF(sub_env, pass_msg);
			jstrUserName = (*sub_env) -> NewStringUTF(sub_env, reve->user_name);
			jstrPortraitPath	 = (*sub_env) -> NewStringUTF(sub_env, reve->portrait_path);
		}
		
		if((jstrChatMode == NULL) || (jstrTransMode == NULL) || (jstrTextMsg == NULL) || (jstrUserName == NULL) ||
			(jstrPortraitPath == NULL) || (jstrSenderType == NULL)) {
			goto PROCESS_MESSAGE_SOUND_ERR;
		}
		
		//Ã•Ã¦Ã•Ã½ÂµÃ·Ã“ÃƒÂºÂ¯ÃŠÃ½
		if(reve->chat_mode == 3) {
			jstrToUserID = (*sub_env) -> NewStringUTF(sub_env, tem_toUserID); 
			(*sub_env)->CallVoidMethod(sub_env, sub_obj, sound_methodID, jstrChatMode, jstrTransMode, jstrTextMsg, jstrUserName, jstrPortraitPath, jstrSenderType, NULL, jstrToUserID, jstrFromUserID);
		}else {
			(*sub_env)->CallVoidMethod(sub_env, sub_obj, sound_methodID, jstrChatMode, jstrTransMode, jstrTextMsg, jstrUserName, jstrPortraitPath, jstrSenderType, NULL, NULL, jstrFromUserID);
		}
	
		
	}else if((reve->trans_mode == ADD_PRAISE_TRANS_MODE) ||
			(reve->trans_mode == UP_Lines_TRANS_MODE) || 
			(reve->trans_mode == DOWN_Lines_TRANS_MODE)||
			(reve->trans_mode == ADD_FRIENDS_MODE)||
			(reve->trans_mode == DEL_FRIENDS_MODE)) {	
		if(sound_methodID) {
			jstrFromUserID = (*sub_env) -> NewStringUTF(sub_env, reve->id); 
			jstrChatMode  = (*sub_env) -> NewStringUTF(sub_env, tem_chatMode);
			jstrTransMode = (*sub_env) -> NewStringUTF(sub_env, tem_cmd);
			jstrUserName  = (*sub_env) -> NewStringUTF(sub_env, reve->user_name);
			jstrPortraitPath	 = (*sub_env) -> NewStringUTF(sub_env, reve->portrait_path);
			jstrSenderType		 = (*sub_env) -> NewStringUTF(sub_env, tem_senderType);
		}
		
		
		if((jstrChatMode == NULL) || (jstrTransMode == NULL) || (jstrUserName == NULL) ||
			(jstrPortraitPath == NULL) || (jstrSenderType == NULL)) {
			goto PROCESS_MESSAGE_SOUND_ERR;
		}
	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "exec java function\n");
	#endif
		if(reve->chat_mode == 3) {
			jstrToUserID = (*sub_env) -> NewStringUTF(sub_env, tem_toUserID); 
			(*sub_env)->CallVoidMethod(sub_env, sub_obj, sound_methodID, jstrChatMode, jstrTransMode, jstrUserName, jstrPortraitPath, jstrSenderType, NULL, NULL, jstrToUserID, jstrFromUserID);
		}else {
			(*sub_env)->CallVoidMethod(sub_env, sub_obj, sound_methodID, jstrChatMode, jstrTransMode, jstrUserName, jstrPortraitPath, jstrSenderType, NULL, NULL, NULL, jstrFromUserID);
		}
		
				
	}else {
		printf("Ã»ÓÐ´ËÃüÁî!\n");
	}


PROCESS_MESSAGE_SOUND_ERR:
	if(jstrTextMsg != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrTextMsg);
	if(jstrUserName != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrUserName);
	if(jstrPortraitPath != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrPortraitPath);
	if(jstrSenderType != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrSenderType);
	if(jstrFilePath != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrFilePath);
	if(jstrTransMode != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrTransMode);	
	if(jstrChatMode != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrChatMode);
	if(jstrToUserID != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrToUserID);
	if(jstrFromUserID != NULL)
		(*sub_env)->DeleteLocalRef(sub_env, jstrFromUserID);
	(*sub_env)->DeleteLocalRef(sub_env, sub_class);
	return 0;

	
}

static int process_message(const struct mosquitto_message *message, struct mosq_config *getcfg) {
	char file_path[128] = {'\0'};
	char text_msg[2048] = {'\0'}; //2048Â¸Ã¶Ã—Ã–Â½Ãš
	//1.ÃÃˆÂ»Ã±ÃˆÂ¡Â½Ã¡Â¹Â¹ÃŒÃ¥ÃÃ…ÃÂ¢
	printf("Get len is :%d\n", message->payloadlen);
	struct sendMessBody *reve = (struct sendMessBody *)malloc(sizeof(struct sendMessBody));
	memset(reve, 0, sizeof(struct sendMessBody));
	memcpy(reve, (char *)message->payload, sizeof(struct sendMessBody));

//#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "Â´Ã²Ã“Â¡ÃˆÂ¡Â³Ã¶Ã€Â´ÂµÃ„ÃŠÃ½Â¾Ã!\n");

	__android_log_print(ANDROID_LOG_INFO, "JNITag", "audio_create_time:%d\n", reve->audio_create_time);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "brand:%s\n", reve->brand);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "channel_id:%s\n", reve->channel_id);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "id:%s\n", reve->id);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "chat_mode:%d\n", reve->chat_mode);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "trans_mode:%d\n", reve->trans_mode);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "ToUserID:%s\n", reve->toUserID);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "phone_type:%s\n", reve->phone_type);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "portrait_path:%s\n", reve->portrait_path);
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "name:%s\n", reve->user_name);
//#endif

	//2.Â¶Ã”Â²Â»ÃÂ¬ÂµÃ„ÃÃ»ÃÂ¢Ã€Ã ÃÃÂ½Ã¸ÃÃÂ´Â¦Ã€Ã­
	if( SEND_SOUNDS_TRANS_MODE == reve->trans_mode ) {
		/* ÃŠÃ—ÃÃˆÂ¶Ã”ÃŽÃ„Â¼Ã¾Â½Ã¸ÃÃÂ´Ã¦Â´Â¢ */
		#ifdef PRINT_ANDROID_INFO
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "tetst1----!\n");
		#endif
		if(write_file(message, getcfg, reve->id, file_path) == 0) {
			send_msg_toJava(reve, getcfg, file_path);	
		}else {
			return -1;
		}
		#ifdef PRINT_ANDROID_INFO
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "tetst2----!\n");
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "return filePath:%s\n", file_path);
		
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "tetst2----!\n");
		#endif
	}else if(SEND_WORDS_TRANS_MODE == reve->trans_mode) {
		get_text_msg(message, getcfg, text_msg);
		#ifdef PRINT_ANDROID_INFO
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "return text_msg:%s\n", text_msg);
		#endif
		send_msg_toJava(reve, getcfg, text_msg);
	}else if(ADD_PRAISE_TRANS_MODE == reve->trans_mode) {
#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "test111\n");
#endif
		/* Â²Â»ÃÃ¨Ã’ÂªÃ—Ã¶ÃŒÃ˜Â±Ã°ÂµÃ„Â²Ã™Ã—Ã·Â£Â¬Ã–Â»ÃÃ¨Ã’ÂªÃ–Â±Â½Ã“Â·Â¢Ã‹ÃÂ¼Â´Â¿Ã‰ */
		send_msg_toJava(reve, getcfg, NULL);
	}else if(UP_Lines_TRANS_MODE == reve->trans_mode) {
	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "test222");
	#endif
		int ret = 0;
		ret = send_msg_toJava(reve, getcfg, NULL);
	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "return ret:%d\n", ret);		
	#endif
	}else if(DOWN_Lines_TRANS_MODE == reve->trans_mode) {
	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "test333");
	#endif
		send_msg_toJava(reve, getcfg, NULL);
	}else if(ADD_FRIENDS_MODE == reve->trans_mode){		
	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "test444");
	#endif
		send_msg_toJava(reve, getcfg, NULL);
	}else if(DEL_FRIENDS_MODE == reve->trans_mode) {
		send_msg_toJava(reve, getcfg, NULL);		
	}else {
		
	}

	//3.Â½Â«ÃÃ Â¹Ã˜ÂµÃ„ÃÃ»ÃÂ¢ÃÂ¨Ã–ÂªÂµÂ½javaÂ²Ã£

	free(reve);
	printf("---Get end ---!\n");

	return 0;
}

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	//subï¿½Ë½ï¿½ï¿½Ð½ï¿½ï¿½ï¿½ï¿½ï¿½Ï¢
	struct mosq_config *cfg;
	int i;
	bool res;

	if(process_messages == false) return;

	assert(obj);
	cfg = (struct mosq_config *)obj;

	if(message->retain && cfg->no_retain) return;
	if(cfg->filter_outs){
		for(i=0; i<cfg->filter_out_count; i++){
			mosquitto_topic_matches_sub(cfg->filter_outs[i], message->topic, &res);
			if(res) return;
		}
	}

	if(cfg->verbose){
		if(message->payloadlen){
			printf("%s ", message->topic);
			fwrite(message->payload, 1, message->payloadlen, stdout);
			if(cfg->eol){
				printf("\n");
			}
		}else{
			if(cfg->eol){
				printf("%s (null)\n", message->topic);
			}
		}
		fflush(stdout);
	}else{
		if(message->payloadlen){
			//ï¿½Ô½ï¿½ï¿½Õ¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï¢ï¿½ï¿½ï¿½Ð´ï¿½ï¿½ï¿½
			process_message(message, cfg);
			//fwrite(message->payload, 1, message->payloadlen, stdout);
			if(cfg->eol){
				printf("\n");
			}
			//fflush(stdout);
		}
	}
	if(cfg->msg_count>0){
		msg_count++;
		if(cfg->msg_count == msg_count){
			process_messages = false;
			mosquitto_disconnect(mosq);
		}
	}
}

void my_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "Reconnect -Exec Topic->xxxx\r\n");
#endif
	int i;
	struct mosq_config *cfg;

	assert(obj);
	cfg = (struct mosq_config *)obj;

	if(!result){
		
		for(i=0; i<cfg->topic_count; i++){
			mosquitto_subscribe(mosq, NULL, cfg->topics[i], cfg->topic_qos[i]);
		}
		//__android_log_print(ANDROID_LOG_INFO, "JNITag", "SubJni: connectTopic:%s\r\n", g_sub_config->topic);
		
		//mosquitto_subscribe(mosq, NULL, g_sub_config->topic, cfg->qos);
	}else{
		if(result && !cfg->quiet){
			fprintf(stderr, "%s\n", mosquitto_connack_string(result));
		}
	}
}

void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	//¿ÉÒÔ½«topicÐ´ÈëtopicsÖÐ
#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "subcribe callback......\r\n");
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "topic:%s......\r\n", change_topic);
#endif
	int rc = 0;
	if(sub_set == 1) {
		pthread_mutex_lock(&mutex);  
		rc = client_topic_set(g_sub_config, CLIENT_SUB, change_topic, change_qos);	
		sub_set = 0;
		pthread_mutex_unlock(&mutex);  
	}

	
	/*	
	int i;
	struct mosq_config *cfg;

	assert(obj);
	cfg = (struct mosq_config *)obj;

	if(!cfg->quiet) printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		if(!cfg->quiet) printf(", %d", granted_qos[i]);
	}
	if(!cfg->quiet) printf("\n");
	*/
}

void my_unsubscribe_callback(struct mosquitto *mosq, void *obj, int mid) {
	//ÍËÔÄ³É¹¦½«topicÊÍ·Åµô,²¢Í¨Öªjava

	pthread_mutex_lock(&mutex);
	g_sub_config->topic_count--;
	pthread_mutex_unlock(&mutex);
	
	/* ÐÞ¸Äby kuaixiang 20151201 
	g_sub_config->topic = change_topic;
	
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "SubJni: resubcribe:%s\r\n", change_topic);
	mosquitto_subscribe(g_sub_mosq, NULL, g_sub_config->topic, 0);
	*/
}

/*
void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	printf("%s\n", str);
}*/



//ï¿½ï¿½ï¿½ï¿½: ï¿½ï¿½ï¿½ï¿½Ê±ï¿½Í·ï¿½struct mosquittoï¿½ï¿½mosq_configï¿½ï¿½ï¿½ï¿½
//ï¿½ï¿½ï¿½ï¿½:
//    struct mosquitto *pMosq:         Òªï¿½ï¿½ï¿½Ãµï¿½struct mosquittoï¿½ï¿½ï¿½ï¿½
//    struct mosq_config *pMosqConfig: pMosqConfigï¿½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï¢
//ï¿½ï¿½ï¿½ï¿½Öµ:
//      0-ï¿½ï¿½È·ï¿½ï¿½1-ï¿½ï¿½ï¿½ï¿½
int CliSubFree(struct mosquitto *pMosq, struct mosq_config *pMosqConfig)
{	
	if(pMosqConfig)
	{
		if(pMosqConfig->topic_count != 0)
		{
			//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ë¶ï¿½Ö®Ç°ï¿½ï¿½topic
			int i = 0;
			for(i = 0; i < pMosqConfig->topic_count; i++)
			{
				mosquitto_unsubscribe(pMosq, NULL, pMosqConfig->topics[i]);
			}
			pMosqConfig->topic_count = 0;
			
		}
				
		free(pMosqConfig);
		pMosqConfig = NULL;
	}
	else
	{

	}

	if(pMosq)
	{
		mosquitto_destroy(pMosq);
		pMosq = NULL;
		mosquitto_lib_cleanup();
	}	
	else
	{
	}
	
	return 0;
}

//subï¿½Ë½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
int sub_init_connect(JNIEnv *env, jobject obj, carRadio_sub sub_object)
{
	 pthread_mutex_init(&mutex, NULL); 

	int rc = 0;
	//1.ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ÄºÏ·ï¿½ï¿½ï¿½
	if((SUCCESS_RESULT != invalid_ipaddr(sub_object.server_ip) ) 
		|| (sub_object.server_port < MIN_SERVER_PORT) || (sub_object.server_port > MAX_SERVER_PORT))
	{
		return INPUT_ILLEGAL_PARA;
	}

	if((sub_object.keepalive < MIN_KEEPALIVE_VALUE) || (sub_object.keepalive >= MAX_KEEPALIVE_VALUE))
	{
		sub_object.keepalive = DEF_KEEPALIVE_VALUE;
	}

	if((sub_object.qos < 0) || (sub_object.qos >= 3))
	{
		sub_object.qos = 0;
	}
	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "SubJni: sub_connect_cns_new[res 1]\r\n");
	#endif
	//2.ï¿½ï¿½ï¿½ï¿½subï¿½á¹¹ï¿½ï¿½
	g_sub_config = (struct mosq_config*)malloc(sizeof(struct mosq_config));
	memset(g_sub_config, 0, sizeof(struct mosq_config));
	g_sub_config->sub_env = env;
	g_sub_config->sub_obj = obj;
	g_sub_config->file_input = sub_object.file_path;
	g_sub_config->id         = sub_object.uuid;
	g_sub_config->client_type = sub_object.client_type;
	g_sub_config->clean_session = sub_object.clean_session;
	g_sub_config->keepalive = sub_object.keepalive;
	g_sub_config->product_id = sub_object.product_id;
	g_sub_config->product_type = sub_object.product_type;
	g_sub_config->quiet = sub_object.quiet;
	g_sub_config->qos = sub_object.qos;
	g_sub_config->port = sub_object.server_port;
	g_sub_config->host = sub_object.server_ip;
	g_sub_config->protocol_version = MQTT_PROTOCOL_V31;
	//g_sub_config->topic = sub_object.topic;
#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "topics[0]:%s\r\n", sub_object.topics[0]);
#endif
	//3.ï¿½ï¿½ï¿½Ä·Ö·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½(CDS)ï¿½ï¿½topic
	int i_num = 0;
	pthread_mutex_lock(&mutex);  
	for(i_num = 0; i_num < sub_object.topic_num; i_num++) {
		rc = client_topic_set(g_sub_config, CLIENT_SUB, sub_object.topics[i_num], sub_object.qos); 	
	}
	pthread_mutex_unlock(&mutex);
	mosquitto_lib_init();

	//4.ÉèÖÃmosq
	mosquitto_max_inflight_messages_set(g_sub_mosq, 100);
	g_sub_config->retain = false;
	mosquitto_opts_set(g_sub_mosq, MOSQ_OPT_PROTOCOL_VERSION, &(g_sub_config->protocol_version));
	g_sub_mosq = mosquitto_new(g_sub_config->id, g_sub_config->clean_session, g_sub_config);
	if(!g_sub_mosq){
		switch(errno){
			case ENOMEM:
				if(!g_sub_config->quiet) fprintf(stderr, "Error: Out of memory.\n");
				break;
			case EINVAL:
				if(!g_sub_config->quiet) fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
				break;
		}
		mosquitto_lib_cleanup();
		return 1;
	}
	
	if(client_opts_set(g_sub_mosq, g_sub_config)){
		return 1;
	}

	//5.ï¿½ï¿½ï¿½Ã»Øµï¿½ï¿½ï¿½ï¿½ï¿½
	/*
	if(g_sub_config->debug){
//		mosquitto_log_callback_set(g_sub_mosq, my_log_callback);
		mosquitto_subscribe_callback_set(g_sub_mosq, my_subscribe_callback);
	}*/
	mosquitto_connect_callback_set(g_sub_mosq, my_connect_callback);
	mosquitto_message_callback_set(g_sub_mosq, my_message_callback);
	mosquitto_subscribe_callback_set(g_sub_mosq, my_subscribe_callback);
	mosquitto_unsubscribe_callback_set(g_sub_mosq, my_unsubscribe_callback);

	//g_sub_mosq->userdata = g_sub_config;

	//6.ï¿½ï¿½ï¿½Ó·ï¿½ï¿½ï¿½ï¿½ï¿½
	rc = client_connect(g_sub_mosq, g_sub_config);
	//if(rc) return rc;

/*
	if(SUCCESS_RESULT != rc) {
		CliSubFree(g_sub_mosq, g_sub_config);
		return rc;
	}
*/	
	//7.Ñ­ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
	rc = mosquitto_loop_forever(g_sub_mosq, -1, 1);

	//8.ï¿½Í·Åµï¿½ï¿½ï¿½ï¿½Ðµï¿½ï¿½ï¿½Ô´
	CliSubFree(g_sub_mosq, g_sub_config);

	return rc;
}


//ï¿½ï¿½ï¿½Ð»ï¿½ï¿½ï¿½topicï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
int sub_unscribe_topic(JNIEnv *env, jobject obj, const char *oldtopic, const char *newtopic) {
	//g_sub_config->will_topic = (char *)newtopic;
#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "SubJni: resubcribe:%s\r\n", newtopic);
#endif

	if(strcmp(newtopic, g_sub_config->topic) == 0){
		//Èç¹ûÇÐ»»µÄtopicºÍÄ¿Ç°¶©ÔÄµÄtopicÏàÍ¬Ôò²»ÄÜ½øÐÐÖØÐÂ¶©ÔÄ
		return 2;
	}else {
		sprintf(change_topic, "%s", newtopic);
		mosquitto_unsubscribe(g_sub_mosq, NULL, g_sub_config->topic);
	} 

	
	/*
	int i = 0;
	for(i = 0; i < g_sub_config->topic_count; i++){
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "SubJni: unsubcribe\r\n");
		mosquitto_unsubscribe(g_sub_mosq, NULL, g_sub_config->topics[i]);
	}*/

	return 0;
}

struct mosquitto *get_mosq() {
	return g_sub_mosq;
}

struct mosq_config *get_config() {
	return g_sub_config;
}

//¿Í»§¶Ë½øÈëÆµµÀ
int sub_enter_channel(JNIEnv *env, jobject obj, char *channel_topics, int qos) {
	//ÏÈÅÐ¶ÏÊÇ·ñÒÑ¾­¶©ÔÄ¸Ãtopic
	int count = g_sub_config->topic_count;	
#ifdef PRINT_ANDROID_INFO	
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "g_sub_count:%d\r\n", count);
#endif
	int j = 0;
	for(j = 0; j < count; j++) {
		if(strcmp(channel_topics, g_sub_config->topics[j]) == 0) {
#ifdef PRINT_ANDROID_INFO			
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "g_sub_config:%s\r\n", g_sub_config->topics[j]);
#endif
			//break;
			return -1;
		}
	}

	if((qos < 0) || (qos > 2)) {
		qos = 0;
	}

	//±ØÐë¼ÓÊ±¼äµÄ²Ù×÷ÒÔÈ·±£Ê±¼ä´ÁÄÜ¹»½øÐÐ¸üÐÂ£¬ÐÄÌøÄÜ¹»¼°Ê±Íê³É
	int i = 0;
	time_t now = mosquitto_time();
	time_t last_msg_out;
	time_t last_msg_in;
	pthread_mutex_lock(&g_sub_mosq->msgtime_mutex);
	last_msg_out = g_sub_mosq->last_msg_out;
	last_msg_in = g_sub_mosq->last_msg_in;
	pthread_mutex_unlock(&g_sub_mosq->msgtime_mutex);

	if(g_sub_mosq->state == mosq_cs_connected) {
		i = mosquitto_subscribe(g_sub_mosq, NULL, channel_topics, qos);
		change_qos = qos;
		sprintf(change_topic, "%s", channel_topics);
		sub_set = 1;
		if(i == 0) {
			//º¯ÊýÕý³£·µ»Ø
			pthread_mutex_lock(&g_sub_mosq->msgtime_mutex);
			g_sub_mosq->last_msg_in = now;
			g_sub_mosq->last_msg_out = now;
			pthread_mutex_unlock(&g_sub_mosq->msgtime_mutex);
			return 0;
		}
	}

	pthread_mutex_lock(&g_sub_mosq->msgtime_mutex);
	g_sub_mosq->last_msg_in = now;
	g_sub_mosq->last_msg_out = now;
	pthread_mutex_unlock(&g_sub_mosq->msgtime_mutex);
#ifdef PRINT_ANDROID_INFO	
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "returnvalue:%d\r\n", i);
#endif

	return 0;
}

//¿Í»§¶ËÍË³öÆµµÀ
int sub_quit_channel(JNIEnv *env, jobject obj, char *channel_topic) {
#ifdef PRINT_ANDROID_INFO 
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "unsubcribe\r\n");
#endif
	//±ØÐë¼ÓÊ±¼äµÄ²Ù×÷ÒÔÈ·±£Ê±¼ä´ÁÄÜ¹»½øÐÐ¸üÐÂ£¬ÐÄÌøÄÜ¹»¼°Ê±Íê³É
	int i = 0;
	time_t now = mosquitto_time();
	time_t last_msg_out;
	time_t last_msg_in;
	pthread_mutex_lock(&g_sub_mosq->msgtime_mutex);
	last_msg_out = g_sub_mosq->last_msg_out;
	last_msg_in = g_sub_mosq->last_msg_in;
	pthread_mutex_unlock(&g_sub_mosq->msgtime_mutex);

	
	//ÏÈÅÐ¶ÏÊÇ·ñÒÑ¾­¶©ÔÄ¸Ãtopic
	int count = g_sub_config->topic_count;
#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "g_sub_count:%d\r\n", count);
#endif
	int j = 0;
	for(j = 0; j < count; j++) {
		if(strcmp(channel_topic, g_sub_config->topics[j]) == 0) {
			#ifdef PRINT_ANDROID_INFO
				__android_log_print(ANDROID_LOG_INFO, "JNITag", "g_sub_config:%s\r\n", g_sub_config->topics[j]);
			#endif
			//break;
			if(g_sub_mosq->state == mosq_cs_connected) {
				i = mosquitto_unsubscribe(g_sub_mosq, NULL, channel_topic);
	
				if(i == 0) {
					pthread_mutex_lock(&g_sub_mosq->msgtime_mutex);
					g_sub_mosq->last_msg_in = now;
					g_sub_mosq->last_msg_out = now;
					pthread_mutex_unlock(&g_sub_mosq->msgtime_mutex);
					//º¯ÊýÕý³£·µ»Ø
					return 0;
				}
			}

			pthread_mutex_lock(&g_sub_mosq->msgtime_mutex);
			g_sub_mosq->last_msg_in = now;
			g_sub_mosq->last_msg_out = now;
			pthread_mutex_unlock(&g_sub_mosq->msgtime_mutex);
		
			return 0;
		}
	}

	return -1;
	
//	return 0;
}

//½«Êý¾ÝÌî³äµ½pub_configÖÐ
static int init_base_pub_config(struct mosq_config *pub_config, carRadio_pub pub_object) {
	pub_config->max_inflight     = 20;
	pub_config->protocol_version = MQTT_PROTOCOL_V31;
	pub_config->eol              = true;
	pub_config->connected        = true;

	pub_config->id           = pub_object.uuid;
	pub_config->user_name    = pub_object.user_name; //
	pub_config->product_id   = pub_object.product_id;
	pub_config->user_type    = pub_object.user_type;
	pub_config->client_type  = pub_object.client_type;
	pub_config->product_type = pub_object.product_type;
	pub_config->sender_type  = pub_object.sender_type;
	pub_config->channel_id   = pub_object.channel_id;
	pub_config->brand        = pub_object.brand;
	pub_config->phone_type   = pub_object.phone_type;

	pub_config->chat_mode    = pub_object.chat_mode;
	pub_config->trans_mode   = pub_object.trans_mode;
	pub_config->quiet        = pub_object.quiet;
	pub_config->retain       = pub_object.retain;
	pub_config->host         = pub_object.server_ip;
	pub_config->port         = pub_object.server_port;
	pub_config->topic        = pub_object.topic;
	pub_config->portrait_path= pub_object.portrait_path;

	if((pub_object.qos < 0) || (pub_object.qos > 2)) {
		pub_config->qos  = 0;
	}else {
		pub_config->qos  = pub_object.qos;
	}

	/*
	if(pub_object.userType == 1) {
		pub_config.user_name = pub_object.username;
	}*/
	//__android_log_print(ANDROID_LOG_INFO, "JNITag", "g_sub_count:%d\r\n", pub_config->user_type);

	return 0;
	
}


//¶ÔÎÄ¼þÂ·¾¶½øÐÐÇÐ¸îÒÔ±ãÈ¡µÃÎÄ¼þÃû³Æ
static char* split_filename(const char *voice_path) 
{	
	int n_num = 0;
	char *p[20];
	char *temp_buf = (char *)malloc(255);
	if(NULL == temp_buf)
	{
		fprintf(stderr, "File[%s]Line[%u]: struct mosq_config malloc failed!\r\n", __FILE__, __LINE__);
		return NULL;
	}
	
	
	if(NULL == voice_path) {
		fprintf(stderr, "\nThe voice_path must have value!!!\n");
		if(NULL != temp_buf)
		{
			free(temp_buf);
			temp_buf = NULL;
		}
		return NULL;
	}else {
		strcpy(temp_buf, voice_path);
		while((p[n_num] = strtok(temp_buf, "/")) != NULL) {
			n_num++;
			temp_buf = NULL; 
		}
	}
	
	free(temp_buf);
	return p[n_num - 1];
}


//ÉèÖÃ·¢ËÍ½á¹¹Ìå
static int SetMessBody(struct sendMessBody *pMsg, struct mosq_config * pConfig)
{
	/* ï¿½ï¿½ä¹«ï¿½ï¿½ï¿½Ö¶ï¿½ */
	strcpy(pMsg->id, pConfig->id);	
	strcpy(pMsg->phone_type, pConfig->phone_type);				/* ï¿½Ö»ï¿½ÏµÍ³*/
	strcpy(pMsg->product_id, pConfig->product_id);
	strcpy(pMsg->channel_id, pConfig->channel_id);
	strcpy(pMsg->brand, pConfig->brand);						/* ï¿½Ö»ï¿½Æ·ï¿½ï¿½*/
	strcpy(pMsg->user_name, pConfig->user_name);
	strcpy(pMsg->portrait_path, pConfig->portrait_path);		/* ï¿½Ã»ï¿½Í·ï¿½ï¿½Â·ï¿½ï¿½ */
	pMsg->userType = pConfig->user_type; 	//ï¿½Ç·ï¿½×¢ï¿½ï¿½ï¿½Â¼ï¿½ï¿½
	pMsg->sender_type = pConfig->sender_type;
	pMsg->chat_mode   = pConfig->chat_mode;
	pMsg->trans_mode  = pConfig->trans_mode;
	pMsg->product_type = pConfig->product_type;

	//__android_log_print(ANDROID_LOG_INFO, "JNITag", "g_sub_count11:%d\r\n", pConfig->userType);

	//__android_log_print(ANDROID_LOG_INFO, "JNITag", "g_sub_count22:%d\r\n", pConfig->user_type);


	//char *pVoicePath = NULL;
	if(pConfig->chat_mode == 3) {
		strcpy(pMsg->toUserID, pConfig->toUserID);
	}
	
	switch(pConfig->trans_mode){
		case 1:
//			pVoicePath = split_filename(pConfig->file_input);
//			strcpy(pMsg->voice_name, pVoicePath);
			pMsg->file_type       = pConfig->file_type;
			pMsg->audio_play_time = pConfig->audio_play_time;
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		default:
			break;
	}

	//ï¿½ï¿½Ó¡ï¿½ï¿½Ï¢
	//printf("sendMessBody->voice_name: %s\r\n", pMsg->voice_name);
	printf("sendMessBody->carid: %s\r\n", pMsg->id);
	printf("sendMessBody->product_id: %s\r\n", pMsg->product_id);
	return 0;
}

//»ñÈ¡ÎÄ¼þ»òÕßÏûÏ¢µÄÄÚÈÝ
static int GetFileOrMsg(char *pFileOrMsg, int iFileOrMsgLen, struct mosq_config *pConfig)
{	
	if(SEND_SOUNDS_TRANS_MODE == pConfig->trans_mode)//ï¿½ï¿½ï¿½ï¿½ï¿½Ä¼ï¿½
	{	
		FILE *pFile = NULL;
		pFile = fopen(pConfig->file_input, "ab+");

		if(pFile == NULL) 
		{
			printf("File[%s]Line[%u]: file[%s] open failed!\r\n", __FILE__, __LINE__, pConfig->file_input);

		}
		
		fseek(pFile, 0, SEEK_SET);
		fread(pFileOrMsg, sizeof(char), iFileOrMsgLen , pFile);

		fclose(pFile);
		pFile = NULL;
	}
	else if(SEND_WORDS_TRANS_MODE == pConfig->trans_mode)//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï¢
	{
		printf("pConfig->message:[%s]\r\n", pConfig->message);
		strcpy(pFileOrMsg, pConfig->message);
		printf("GetFileOrMsg:[%s]\r\n", pFileOrMsg);
	}

	return 0;
}


//ÉèÖÃ»ù´¡ÐÅÏ¢
static int SetSendInfo(char *pInfo,  struct mosq_config *pConfig, int iSendMessBodyLen) {
	//ï¿½ï¿½ï¿½ï¿½ï¿½á¹¹ï¿½ï¿½
	int sendMessBody_len = sizeof(struct sendMessBody);

	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "*The jiegoutichangdu* :%d\r\n", sendMessBody_len);
	#endif
	
	int file_or_msg_len  = iSendMessBodyLen-sendMessBody_len;
	struct sendMessBody stSendMsg; //ï¿½ï¿½ï¿½Í½á¹¹ï¿½ï¿½
	memset(&stSendMsg, 0, sendMessBody_len);
	if(SetMessBody(&stSendMsg, pConfig) == 0) {
		printf("ï¿½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½É¹ï¿½!\n");
	};

	memcpy(pInfo, &stSendMsg, sendMessBody_len);
	//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï¢
	char *pFileOrMsg = NULL;
	if(file_or_msg_len > 0) {
		pFileOrMsg = (char *)malloc(file_or_msg_len);
		if(NULL == pFileOrMsg) {
			return MALLOC_FAILED_ERR;
		}
		memset(pFileOrMsg, 0, file_or_msg_len);
		int rc = 0;
		rc = GetFileOrMsg(pFileOrMsg, file_or_msg_len, pConfig);

		//Ê¹ï¿½ï¿½memcpyï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		memcpy(pInfo+sendMessBody_len, pFileOrMsg, file_or_msg_len);
		free(pFileOrMsg);
	}

	return 0;
}


//»ñÈ¡ÎÄ¼þ»òÕßÏûÏ¢µÄ³¤¶È
static int GetFileOrMsgLen(struct mosq_config *pConfig)
{
	int iFileOrMsgLen = -1;
	struct stat stFileStat;
	
	if(SEND_SOUNDS_TRANS_MODE == pConfig->trans_mode)//ï¿½ï¿½ï¿½ï¿½ï¿½Ä¼ï¿½
	{	
		if(stat(pConfig->file_input, &stFileStat) < 0)
		{
			return iFileOrMsgLen;
		}
		else
		{
			iFileOrMsgLen = stFileStat.st_size;
		}

	}

	else if(SEND_WORDS_TRANS_MODE == pConfig->trans_mode)//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï¢
	{
		iFileOrMsgLen = strlen(pConfig->message) + 1;
	}

	else {
		iFileOrMsgLen = 0;
	}

	return iFileOrMsgLen;
}


//½øÐÐ·¢ËÍÏûÏ¢
static int real_publish_msg(struct mosquitto *mosq, void *obj)
{
	printf("----pub_connect_callback!!!\r\n");
	int rc = 0;

	struct mosq_config *pMosqCfg = (struct mosq_config *)obj;
	printf("pMosqConfig->transMode:%d\r\n", pMosqCfg->trans_mode);

	if((pMosqCfg->trans_mode == SEND_SOUNDS_TRANS_MODE) 
		|| (pMosqCfg->trans_mode == SEND_WORDS_TRANS_MODE)
		|| (pMosqCfg->trans_mode == UP_Lines_TRANS_MODE)
		|| (pMosqCfg->trans_mode == DOWN_Lines_TRANS_MODE)
		|| (pMosqCfg->trans_mode == ADD_PRAISE_TRANS_MODE)
		|| (pMosqCfg->trans_mode == ADD_FRIENDS_MODE)
		|| (pMosqCfg->trans_mode == DEL_FRIENDS_MODE)) {
		//ï¿½ï¿½ï¿½ï¿½Ú·ï¿½ï¿½ï¿½Ä£Ê½ï¿½ï¿½
		int iSendMessBodyLen= 0; //ï¿½ï¿½ï¿½Í³ï¿½ï¿½ï¿½

		/* ï¿½ï¿½È¡ï¿½Ä¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä¼ï¿½ï¿½Ä³ï¿½ï¿½È£ï¿½Í¬Ê±ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½ */
		int msg_or_file_len = 0;
		msg_or_file_len = GetFileOrMsgLen(pMosqCfg);

		//ï¿½ï¿½È¡ï¿½Ü¹ï¿½ï¿½Ä·ï¿½ï¿½Í³ï¿½ï¿½ï¿½
		iSendMessBodyLen = msg_or_file_len + sizeof(struct sendMessBody);

		/* ï¿½ï¿½ä·¢ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ */
		char *pInfo = NULL;
		pInfo = (char *)malloc(iSendMessBodyLen);
		if(NULL == pInfo) {
			printf("Don't execute public--->: File[%s]Line[%u]: pInfo malloc failed!\r\n", __FILE__, __LINE__);
			return;
		}
		memset(pInfo, 0, iSendMessBodyLen);
		rc = SetSendInfo(pInfo, pMosqCfg, iSendMessBodyLen); //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½äº¯ï¿½ï¿½
		if(rc == 0) {
			printf("ï¿½ï¿½ï¿½Ðµï¿½ï¿½ï¿½ï¿½ï¿½×¼ï¿½ï¿½ï¿½ï¿½ï¿½!\n");
		}

		printf("The Len is:%d\n", iSendMessBodyLen);
		//ï¿½ï¿½ï¿½Í¸ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		int iMidSent = 0;
		rc = mosquitto_publish(mosq, &iMidSent, pMosqCfg->topic, iSendMessBodyLen, pInfo, pMosqCfg->qos, pMosqCfg->retain);
		
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "publish result[%d]\r\n", rc);
		printf("publish result[%d]\r\n", rc);
		if(NULL != pInfo) {
			free(pInfo);
			pInfo = NULL;
		}

		pMosqCfg = NULL;
		
	}

	return rc;
}

//»¹ÊÇÉèÖÃconfig½á¹¹Ìå
static int SetSendMosqConfig(struct mosq_config *pMosqConfig, carRadio_pub pub_para) {
	if(pub_para.chat_mode == 3) {
		pMosqConfig->toUserID = pub_para.toUserID;
	}

	//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ê±ï¿½ï¿½Òªï¿½ï¿½ï¿½ÃµÄ²ï¿½ï¿½ï¿½
	switch(pub_para.trans_mode) {
		case 1:			
			if( -1 == access(pub_para.file_path, 0) ) {
				#ifdef PRINT_ANDROID_INFO
					__android_log_print(ANDROID_LOG_INFO, "JNITag", "wenjianlujin :%s\r\n", pub_para.file_path);
				#endif
				return NO_FILE_EXIST;
			}else {
				pMosqConfig->file_input = pub_para.file_path;
				pMosqConfig->file_type  = pub_para.file_type;
				pMosqConfig->audio_play_time = pub_para.audio_play_time;
			}
			break;
		case 2:
			if(NULL == pub_para.message) {
				return NO_FILE_EXIST;
			}else {
				pMosqConfig->message = pub_para.message;
				pMosqConfig->msglen  = strlen(pub_para.message);
			}		
			break;
		case 3:
			//ï¿½ï¿½ï¿½ï¿½ï¿½Þ²ï¿½ï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½Òªï¿½ï¿½ï¿½ï¿½
			break;
		case 4:
			//ï¿½ï¿½ï¿½ï¿½ï¿½ß²ï¿½ï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½Òªï¿½ï¿½ï¿½ï¿½
			break;
		case 5:
			//ï¿½ï¿½ï¿½ï¿½ï¿½ß²ï¿½ï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½Òªï¿½ï¿½ï¿½ï¿½
			break;
		case 6:
			break;
		case 7:
			break;
		default:
			break;
			
	}

	return 0;
}


//·¢ËÍÏàÓ¦µÄÏûÏ¢
int pub_send_msg(JNIEnv *env, jobject obj, carRadio_pub pub_object) {
	int res = 0;

	//1.ÉêÇëÒ»¸öpub_configÓÃÓÚ´æ´¢
	struct mosq_config *pub_config = NULL;
	pub_config = malloc(sizeof(struct mosq_config));
	if(NULL == pub_config) {
		printf("File[%s]Line[%u]: struct mosq_config malloc failed!\r\n", __FILE__, __LINE__);
		return MALLOC_FAILED_ERR;	
	}
	memset(pub_config, 0, sizeof(struct mosq_config));

	//2.Ìî³äÉêÇëµÄpub_config
	res = init_base_pub_config(pub_config, pub_object);
	if(res != 0) {
		free(pub_config);
		return -1;
	}

	//3.ÉèÖÃ·¢ËÍµÄpub_config
	res = SetSendMosqConfig(pub_config, pub_object);
	if((res == NO_FILE_EXIST)) {
		free(pub_config);
		return res;
	}

	//ÔÚÕâÀï½øÐÐ·¢ËÍÏûÏ¢
	res = real_publish_msg(g_sub_mosq, pub_config);

	free(pub_config);
	pub_config = NULL;
	return res;
	
}



