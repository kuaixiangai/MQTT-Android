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
#include <stdbool.h>
#include <sys/stat.h>

#include "jni.h"
#include <android/log.h>
#include <mosquitto.h>
#include "client_shared.h"

#define STATUS_CONNECTING 0
#define STATUS_CONNACK_RECVD 1
#define STATUS_WAITING 2

/* Global variables for use in callbacks. See sub_client.c for an example of
 * using a struct to hold variables for use in callbacks. */


//���ļ��������и�--Ŀǰ�ȱ���
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


//����: ��ô����͵��ļ���Υ����Ϣ����
//����:
//    char *pInfo:                    ����ִ����֮��pInfo��ô����͵���Ϣ
//    struct mosq_config * pConfig:   ����pConfig��ȡ��Ӧ����Ϣ����
//����ֵ:
//    int-�����͵��ļ���Υ����Ϣ����
static int GetFileOrMsgLen(struct mosq_config *pConfig)
{
	int iFileOrMsgLen = -1;
	struct stat stFileStat;
	
	if(SEND_SOUNDS_TRANS_MODE == pConfig->trans_mode)//�����ļ�
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

	else if(SEND_WORDS_TRANS_MODE == pConfig->trans_mode)//������Ϣ
	{
		iFileOrMsgLen = strlen(pConfig->message) + 1;
	}

	else {
		iFileOrMsgLen = 0;
	}

	return iFileOrMsgLen;
}


//����: ��ô����͵��ļ���Υ����Ϣ
//����:
//    char *pFileOrMsg:               �����͵��ļ���Υ����Ϣ
//    int *pInfoLen:                  �����͵��ļ���Υ����Ϣ����
//    struct mosq_config * pConfig:   ����pConfig���pFileOrMsg
//����ֵ:
//    0-��ȷ, ����-���ش�����
static int GetFileOrMsg(char *pFileOrMsg, int iFileOrMsgLen, struct mosq_config *pConfig)
{	
	if(SEND_SOUNDS_TRANS_MODE == pConfig->trans_mode)//�����ļ�
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
	else if(SEND_WORDS_TRANS_MODE == pConfig->trans_mode)//������Ϣ
	{
		printf("pConfig->message:[%s]\r\n", pConfig->message);
		strcpy(pFileOrMsg, pConfig->message);
		printf("GetFileOrMsg:[%s]\r\n", pFileOrMsg);
	}

	return 0;
}


//����: ����struct sendMessBody
//����:
//    struct sendMessBody *pMsg:      �����õ�struct sendMessBody����pMsg
//    struct mosq_config * pConfig:   ����pConfig����pMsg
//����ֵ:
//    0-��ȷ��1-����
static int SetMessBody(struct sendMessBody *pMsg, struct mosq_config * pConfig)
{
	/* ��乫���ֶ� */
	strcpy(pMsg->id, pConfig->id);	
	strcpy(pMsg->phone_type, pConfig->phone_type);				/* �ֻ�ϵͳ*/
	strcpy(pMsg->product_id, pConfig->product_id);
	strcpy(pMsg->channel_id, pConfig->channel_id);
	strcpy(pMsg->brand, pConfig->brand);						/* �ֻ�Ʒ��*/
	strcpy(pMsg->user_name, pConfig->user_name);
	strcpy(pMsg->portrait_path, pConfig->portrait_path);		/* �û�ͷ��·�� */
	pMsg->userType = pConfig->userType; 	//�Ƿ�ע���¼��
	pMsg->sender_type = pConfig->sender_type;
	pMsg->trans_mode = pConfig->trans_mode;
	pMsg->product_type = pConfig->product_type;

	char *pVoicePath = NULL;
	switch(pConfig->trans_mode){
		case 1:
			pVoicePath = split_filename(pConfig->file_input);
			strcpy(pMsg->voice_name, pVoicePath);
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
		default:
			break;
	}

	//��ӡ��Ϣ
	printf("sendMessBody->voice_name: %s\r\n", pMsg->voice_name);
	printf("sendMessBody->carid: %s\r\n", pMsg->id);
	printf("sendMessBody->product_id: %s\r\n", pMsg->product_id);
	return 0;
}

