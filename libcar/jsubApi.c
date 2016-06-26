#include <string.h>
#include <stdio.h>
#include <memory.h>

#include "jsubApi.h"
#include "client_shared.h"
#ifdef PRINT_ANDROID_INFO
#include <android/log.h>
#endif

extern int sub_unscribe_topic(JNIEnv *env, jobject obj, const char *oldtopic, const char *newtopic);
extern int sub_init_connect(JNIEnv *env, jobject obj, carRadio_sub sub_object);
extern int sub_enter_channel(JNIEnv *env, jobject obj, char * channel_topics, int qos);

// sub���ӷ�����
JNIEXPORT jint JNICALL Java_com_qichexiaozi_dtts_service_Connect_subObjectConnectServer
(JNIEnv *env, jobject obj, jobject obj_sub, jint topic_num, jobjectArray topicArray)
{
	int res = 0;
	int jni_err = 0;
	carRadio_sub st_sub_Interface;
	memset(&st_sub_Interface, 0, sizeof(carRadio_sub));
	
	jclass sub_cls = (*env)->GetObjectClass(env, obj_sub);
	if(sub_cls == NULL)
	{
		return JNI_GET_JAVA_OBJECT_ERR;
	}

	//��ȡtopic����
	int i = 0;
	memset(st_sub_Interface.topics, 0, sizeof(st_sub_Interface.topics));
	for(i = 0; i < topic_num; i++) {
		jstring js =(jstring)(*env)->GetObjectArrayElement(env, topicArray, i);
		const char* psz = (char*)(*env)->GetStringUTFChars(env, js, 0);
		if(psz == NULL) {
			return JNI_GET_JAVA_OBJECT_ERR;
		}
		sprintf(st_sub_Interface.topics[i], "%s", psz);
		#ifdef PRINT_ANDROID_INFO
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "joarr[%d]=%s", i, psz);
		#endif		
	}


	//��ȡ js(car_id��product_id��file_path��server_ip)
    jfieldID id = (*env)->GetFieldID(env, sub_cls,"id","Ljava/lang/String;"); 
	jstring js_id = (*env)->GetObjectField(env, obj_sub, id);

	jfieldID product_id_FieldID = (*env)->GetFieldID(env, sub_cls,"product_id","Ljava/lang/String;"); 
	jstring js_product_id = (*env)->GetObjectField(env, obj_sub, product_id_FieldID);

	jfieldID server_ip_FieldID = (*env)->GetFieldID(env, sub_cls,"server_ip","Ljava/lang/String;"); 
	jstring js_server_ip = (*env)->GetObjectField(env, obj_sub, server_ip_FieldID);
	
	jfieldID single_topic_FieldID = (*env)->GetFieldID(env, sub_cls,"topic","Ljava/lang/String;");
	jstring js_single_topic = (*env)->GetObjectField(env, obj_sub, single_topic_FieldID);

	jfieldID file_path_FieldID = (*env)->GetFieldID(env, sub_cls,"file_path","Ljava/lang/String;");
	jstring js_file_path = (*env)->GetObjectField(env, obj_sub, file_path_FieldID);


	if((js_id == NULL) || (js_product_id == NULL) 
		|| (js_server_ip == NULL) || (js_single_topic == NULL) || (js_file_path == NULL))
	{
		return JNI_INPUT_NULL_STRING_ERR;
	}
		
	//js�ַ���ת��char�ַ���
	const char *uuid      = (*env)->GetStringUTFChars(env, js_id, NULL);
	const char *product_id  = (*env)->GetStringUTFChars(env, js_product_id, NULL);
	const char *server_ip   = (*env)->GetStringUTFChars(env, js_server_ip, NULL);
	const char *single_topic   = (*env)->GetStringUTFChars(env, js_single_topic, NULL);
	const char *file_path  = (*env)->GetStringUTFChars(env, js_file_path, NULL);

	if((uuid == NULL) || (product_id == NULL) 
		|| (server_ip == NULL) || (single_topic == NULL) || (file_path == NULL))
	{
		jni_err = JNI_GET_STRING_ERR;
		goto SUB_OBJECT_CONNECT_ERR;
	}	
	
	//��ȡint��Ա
	jfieldID client_type_FieldID = (*env)->GetFieldID(env, sub_cls,"client_type","I"); 
	jint client_type = (*env)->GetIntField(env, obj_sub, client_type_FieldID);
	
	jfieldID product_type_FieldID = (*env)->GetFieldID(env, sub_cls,"product_type","I"); 
	jint product_type = (*env)->GetIntField(env, obj_sub, product_type_FieldID);
	
	jfieldID qos_FieldID = (*env)->GetFieldID(env, sub_cls,"qos","I"); 
	jint qos = (*env)->GetIntField(env, obj_sub, qos_FieldID);

	jfieldID keepalive_FieldID = (*env)->GetFieldID(env, sub_cls,"keepalive","I"); 
	jint keepalive = (*env)->GetIntField(env, obj_sub, keepalive_FieldID);

	jfieldID server_port_FieldID = (*env)->GetFieldID(env, sub_cls,"server_port","I"); 
	jint server_port = (*env)->GetIntField(env, obj_sub, server_port_FieldID);
	
	//��ȡbool��Ա
	jfieldID clean_session_FieldID = (*env)->GetFieldID(env, sub_cls, "clean_session", "Z");
	jboolean clean_session = (*env)->GetBooleanField(env, obj_sub, clean_session_FieldID);

	jfieldID quiet_FieldID = (*env)->GetFieldID(env, sub_cls, "quiet", "Z"); 
	jboolean quiet = (*env)->GetBooleanField(env, obj_sub, quiet_FieldID);


	//设置st_sub_Interface
	st_sub_Interface.uuid = (char *)uuid;
	st_sub_Interface.product_id = (char *)product_id;
	st_sub_Interface.server_ip = (char *)server_ip;
	st_sub_Interface.topic = (char *)single_topic;
	st_sub_Interface.file_path = (char *)file_path;

	st_sub_Interface.client_type = client_type;
	st_sub_Interface.server_port = server_port;
	st_sub_Interface.product_type = product_type;
	st_sub_Interface.keepalive = keepalive;

	st_sub_Interface.qos  = qos;
	st_sub_Interface.clean_session = clean_session;
	st_sub_Interface.quiet = quiet;

	st_sub_Interface.topic_num = topic_num;


	//����C�ӿں���
	res = sub_init_connect(env, obj, st_sub_Interface);
	//res = sub_connect_cns(env, obj, st_sub_Interface);
