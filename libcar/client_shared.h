/*
Copyright (c) 2014 Roger Light <roger@atchoo.org>

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

#ifndef _CLIENT_CONFIG_H
#define _CLIENT_CONFIG_H

#include <stdio.h>
#include <stdbool.h>
#include "jni.h"

/* pub_client.c modes */
#define MSGMODE_NONE 0
#define MSGMODE_CMD 1
#define MSGMODE_STDIN_LINE 2
#define MSGMODE_STDIN_FILE 3
#define MSGMODE_FILE 4
#define MSGMODE_NULL 5

//设置pub的发送模式
#define SEND_SOUNDS_TRANS_MODE 1 //发送语音
#define SEND_WORDS_TRANS_MODE 2  //发送文字
#define ADD_PRAISE_TRANS_MODE 3  //发送点赞命令
#define UP_Lines_TRANS_MODE 4  //发送上线命令
#define DOWN_Lines_TRANS_MODE 5  //发送上线命令
#define ADD_FRIENDS_MODE 6
#define DEL_FRIENDS_MODE 7

#define CLIENT_PUB 1
#define CLIENT_SUB 2

/* 函数返回值 */
#define SUCCESS_RESULT                 (0)     /* 结果正确 */
#define MALLOC_FAILED_ERR              (101)  /* malloc开辟内存失败错误 */
#define CONNECT_FAILED_ERR             (102)  /* 错误: 连接失败 */
#define OPEN_FILE_ERR                  (103)   /* 错误: 文件打开失败 */
#define NO_FILE_EXIST                  (104)  /* 错误: 文件不存在 */
#define SERVER_IP_OR_PORT_ERR          (105)  /* 错误: 无效的服务器IP或端口 */
#define JNI_MALLOC_FAILED_ERR          (106)  /* 错误: jni层开内存失败 */
#define JNI_GET_STRING_ERR             (107)  /* 错误: jni字符串转换失败 */
#define JNI_INPUT_NULL_STRING_ERR     (108)  /* 错误: jni输入字符串为空 */
#define JNI_GET_METHOD_ID_ERR          (109)  /* 错误: jni函数GetMethodID调用失败 */
#define JNI_THROW_ERR                   (110)  /* 错误: jni函数调用发生异常 */
#define JNI_GET_JAVA_OBJECT_ERR        (111)  /* 错误: jni函数获得接口java对象(GetObjectClass)失败 */
#define JNI_NULL_VALUE                  (112)  /* 错误: jni的env、object、class为NULL */
#define INPUT_ILLEGAL_PARA             (113)  /* 错误: 给函数传入非法的输入参数 */
#define TOPIC_ILLEGAL_ERR             (114)  /* 错误: topic非法 */
#define JNI_NULL_FUNCTION             (115)  /* 错误: 无效函数，函数删除不存在 */
#define CHANGE_TOPIC_ERR              (116)  /* 错误: 改变topic失败:添加时发现旧的topic未delete,或者删除时发现没有topic */

#define MAX_SERVER_PORT         (65535)       /* 服务器端口最大值 */
#define MIN_SERVER_PORT         (1024)            /* 服务器端口最小值 */

#define MAX_KEEPALIVE_VALUE     (3600)  /* mosquitto心跳参数最大值 */
#define MIN_KEEPALIVE_VALUE     (0)       /* mosquitto心跳参数最小值 */
#define DEF_KEEPALIVE_VALUE     (60)     /* mosquitto心跳参数默认值 */

/* pub���͵Ľṹ�� */
struct sendMessBody {
//	char  voice_name[20];  /* �������� */
	char  product_id[20];  /*  */
	char  id[48];        /* 车辆唯一识别号 */
	char  user_name[16];
	short int   userType;      //标识用户是否注册过:0表示未注册 1表示注册了
	short int   product_type;    /* 产品类型 */
	short int   sender_type;       /* 发送类型 */
	short int   chat_mode;
	short int   trans_mode;       /* 传输模式 :4-文件;3-消息*/
	char  toUserID[48];
//	char  toUser_id[32];   /* 用户ID */
	short int   audio_play_time;  /* 音频文件播放时长 */

	char  channel_id[11];  /* 频道ID */
//	char  car_plate_number[16]; /* 车牌号 */
	char  brand[16];             /* 手机品牌(华为\小米\三星等等) */
	char  phone_type[16];       /* 手机系统(android4.4.4\android5.0.2) */
	char  portrait_path[80];    /* 用户头像路径 */
	
	short int  audio_create_time;  /* 语音创建的时间 */
	short int  file_type;         //文件类型:音频(mp3、amr等等)
	short int  online_status;     //用户上线或下线状态
} __attribute__ ((aligned (4)));


