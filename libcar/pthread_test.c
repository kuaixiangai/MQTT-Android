#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#define MAX 10

#include "client_shared.h"
#include "stdbool.h"
#include "jsubApi.h"

pthread_t thread[2];
pthread_mutex_t mut;
int number=0, i;

void *thread1()
{
        printf ("thread1 : I'm thread 1\n");
	//执行连接
	carRadio_sub st_sub_Interface;

	int i = 0;
	memset(st_sub_Interface.topics, 0, sizeof(st_sub_Interface.topics));
	for(i = 0; i < 2; i++) {
		sprintf(st_sub_Interface.topics[i], "1/%d", i);
//		__android_log_print(ANDROID_LOG_INFO, "JNITag", "joarr[%d]=%s", i, psz);
	}


	//设置st_sub_Interface
	st_sub_Interface.uuid = "kuaikuaai";
	st_sub_Interface.product_id = "111";
	st_sub_Interface.server_ip = "10.1.6.56";
	st_sub_Interface.topic = "1/3";
	st_sub_Interface.file_path = "/home";

	st_sub_Interface.client_type = 2;
	st_sub_Interface.server_port = 1883;
	st_sub_Interface.product_type = 1;
	st_sub_Interface.keepalive = 30;

	st_sub_Interface.clean_session = true;
	st_sub_Interface.quiet = true;

	st_sub_Interface.topic_num = 2;
	
	
	sub_init_connect(NULL, NULL, st_sub_Interface);



        pthread_exit(NULL);
}

void *thread2()
{
        printf("thread2 : I'm thread 2\n");
	//
	sleep(5);
	sub_enter_channel(NULL, NULL, "3/3");

    pthread_exit(NULL);
}

void thread_create(void)
{
        int temp;
        memset(&thread, 0, sizeof(thread));          //comment1
        /*创建线程*/
        if((temp = pthread_create(&thread[0], NULL, thread1, NULL)) != 0)  //comment2     
                printf("线程1创建失败!\n");
        else
                printf("线程1被创建\n");

        if((temp = pthread_create(&thread[1], NULL, thread2, NULL)) != 0)  //comment3
                printf("线程2创建失败\n");
        else
                printf("线程2被创建\n");
}

void thread_wait(void)
{
        /*等待线程结束*/
        if(thread[0] !=0)
           {             //comment4    
                pthread_join(thread[0],NULL);
                printf("线程1已经结束\n");
          }
        if(thread[1] !=0) 
           {  
                //comment5
               pthread_join(thread[1],NULL);
                printf("线程2已经结束\n");
         }
}

int main()
{
        /*用默认属性初始化互斥锁*/
        pthread_mutex_init(&mut,NULL);

        printf("我是主函数哦，我正在创建线程，呵呵\n");
        thread_create();
        printf("我是主函数哦，我正在等待线程完成任务阿，呵呵\n");
        thread_wait();

        return 0;
}