#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "SubJni: sub_connect_cns_new[res = %d]\r\n",res);
#endif

// �ͷ�jni��Դ
SUB_OBJECT_CONNECT_ERR:
	if(id != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_id, uuid);
	}
	if(product_id != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_product_id, product_id);
	}
	if(server_ip != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_server_ip, server_ip);
	}
	if(single_topic != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_single_topic, single_topic);
	}

	if(jni_err > 0)
	{
		return jni_err;
	}
	else
	{	
		return res;
	}

}


JNIEXPORT jint JNICALL Java_com_qichexiaozi_dtts_service_Connect_subChangeTopic
(JNIEnv *env, jobject obj, jstring old_topic, jstring new_topic) {
	int res = 0;

	if((NULL == old_topic) || (NULL == new_topic)) 
	{
		return JNI_INPUT_NULL_STRING_ERR;
	}

	
	const char *cold_topic = (*env)->GetStringUTFChars(env, old_topic, NULL);
	if(NULL == cold_topic)
	{
		return JNI_GET_STRING_ERR;
	}
	const char *cnew_topic = (*env)->GetStringUTFChars(env, new_topic, NULL);
	if(NULL == cnew_topic)
	{
		goto SUBSETMSGPARA_1;
	}

	//����c����--����sub����
	res = sub_unscribe_topic(env, obj, cold_topic, cnew_topic);

	(*env)->ReleaseStringUTFChars(env, old_topic, cold_topic);
	(*env)->ReleaseStringUTFChars(env, new_topic, cnew_topic);
	
	return res;
	
SUBSETMSGPARA_1:
	(*env)->ReleaseStringUTFChars(env, old_topic, cold_topic);
	(*env)->ReleaseStringUTFChars(env, new_topic, cnew_topic);
	return JNI_GET_STRING_ERR;
}