//��������䷢�͵���Ϣ��
static int SetSendInfo(char *pInfo,  struct mosq_config *pConfig, int iSendMessBodyLen) {
	//�����ṹ��
	int sendMessBody_len = sizeof(struct sendMessBody);
	int file_or_msg_len  = iSendMessBodyLen-sendMessBody_len;
	struct sendMessBody stSendMsg; //���ͽṹ��
	memset(&stSendMsg, 0, sendMessBody_len);
	if(SetMessBody(&stSendMsg, pConfig) == 0) {
		printf("�ṹ�����ɹ�!\n");
	};

	memcpy(pInfo, &stSendMsg, sendMessBody_len);
	//������ļ�������Ϣ
	char *pFileOrMsg = NULL;
	if(file_or_msg_len > 0) {
		pFileOrMsg = (char *)malloc(file_or_msg_len);
		if(NULL == pFileOrMsg) {
			return MALLOC_FAILED_ERR;
		}
		memset(pFileOrMsg, 0, file_or_msg_len);
		int rc = 0;
		rc = GetFileOrMsg(pFileOrMsg, file_or_msg_len, pConfig);

		//ʹ��memcpy�������
		memcpy(pInfo+sendMessBody_len, pFileOrMsg, file_or_msg_len);
		free(pFileOrMsg);
	}

	return 0;
}

//����: �Ͽ����Ӳ����Ļص�����
//����:
//    struct mosquitto *mosq:      ׼���Ͽ����ӵ�struct mosquitto����
//    void             *obj:       ��ʱ����
//    int              result:     �ص�������Ĭ�ϲ�������ʱ����
//����ֵ:  ��
static void my_disconnect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("----pub disconnect!!!\n");	
	struct mosq_config *pMosqCfg = (struct mosq_config *)obj;
	pMosqCfg->connected = false;
}


//����: publish �Ļص�����
//����:
//    struct mosquitto *mosq:      ���ӵ�struct mosquitto����
//    void             *obj:       ��ʱ����
//    int              result:     �ص�������Ĭ�ϲ�������ʱ����
//����ֵ:  ��
static void my_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
	/* publish��ɽ��лص� */

	printf("I Have Publish!\r\n");
	mosquitto_disconnect(mosq);
}

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	printf("%s\n", str);
}

//����: ���Ӳ����Ļص�����
//����:
//    struct mosquitto *mosq:      ����ʱ��struct mosquitto����
//    void             *obj:      ��ʱ����
//    int              result:     �ص�������Ĭ�ϲ�������ʱ����
//����ֵ:  ��
static void my_pub_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("----pub_connect_callback!!!\r\n");
	int rc = 0;

	struct mosq_config *pMosqCfg = (struct mosq_config *)obj;
	printf("pMosqConfig->transMode:%d\r\n", pMosqCfg->trans_mode);

	if((pMosqCfg->trans_mode == SEND_SOUNDS_TRANS_MODE) 
		|| (pMosqCfg->trans_mode == SEND_WORDS_TRANS_MODE)
		|| (pMosqCfg->trans_mode == UP_Lines_TRANS_MODE)
		|| (pMosqCfg->trans_mode == DOWN_Lines_TRANS_MODE)
		|| (pMosqCfg->trans_mode == ADD_PRAISE_TRANS_MODE)) {
		//����ڷ���ģʽ��
		int iSendMessBodyLen= 0; //���ͳ���

		/* ��ȡ�ļ������ļ��ĳ��ȣ�ͬʱ��ȡ���� */
		int msg_or_file_len = 0;
		msg_or_file_len = GetFileOrMsgLen(pMosqCfg);

		//��ȡ�ܹ��ķ��ͳ���
		iSendMessBodyLen = msg_or_file_len + sizeof(struct sendMessBody);

		/* ��䷢������ */
		char *pInfo = NULL;
		pInfo = (char *)malloc(iSendMessBodyLen);
		if(NULL == pInfo) {
			printf("Don't execute public--->: File[%s]Line[%u]: pInfo malloc failed!\r\n", __FILE__, __LINE__);
			return;
		}
		memset(pInfo, 0, iSendMessBodyLen);
		rc = SetSendInfo(pInfo, pMosqCfg, iSendMessBodyLen); //��������亯��
		if(rc == 0) {
			printf("���е�����׼�����!\n");
		}

		printf("The Len is:%d\n", iSendMessBodyLen);
		//���͸�������
		int iMidSent = 0;
		rc = mosquitto_publish(mosq, &iMidSent, pMosqCfg->topic, iSendMessBodyLen, pInfo, pMosqCfg->qos, pMosqCfg->retain);
		printf("publish result[%d]\r\n", rc);
		if(NULL != pInfo) {
			free(pInfo);
			pInfo = NULL;
		}

		pMosqCfg = NULL;
		
	}

	return;
}

