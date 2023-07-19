//控制四轮小车行动，使用了WiringPi库来控制GPIO（General Purpose Input/Output，通用输入/输出）引脚
//使用了软件PWM来控制电机的转速，使用了delay函数来控制电机的转动时间
//使用了wiringPiSetup函数来初始化WiringPi库，使用了pinMode函数来设置引脚的输入输出模式
//使用了softPwmCreate函数来创建软件PWM，使用了softPwmWrite函数来设置软件PWM的占空比
//使用了digitalWrite函数来设置引脚的电平，使用了delay函数来设置延时

#include "iostream"
#include "stdio.h"
#include "wiringPi.h"
#include "stdlib.h"
#include <unistd.h>
#include <errno.h>
#include "string.h"
#include "netdb.h"
#include "sys/types.h"
#include <time.h>
#include "arpa/inet.h"

#include "softPwm.h"

using namespace std;

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

// 创建一个函数用于控制小车向右转
void t_right(unsigned int speed,unsigned int t_time){
    digitalWrite(AIN2,0);  // 设置电机A的方向
    digitalWrite(AIN1,1);  // 设置电机A的方向
    softPwmWrite(PWMA,speed);  // 控制电机A的速度
    digitalWrite(BIN2,1);  // 设置电机B的方向
    digitalWrite(BIN1,0);  // 设置电机B的方向
    softPwmWrite(PWMB,speed);  // 控制电机B的速度
    delay(t_time);  // 运动指定的时间
}

// 创建一个函数用于停止电机运动
void t_stop(unsigned int t_time){
    digitalWrite(AIN2,0);  // 停止电机A
    digitalWrite(AIN1,0);  // 停止电机A
    softPwmWrite(PWMA,0);  // 停止电机A
    digitalWrite(BIN2,0);  // 停止电机B
    digitalWrite(BIN1,0);  // 停止电机B
    softPwmWrite(PWMB,0);  // 停止电机B
    delay(t_time);  // 等待指定的时间
}

int main(int argc, char *argv[]){
    wiringPiSetup();

    // 设置GPIO接口为输出模式
    pinMode(PWMA, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);

    // 创建PWM控制
    softPwmCreate(PWMA, 0, 100); // 设置电机A的PWM范围为0到100
    softPwmCreate(PWMB, 0, 100); // 设置电机B的PWM范围为0到100

    // 控制小车运动
    t_up(50,2000); // 小车以50%的速度向前运动2000ms
    t_down(50,2000); // 小车以50%的速度向后运动2000ms
    t_left(50,2000); // 小车以50%的速度向左转2000ms
    t_right(50,2000); // 小车以50%的速度向右转2000ms
    t_stop(2000); // 小车停止2000ms

    while(1){

    }
    return 0;
}