JNIEXPORT jint JNICALL Java_com_qichexiaozi_dtts_service_Connect_subEnterChannel
(JNIEnv *env, jobject obj, jstring channel_topic, jint qos) {
	int res = 0;

	if((NULL == channel_topic)) 
	{
		return JNI_INPUT_NULL_STRING_ERR;
	}

	const char *cchannel_topic = (*env)->GetStringUTFChars(env, channel_topic, NULL);
	if(NULL == cchannel_topic)
	{
		goto SUBSETMSGPARA_1;
	}

	
#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "cchannel_topic:%s\r\n", cchannel_topic);
#endif

	//����c����--����sub����
	res = sub_enter_channel(env, obj, (char *)cchannel_topic, qos);

	(*env)->ReleaseStringUTFChars(env, channel_topic, cchannel_topic);
	
	return res;
	
SUBSETMSGPARA_1:
	(*env)->ReleaseStringUTFChars(env, channel_topic, cchannel_topic);
	return JNI_GET_STRING_ERR;
}


JNIEXPORT jint JNICALL Java_com_qichexiaozi_dtts_service_Connect_subQuitChannel
(JNIEnv *env, jobject obj, jstring channel_topic) {
	//sub_quit_channel("1/2");


	int res = 0;

	if((NULL == channel_topic)) 
	{
		return JNI_INPUT_NULL_STRING_ERR;
	}

	
	const char *cchannel_topic = (*env)->GetStringUTFChars(env, channel_topic, NULL);
	if(NULL == cchannel_topic)
	{
		goto SUBSETMSGPARA_1;
	}

	//����c����--����sub����
	res = sub_quit_channel(env, obj, cchannel_topic);

	
	(*env)->ReleaseStringUTFChars(env, channel_topic, cchannel_topic);
	
	return res;
	
SUBSETMSGPARA_1:
	(*env)->ReleaseStringUTFChars(env, channel_topic, cchannel_topic);
	return JNI_GET_STRING_ERR;
}


