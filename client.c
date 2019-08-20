/*************************************************************************
	> File Name: client.c
	> Author: 
	> Mail: 
	> Created Time: Fri 19 Jul 2019 03:06:47 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<event2/event.h>
#include<event2/bufferevent.h>
#include<fcntl.h>
#include<arpa/inet.h>

//读回调
void read_cb(struct bufferevent* bev, void* arg)
{
    //接收对方发过来的数据
    char buf[1024] = {0};
    int len = bufferevent_read(bev, buf, sizeof(buf));
    printf("recv buf: %s\n", buf);
    
    //给对方发数据
   // bufferevent_write(bev, buf, len+1);
    printf("数据已发送完成....\n");
}

//写回调
void write_cb(struct bufferevent* bev, void* arg)
{
   printf("我是一个混吃混喝的函数"); 
}

void event_cb(struct bufferevent *bev, short events, void *arg)
{
    if(events & BEV_EVENT_EOF)
    {
        printf("connection closed\n");
    }
    else if(events & BEV_EVENT_ERROR)
    {
        printf("some other error\n");
    }
    else if(events & BEV_EVENT_CONNECTED)
    {
        printf("event connect success\n");
        return;
    }
    //释放资源
    bufferevent_free(bev);
}


void read_terminal(int fd, short what, void* arg)
{
    //读终端中的数据
    char buf[1024] = {0};
    int len = read(fd, buf, sizeof(buf));

    //将数据发送给server
    struct bufferevent* bev = (struct bufferevent*)arg;
    bufferevent_write(bev, buf, len+1);
}

//终端接收输入，将数据发送给server



int main(int argc, const char *argv[])
{
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family  = AF_INET;
    serv.sin_port = htons(8765);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr.s_addr);


    //创建一个事件处理框架
    struct event_base* base = event_base_new();
    
    
    //创建事件
    //连接服务器 创建文件描述符
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    //把fd搞到bufferevent里
    struct bufferevent* bev = NULL;
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    //连接服务器 
    bufferevent_socket_connect(bev, (struct sockaddr*)&serv, sizeof(serv));
    //给缓冲区设置回调
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
    bufferevent_enable(bev, EV_READ);

    //接收键盘输入
    //创建一个新事件
    struct event* ev = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, read_terminal, bev);
    
    //将事件添加到event_base
    event_add(ev, NULL);

    //启动事件循环
    event_base_dispatch(base);

    //释放资源
    event_base_free(base);
    return 0;
}
