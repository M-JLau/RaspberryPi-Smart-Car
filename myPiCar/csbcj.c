#include "stdio.h"
#include "wiringPi.h"
#include "stdlib.h"
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "string.h"
#include "netdb.h"
#include "sys/types.h"
#include <time.h>
#include "arpa/inet.h"
#include "sys/socket.h"

#include "softPwm.h"

#define Trig 25 // 定义 Trig 引脚连接的 GPIO
#define Echo 26 // 定义 Echo 引脚连接的 GPIO



void ultraInit(void)
{
    pinMode(Echo, INPUT); // 设置 Echo 引脚为输入模式
    pinMode(Trig, OUTPUT); // 设置 Trig 引脚为输出模式
}

float disMeasure(void)
{
    struct timeval tv1; // 定义一个时间结构体，用来存放开始接收到高电平的时间
    struct timeval tv2; // 定义一个时间结构体，用来存放高电平结束的时间（接收到低電平的時間）
    long start, stop;
    float dis;

    digitalWrite(Trig, LOW); // 给 Trig 引脚输出低电平
    delayMicroseconds(2); // 等待 2 微秒，让传感器稳定

    digitalWrite(Trig, HIGH); // 给 Trig 引脚输出高电平
    delayMicroseconds(10); // 等待 10 微秒，保持高电平
    digitalWrite(Trig, LOW); // 给 Trig 引脚输出低电平

    printf("Before first while loop\n");
    while(!(digitalRead(Echo) == 1)); // 等待接收高电平
    printf("After first while loop\n");
    gettimeofday(&tv1, NULL); // 获取当前时间

    printf("Before second while loop\n");
    while(!(digitalRead(Echo) == 0)); // 等待高电平结束
    printf("After second while loop\n");
    gettimeofday(&tv2, NULL); // 获取当前时间

    start = tv1.tv_sec * 1000000 + tv1.tv_usec; // 将开始时间转换为微秒
    stop  = tv2.tv_sec * 1000000 + tv2.tv_usec; // 将结束时间转换为微秒

    dis = (float)(stop - start) / 1000000 * 34000 / 2; // 计算距离，声速 340m/s，转换为 cm 需要 *100，来回距离除以 2

    return dis;
}

int main(int argc,char *argv[]){

    float dis;
    wiringPiSetup();
    ultraInit(); // 初始化超声波模块
    while(1){
        printf("Start measuring distance\n");
        dis = disMeasure();
        printf("Finished measuring distance\n");
        printf("distance = %0.2f cm\n",dis);
        //elay(1000);
    }
    return 0;
}

