//pub����������Ϣ, ����, so on
JNIEXPORT jint JNICALL Java_com_qichexiaozi_dtts_service_Connect_pubObjectSendMsg
(JNIEnv *env, jobject obj, jobject obj_pub)
{

	int res = 0;
	int jni_err = 0;
	carRadio_pub st_pub_Interface;
	memset(&st_pub_Interface, 0, sizeof(carRadio_pub));

	jclass pub_cls = (*env)->GetObjectClass(env, obj_pub);
	if(pub_cls == NULL)
	{
		return JNI_GET_JAVA_OBJECT_ERR;
	}

	/* ��ʼ��һЩ���� */
	jfieldID file_path_FieldID;
	jfieldID file_type_FieldID;
	jfieldID audio_play_time_FieldID;
	jfieldID message_FieldID;
	jstring js_file_path = NULL;
	char *file_path = NULL;
	jstring js_message = NULL;
	char *message = NULL;
	jint file_type = 0;
	jint audio_play_time = 0;

	//�����ж�transmode
	jfieldID trans_mode_FieldID = (*env)->GetFieldID(env, pub_cls,"trans_mode","I"); 
	jint trans_mode = (*env)->GetIntField(env, obj_pub, trans_mode_FieldID);

	switch (trans_mode) {
		case 1:
			//�ļ�
			file_path_FieldID = (*env)->GetFieldID(env, pub_cls,"file_path","Ljava/lang/String;"); 
			js_file_path = (*env)->GetObjectField(env, obj_pub, file_path_FieldID);
			file_path    = (char *)(*env)->GetStringUTFChars(env, js_file_path, NULL);
			if(NULL == file_path) {
				return JNI_INPUT_NULL_STRING_ERR;
			}
			#ifdef PRINT_ANDROID_INFO
				__android_log_print(ANDROID_LOG_INFO, "JNITag", " filePath--- > :%s\r\n", file_path);
			#endif

			file_type_FieldID = (*env)->GetFieldID(env, pub_cls,"file_type","I"); 
			file_type = (*env)->GetIntField(env, obj_pub, file_type_FieldID);

			audio_play_time_FieldID = (*env)->GetFieldID(env, pub_cls,"audio_play_time","I"); 
			audio_play_time = (*env)->GetIntField(env, obj_pub, audio_play_time_FieldID);
			break;
		case 2:
			//��������
			message_FieldID = (*env)->GetFieldID(env, pub_cls,"message","Ljava/lang/String;"); 
			js_message = (*env)->GetObjectField(env, obj_pub, message_FieldID);
			message  = (char *)(*env)->GetStringUTFChars(env, js_message, NULL);
			break;
	}
		
	//��ȡ�ַ�����Ա
	jfieldID uuid_FieldID = (*env)->GetFieldID(env, pub_cls,"id","Ljava/lang/String;"); 
	jstring js_uuid = (*env)->GetObjectField(env, obj_pub, uuid_FieldID);

	jfieldID user_name_FieldID = (*env)->GetFieldID(env, pub_cls,"user_name","Ljava/lang/String;"); 
	jstring js_user_name = (*env)->GetObjectField(env, obj_pub, user_name_FieldID);

	jfieldID channel_id_FieldID = (*env)->GetFieldID(env, pub_cls,"channel_id","Ljava/lang/String;"); 
	jstring js_channel_id = (*env)->GetObjectField(env, obj_pub, channel_id_FieldID);

	jfieldID portrait_path_FieldID = (*env)->GetFieldID(env, pub_cls,"portrait_path","Ljava/lang/String;"); 
	jstring js_portrait_path = (*env)->GetObjectField(env, obj_pub, portrait_path_FieldID);

	jfieldID server_ip_FieldID = (*env)->GetFieldID(env, pub_cls,"server_ip","Ljava/lang/String;"); 
	jstring js_server_ip = (*env)->GetObjectField(env, obj_pub, server_ip_FieldID);

	jfieldID topic_FieldID = (*env)->GetFieldID(env, pub_cls,"topic","Ljava/lang/String;"); 
	jstring js_topic = (*env)->GetObjectField(env, obj_pub, topic_FieldID);
	
	jfieldID product_id_FieldID = (*env)->GetFieldID(env, pub_cls,"product_id","Ljava/lang/String;"); 
	jstring js_product_id = (*env)->GetObjectField(env, obj_pub, product_id_FieldID);

	jfieldID brand_FieldID = (*env)->GetFieldID(env, pub_cls,"brand","Ljava/lang/String;"); 
	jstring js_brand = (*env)->GetObjectField(env, obj_pub, brand_FieldID);

	jfieldID phone_type_FieldID = (*env)->GetFieldID(env, pub_cls,"phone_type","Ljava/lang/String;"); 
	jstring js_phone_type = (*env)->GetObjectField(env, obj_pub, phone_type_FieldID);


	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "SubJni: sub_connect_cns_new[res = ]\r\n");
	#endif

	if((js_uuid == NULL) || (js_product_id == NULL) || (js_server_ip == NULL) 
		|| (js_topic == NULL) || (js_channel_id == NULL) || (js_brand == NULL) 
		|| (js_phone_type == NULL)|| (js_portrait_path == NULL) || (js_user_name == NULL))
	{
		return JNI_INPUT_NULL_STRING_ERR;
	}

	//---js�ַ���ת��char�ַ���
	const char *uuid      = (*env)->GetStringUTFChars(env, js_uuid, NULL);
	const char *user_name = (*env)->GetStringUTFChars(env, js_user_name, NULL);
	const char *product_id  = (*env)->GetStringUTFChars(env, js_product_id, NULL);
	const char *server_ip   = (*env)->GetStringUTFChars(env, js_server_ip, NULL);
	const char *topic       = (*env)->GetStringUTFChars(env, js_topic, NULL);
	
	const char *channel_id  = (*env)->GetStringUTFChars(env, js_channel_id, NULL);
	const char *brand        = (*env)->GetStringUTFChars(env, js_brand, NULL);
	const char *phone_type   = (*env)->GetStringUTFChars(env, js_phone_type, NULL);
	const char *portrait_path   = (*env)->GetStringUTFChars(env, js_portrait_path, NULL);

	#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " --- > :%s\r\n", uuid);
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " --- > :%s\r\n", user_name);
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " --- > :%s\r\n", product_id);
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " --- > :%s\r\n", server_ip);
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " --- > :%s\r\n", topic);
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " --- > :%s\r\n", channel_id);
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " --- > :%s\r\n", brand);
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " --- > :%s\r\n", phone_type);
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " --- > :%s\r\n", portrait_path);
	#endif

	if((uuid == NULL) || (product_id == NULL) || (server_ip == NULL) || (topic == NULL)
		|| (channel_id == NULL) || (brand == NULL) 
		|| (phone_type == NULL) || (portrait_path == NULL) || (user_name == NULL))
	{
		jni_err = JNI_GET_STRING_ERR;
		goto PUB_OBJECT_SEND_AUDIO_ERR;
	}	

	//��ȡint��Ա (������Դʵ�ķ�ʽ����ѭ����ȡ�������Ż�)
	jfieldID client_type_FieldID = (*env)->GetFieldID(env, pub_cls, "client_type","I"); 
	jint client_type = (*env)->GetIntField(env, obj_pub, client_type_FieldID);

