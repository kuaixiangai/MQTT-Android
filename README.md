
# 一起使用MQTT

@(MQTT)[LibMosquitto|聊天|北京汽车小子信息科技有限公司]

**MQTT** （**Message Queuing Telemetry Transport**，消息队列遥测传输）是IBM开发的一个即时通讯协议，有可能成为物联网的重要组成部分。该协议支持所有平台，几乎可以把所有联网物品和外部连接起来，被用来当做传感器和致动器（比如通过Twitter让房屋联网）的通信协议。


#如果想使用iOS版本和Mosquitto服务器，联系QQ：136062834

----------

[TOC]

## MQTT特点

>MQTT协议是为大量计算能力有限，且工作在低带宽、不可靠的网络的远程传感器和控制设备通讯而设计的协议，它具有以下主要的几项特性：
  1、使用发布/订阅消息模式，提供一对多的消息发布，解除应用程序耦合；
  2、对负载内容屏蔽的消息传输；
  3、使用 TCP/IP 提供网络连接；
  4、有三种消息发布服务质量：
		  - “至多一次”，消息发布完全依赖底层 TCP/IP 网络。会发生消息丢失或重复。这一级别可用于如下情况，环境传感器数据，丢失一次读记录无所谓，因为不久后还会有第二次发送。
		- “至少一次”，确保消息到达，但消息重复可能会发生。
		- “只有一次”，确保消息到达一次。这一级别可用于如下情况，在计费系统中，消息重复或丢失会导致不正确的结果。
5、小型传输，开销很小（固定长度的头部是 2 字节），协议交换最小化，以降低网络流量；
6、使用 Last Will 和 Testament 特性通知有关各方客户端异常中断的机制；

通过分享此代码希望更多的开发人员加入MQTT阵营，并且能够维护好此项目. 您也可以去官网 [MQTT官网](http://mqtt.org) 查看最新的源码.

### 代码Demo
```cpp
static int get_text_msg(const struct mosquitto_message *message, struct mosq_config *getcfg, char msg_content[]) {
	int sendMsgLen = sizeof(struct sendMessBody);
	int msg_len = message->payloadlen - sendMsgLen; //

	memcpy(msg_content, message->payload + sendMsgLen, msg_len);

	return 0;
}
```

### 接口
| 函数      |    参数 |
| :-------- | --------:|
| Connect_subObjectConnectServer | jobject obj_sub，topic_num，jobjectArray topicArray |


### 流程
#### MQTT客户端
```flow
st=>start: 开始
e=>end
op=>operation: sub连接服务器
cond=>condition: 连接成功 or 连接失败?

st->op->cond
cond(yes)->e
cond(no)->op
```


> **注意:**首先一定先将sub连接到服务器:

> - 下面可以使用sub进行进入、切换、退出频道,
> - 同时可以使用pub发送消息.


> **注意:** 当前版本的pub支持如下功能：
> 发送消息和发送文件。（发送消息，我们制定了一些模式，可以是文本消息，也可以是相关的命令。比如上下线的提示，文件的话，目前在客户端只做了音频文件的接收，格式为amr的格式。同时你也可以进行扩展）