/* sub结构体的定义 */
typedef struct subObject {
	char *uuid; //用户的唯一标识
	int client_type;
	char *server_ip;
	int  server_port;
	int  keepalive;
	int  qos;
	bool clean_session;
	bool quiet;
	
	/* 用户订阅的topic */
	int topic_num; //topicĬ�������������ĸ�
	char *topic;
	char topics[4][64];
	
	/* 产品类型和产品ID */
	int  product_type;
	char *product_id;
	
	/* 用户接收到的语音文件存放的路径 */
	char *file_path;
}carRadio_sub;

/* pub结构体的定义 */
typedef struct pubObject {
	/* 用户信息 */
	char *uuid;        //用户的唯一标识
	char *user_name;   //只有当用户注册登录以后才会出现
	int  client_type;  //1 //1代表pub端, 2代表sub端
	char *channel_id;  //用户当前所在频道
	int  user_type;     //标识用户是否注册过:0表示未注册 1表示注册了
	char *portrait_path; //用户头像地址(如果注册过，则是网络地址,否则是本地地址)

	/* 此次消息设置 */
	int  qos;
	bool quiet;
	bool retain;

	/* 消息源 */
	char *file_path; //文件信息
	char *message; //文字信息
	
	/* 消息目的地 */
	char *server_ip;
	int  server_port;
	char *topic;

	/* 消息属性 */
	int sender_type; //消息发送者属性:1.用户发送 2.运营发送 3.机器人发送
	int chat_mode;
	char *toUserID;
	int trans_mode;  //消息模式:1.语音消息 2.文字消息 3.点赞命令 4.上线命令 5.下线命令(如果使用服务器直接推送的方式则上下线可不必写)
	int file_type;   //文件类型(amr, mp3)
	int audio_play_time; //文件播放时长
	
	/* 客户端相关信息 */
	int  product_type;
	char *product_id;
	char *brand; //手机品牌
	char *phone_type; //手机操作系统
	
}carRadio_pub;


struct mosq_config {
	JNIEnv *sub_env;            
	jobject sub_obj;
	bool connected;
	char *id;
	char *id_prefix;
	char *user_name;
	int  client_type; //add by kuaixiang
	char *channel_id; //用户当前所在频道 by kuaixiang
	int  userType;     //标识用户是否注册过 by kuaixiang
	char *portrait_path; //用户头像地址(如果注册过，则是网络地址,否则是本地地址) by kuaixiang
	int  product_type; //add by kuaixiang, 产品类型
	char *product_id;  //add by kuaixiang, 产品ID
	char *brand; // add by kuaixiang 手机品牌
	char *phone_type; //add by kuaixiang 手机操作系统
	int protocol_version;
	int keepalive;
	char *host;
	int port;
	int qos;
	bool retain;
	int user_type;
	int pub_mode; /* pub */
	char *file_input; /* pub */
	char *message; /* pub */
	long msglen; /* pub */
	char *topic; /* pub */
	int sender_type; //pub-消息发送者属性:1.用户发送 2.运营发送 3.机器人发送
	int chat_mode;   // 1.ϵͳ��Ϣ 2.����Ƶ����Ϣ 3.˽����Ϣ
	int trans_mode;  //pub-消息模式:1.语音消息 2.文字消息 3.点赞命令 4.上线命令 5.下线命令(如果使用服务器直接推送的方式则上下线可不必写)
	char *toUserID;
	int file_type;   //pub-文件类型(amr, mp3)
	int audio_play_time; //pub-文件播放时长
	char *bind_address;
#ifdef WITH_SRV
	bool use_srv;
#endif
	bool debug;
	bool quiet;
	unsigned int max_inflight;
	char *username;
	char *password;
	char *will_topic;
	char *will_payload;
	long will_payloadlen;
	int will_qos;
	bool will_retain;
#ifdef WITH_TLS
	char *cafile;
	char *capath;
	char *certfile;
	char *keyfile;
	char *ciphers;
	bool insecure;
	char *tls_version;
#  ifdef WITH_TLS_PSK
	char *psk;
	char *psk_identity;
#  endif
#endif
	bool clean_session; /* sub */
	char **topics; /* sub */
	int topic_qos[5]; /* sub �����Ϊ5�� */
	int topic_count; /* sub */
	bool no_retain; /* sub */
	char **filter_outs; /* sub */
	int filter_out_count; /* sub */
	bool verbose; /* sub */
	bool eol; /* sub */
	int msg_count; /* sub */
#ifdef WITH_SOCKS
	char *socks5_host;
	int socks5_port;
	char *socks5_username;
	char *socks5_password;
#endif
};

int client_config_load(struct mosq_config *config, int pub_or_sub, int argc, char *argv[]);
void client_config_cleanup(struct mosq_config *cfg);
int client_opts_set(struct mosquitto *mosq, struct mosq_config *cfg);
int client_id_generate(struct mosq_config *cfg, const char *id_base);
int client_connect(struct mosquitto *mosq, struct mosq_config *cfg);
int client_topic_set(struct mosq_config *cfg, int pub_or_sub, const char *topic, const int qos);
int invalid_ipaddr(char *str);
int JNU_ThrowByName(JNIEnv *env, char *java_exc_class, char *msg);

#endif