static int SetSendMosqConfig(struct mosq_config *pMosqConfig, carRadio_pub pub_para) {
	//��������ʱ��Ҫ���õĲ���
	switch(pub_para.trans_mode) {
		case 1:			
			if( -1 == access(pub_para.file_path, 0) ) {
				__android_log_print(ANDROID_LOG_INFO, "JNITag", "wenjianlujin :%s\r\n", pub_para.file_path);
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
			//�����޲���ʱ����Ҫ����
			break;
		case 4:
			//�����߲���ʱ����Ҫ����
			break;
		case 5:
			//�����߲���ʱ����Ҫ����
			break;
		default:
			break;
			
	}

	return 0;
}

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

	return 0;
	
}

static int close_conversation(struct mosq_config *pub_config, struct mosquitto *pMosq) {
	free(pub_config);
	pub_config = NULL;

	mosquitto_destroy(pMosq);
	pMosq = NULL;
	mosquitto_lib_cleanup();

	return 0;
}

//������Ϣ����
int pub_send_msg(JNIEnv *env, jobject obj, carRadio_pub pub_object) {
	int res = 0;

	//1.�����ڴ�Դ��ݹ����Ĳ������б���
	struct mosq_config *pub_config = NULL;
	pub_config = malloc(sizeof(struct mosq_config));
	if(NULL == pub_config) {
		printf("File[%s]Line[%u]: struct mosq_config malloc failed!\r\n", __FILE__, __LINE__);
		return MALLOC_FAILED_ERR;	
	}
	memset(pub_config, 0, sizeof(struct mosq_config));

	//2.����������
	init_base_pub_config(pub_config, pub_object);


	//3.���ݷ��Ͳ�ͬ���ݽ��в�ͬ������
	res = SetSendMosqConfig(pub_config, pub_object);
	if((res == NO_FILE_EXIST)) {
		free(pub_config);
		return res;
	}


	//4.����mosquitto����pMosq
	struct mosquitto *pMosq = NULL;
	mosquitto_max_inflight_messages_set(pMosq, pub_config->max_inflight);
	mosquitto_opts_set(pMosq, MOSQ_OPT_PROTOCOL_VERSION, &(pub_config->protocol_version));
	pMosq = mosquitto_new(pub_config->id, true, pub_config);

	//5.���ûص�����
	mosquitto_connect_callback_set(pMosq, my_pub_connect_callback);
	mosquitto_disconnect_callback_set(pMosq, my_disconnect_callback);
	mosquitto_publish_callback_set(pMosq, my_publish_callback);
	

	res = client_connect(pMosq, pub_config);
	if(0 != res) {
		printf("File[%s]Line[%u]: client_connect failed! ErrCode[%d]\r\n", __FILE__, __LINE__, res);
		close_conversation(pub_config, pMosq);
		return CONNECT_FAILED_ERR;
	}

	do{
		res = mosquitto_loop(pMosq, -1, 1);
		printf("Current rc:%d\r\n", res);
	}while(res == MOSQ_ERR_SUCCESS && pub_config->connected);
	
	close_conversation(pub_config, pMosq);
	return 0;
	
}