#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", " int :%d\r\n", client_type);
#endif

	jfieldID chat_mode_FieldID = (*env)->GetFieldID(env, pub_cls, "chat_mode","I"); 
	jint chat_mode = (*env)->GetIntField(env, obj_pub, chat_mode_FieldID);
	
	jfieldID user_type_FieldID = (*env)->GetFieldID(env, pub_cls, "user_type","I"); 
	jint user_type = (*env)->GetIntField(env, obj_pub, user_type_FieldID);

#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " int :%d\r\n", user_type);
#endif

	jfieldID qos_FieldID = (*env)->GetFieldID(env, pub_cls, "qos","I"); 
	jint qos = (*env)->GetIntField(env, obj_pub, qos_FieldID);

#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " int :%d\r\n", qos);
#endif
	
	jfieldID server_port_FieldID = (*env)->GetFieldID(env, pub_cls, "server_port","I"); 
	jint server_port = (*env)->GetIntField(env, obj_pub, server_port_FieldID);

#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " int :%d\r\n", server_port);
#endif

	jfieldID sender_type_FieldID = (*env)->GetFieldID(env, pub_cls, "sender_type","I"); 
	jint sender_type = (*env)->GetIntField(env, obj_pub, sender_type_FieldID);

#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " int :%d\r\n", sender_type);
#endif

	jfieldID product_type_FieldID = (*env)->GetFieldID(env, pub_cls, "product_type","I"); 
	jint product_type = (*env)->GetIntField(env, obj_pub, product_type_FieldID);

#ifdef PRINT_ANDROID_INFO
		__android_log_print(ANDROID_LOG_INFO, "JNITag", " int :%d\r\n", product_type);
