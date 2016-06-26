
LOCAL_PATH := $(call my-dir) 

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include       
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog -llog -lz -pthread -lm -luuid
 
LOCAL_MODULE := libcarRadio     
LOCAL_SRC_FILES:= \
	mosquitto.c \
	client_shared.c \
	sub_client.c \
	jsubApi.c \
	logging_mosq.c \
	memory_mosq.c \
	messages_mosq.c \
	net_mosq.c \
	read_handle.c \
	read_handle_client.c \
	read_handle_shared.c \
	send_mosq.c \
	send_client_mosq.c \
	socks_mosq.c \
	srv_mosq.c \
	thread_mosq.c \
	time_mosq.c \
	tls_mosq.c \
	util_mosq.c \
	will_mosq.c 
	
LOCAL_CFLAGS := \
    -DHAVE_CONFIG_H \
    -DFPM_DEFAULT

include $(BUILD_SHARED_LIBRARY)