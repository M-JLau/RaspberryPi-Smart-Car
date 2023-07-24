//超聲波避障

#include "iostream"
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

using namespace std;

#define Trig 25 //定义超声波模块的Trig引脚连接到GPIO28
#define Echo 26 //定义超声波模块的Echo引脚连接到GPIO29

#define BUFSIZE 512 //缓冲区大小

//將控制電機的PWM、IN2、IN1分別與wirePi的引腳相對應
//AIN1/AIN2控制左輪，BIN1/BIN2控制右輪
//PWMA/PWMB控制左輪/右輪的轉速
//AIN1/AIN2、BIN1/BIN2、PWMA/PWMB為控制信號輸入端
//AO1/AO2、BO1/BO2(兩路電機控制輸出端)、VCC、GND為電機驅動端
// 定义电机A和电机B的GPIO接口号
int PWMA = 1;  // PWM控制A
int AIN2 = 2;  // 控制电机A的方向
int AIN1 = 3;  // 控制电机A的方向

int PWMB = 4;  // PWM控制B
int BIN2 = 5;  // 控制电机B的方向
int BIN1 = 6;  // 控制电机B的方向

// 创建一个函数用于控制电机向前运动
void t_up(unsigned int speed,unsigned int t_time){
    digitalWrite(AIN2,0);  // 设置电机A的方向
    digitalWrite(AIN1,1);  // 设置电机A的方向
    softPwmWrite(PWMA,speed);  // 控制电机A的速度
    digitalWrite(BIN2,0);  // 设置电机B的方向
    digitalWrite(BIN1,1);  // 设置电机B的方向
    softPwmWrite(PWMB,speed);  // 控制电机B的速度
    delay(t_time);  // 运动指定的时间
}

// 创建一个函数用于控制电机向后运动
void t_down(unsigned int speed,unsigned int t_time){
    digitalWrite(AIN2,1);  // 设置电机A的方向
    digitalWrite(AIN1,0);  // 设置电机A的方向
    softPwmWrite(PWMA,speed);  // 控制电机A的速度
    digitalWrite(BIN2,1);  // 设置电机B的方向
    digitalWrite(BIN1,0);  // 设置电机B的方向
    softPwmWrite(PWMB,speed);  // 控制电机B的速度
    delay(t_time);  // 运动指定的时间
}

// 创建一个函数用于控制小车向左转
void t_left(unsigned int speed,unsigned int t_time){
    digitalWrite(AIN2,1);  // 设置电机A的方向
    digitalWrite(AIN1,0);  // 设置电机A的方向
    softPwmWrite(PWMA,speed);  // 控制电机A的速度
    digitalWrite(BIN2,0);  // 设置电机B的方向
    digitalWrite(BIN1,1);  // 设置电机B的方向
    softPwmWrite(PWMB,speed);  // 控制电机B的速度
    delay(t_time);  // 运动指定的时间
}




//用于初始化超声波测距模块的Trig和Echo脚位
void ultraInit(void)
{
    pinMode(Echo, INPUT); // 设置Echo脚位为输入模式
    pinMode(Trig, OUTPUT); // 设置Trig脚位为输出模式
}

// 定义测量距离函数
float disMeasure(void)
{
    struct timeval tv1; // 定义时间结构体tv1
    struct timeval tv2; // 定义时间结构体tv2
    long start, stop; // 定义开始和结束时间
    float dis; // 定义距离

    digitalWrite(Trig, LOW); // 给触发脚位低电平2微秒，// Trig脚位输出低电平
    delayMicroseconds(2);

    digitalWrite(Trig, HIGH); // 给触发脚位高电平10微秒，// Trig脚位输出高电平
    delayMicroseconds(10);
    digitalWrite(Trig, LOW); // 给触发脚位低电平

    while(!(digitalRead(Echo) == 1)); // 等待接收脚位变高
    gettimeofday(&tv1, NULL); // 获取当前时间

    while(!(digitalRead(Echo) == 0)); // 等待接收脚位变低
    gettimeofday(&tv2, NULL); // 获取当前时间

    start = tv1.tv_sec * 1000000 + tv1.tv_usec; // 计算开始时间
    stop  = tv2.tv_sec * 1000000 + tv2.tv_usec; // 计算结束时间

    dis = (float)(stop - start) / 1000000 * 34000 / 2; // 计算距离

    return dis; // 返回距离
}



int main(int argc,char *argv[]){
    float dis; //定义距离变量

    wiringPiSetup(); //初始化wiringPi
    ultraInit(); //初始化超声波模块

    // 设置GPIO接口为输出模式
    pinMode(PWMA, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);

    // 创建两个软件控制的PWM信号，用于控制电机转速
    softPwmCreate(PWMA, 0, 100); // 设置电机A的PWM范围为0到100
    softPwmCreate(PWMB, 0, 100); // 设置电机B的PWM范围为0到100


    while(1){
        dis = disMeasure();
        printf("distance = %0.2f cm\n",dis);
        //delay(1000);
        if(dis < 10){
            t_down(30,0);
            t_left(30,0);
        } else{
            t_up(30,0);
        }
    }
    return 0;
}




//1、ultraInit：这个函数用于初始化超声波测距模块的Trig和Echo脚位。

//2、disMeasure：这个函数用于测量超声波测距模块的距离。首先，它通过Trig脚位发送一个超声波信号，然后通过Echo脚位接收回波。
//  通过计算发送和接收的时间差，可以计算出距离。

//3、主函数：在主函数中，首先调用wiringPiSetup函数和ultraInit函数来初始化GPIO接口和超声波模块。然后，设置电机控制脚位为输出模式，
//  并创建软件PWM信号。在无限循环中，不断测量距离并通过打印输出。如果距离小于30cm，则让机器人后退并左转，否则让机器人前进。最后，返回0表示程序正常退出。





