#endif

	//��ȡbool��Ա
	jfieldID quiet_FieldID = (*env)->GetFieldID(env, pub_cls, "quiet", "Z");  
	jboolean quiet = (*env)->GetBooleanField(env, obj_pub, quiet_FieldID);

	jfieldID retain_FieldID = (*env)->GetFieldID(env, pub_cls, "retain", "Z");  
	jboolean retain = (*env)->GetBooleanField(env, obj_pub, retain_FieldID);

	//�ж���ʲôģʽ
	char *toUserID  = NULL;
	jstring js_toUserID   = NULL;
	if(chat_mode == 3) {
		/* ���ǵ�Ե������ģʽ */
		jfieldID toUserID_FieldID = (*env)->GetFieldID(env, pub_cls, "toUserID","Ljava/lang/String;"); 
		js_toUserID = (*env)->GetObjectField(env, obj_pub, toUserID_FieldID);
		if(js_toUserID == NULL) {
			return JNI_GET_STRING_ERR;
		}
		toUserID      = (*env)->GetStringUTFChars(env, js_toUserID, NULL);
		st_pub_Interface.toUserID = toUserID;
	}


   //��ֵstPubFilePara����st_pub_Interface
   	st_pub_Interface.uuid = (char *)uuid;
   	st_pub_Interface.user_name = (char *)user_name;
	st_pub_Interface.product_id = (char *)product_id;
	st_pub_Interface.server_ip = (char *)server_ip;
	st_pub_Interface.topic = (char *)topic;
	st_pub_Interface.channel_id= (char *)channel_id;
	
	st_pub_Interface.brand = (char *)brand;
	st_pub_Interface.phone_type = (char *)phone_type;
	st_pub_Interface.portrait_path = (char *)portrait_path;

	st_pub_Interface.client_type = client_type;
	st_pub_Interface.user_type   = user_type;
	st_pub_Interface.server_port = server_port;
	st_pub_Interface.product_type = product_type;
	st_pub_Interface.sender_type = sender_type;
	st_pub_Interface.qos = qos;

	st_pub_Interface.chat_mode  = chat_mode;
	st_pub_Interface.trans_mode = trans_mode;
	st_pub_Interface.quiet = quiet;
	st_pub_Interface.retain = retain;

	if(SEND_SOUNDS_TRANS_MODE == trans_mode) {
		st_pub_Interface.file_path = (char *)file_path;
		st_pub_Interface.audio_play_time = audio_play_time;
		st_pub_Interface.file_type = file_type;
	}else if(SEND_WORDS_TRANS_MODE == trans_mode) {
		st_pub_Interface.message = (char *)message;
	}
	


//����C�ӿں���
	res = pub_send_msg(env, obj, st_pub_Interface);

#ifdef PRINT_ANDROID_INFO
	__android_log_print(ANDROID_LOG_INFO, "JNITag", " message return :%d\n", res);
#endif

	//res = pub_send_audio(env, obj, st_pub_Interface);

// �ͷ�jni��Դ
PUB_OBJECT_SEND_AUDIO_ERR:
	if(uuid != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_uuid, uuid);
	}
	if(product_id != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_product_id, product_id);
	}
	if(server_ip != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_server_ip, server_ip);
	}
		
    if(topic != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_topic, topic);
	}
	if(file_path != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_file_path, file_path);
	}
	if(channel_id != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_channel_id, channel_id);
	}

	if(brand != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_brand, brand);
	}
	if(phone_type != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_phone_type, phone_type);
	}
	if(portrait_path != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_portrait_path, portrait_path);
	}

	if(toUserID != NULL)
	{
		(*env)->ReleaseStringUTFChars(env, js_toUserID, toUserID);
	}
	
	if(jni_err > 0)
	{
		return jni_err;
	}
	else
	{	
		return res;
	}

}


