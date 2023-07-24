#include "iostream"
#include "stdio.h"
#include "wiringPi.h"
#include "stdlib.h"
#include "sys/time.h"
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

//定义了左右传感器的GPIO接口
#define LEFT 27 // 定义左传感器的GPIO端口号
#define RIGHT 26 // 定义右传感器的GPIO端口号

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

// 定义电机控制宏，控制电机的方向
#define MOTOR_GO_FORWARD digitalWrite(1,HIGH);digitalWrite(4, LOW);digitalWrite(5, HIGH);digitalWrite(6, LOW)  // 小車向前
#define MOTOR_GO_BACK digitalWrite(4, HIGH);digitalWrite(1,LOW);digitalWrite(6, HIGH);digitalWrite(5, LOW)  // 小車向后
#define MOTOR_GO_RIGHT digitalWrite(1, HIGH);digitalWrite(4, LOW);digitalWrite(6, HIGH);digitalWrite(5, LOW)  // 小車向右
#define MOTOR_GO_LEFT digitalWrite(4, HIGH);digitalWrite(1,LOW);digitalWrite(5, HIGH);digitalWrite(6, LOW)  // 小車向左
#define MOTOR_GO_STOP digitalWrite(1, LOW);digitalWrite(4, LOW);digitalWrite(5, LOW);digitalWrite(6, LOW)  // 小車停止

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

int main(){
    wiringPiSetup();

    // 设置GPIO接口为输出模式
    pinMode(1,OUTPUT);
    pinMode(2,OUTPUT);
    pinMode(3,OUTPUT);
    pinMode(4,OUTPUT);
    pinMode(5,OUTPUT);
    pinMode(6,OUTPUT);

    // 创建PWM控制
    softPwmCreate(PWMA, 0, 100); // 设置电机A的PWM范围为0到100
    softPwmCreate(PWMB, 0, 100); // 设置电机B的PWM范围为0到100

    int SR; // 定义变量，用于存储右传感器的值
    int SL; // 定义变量，用于存储左传感器的值

    //有障礙物為LOW，無障礙物為HIGH
    while(1){
        SR = digitalRead(RIGHT); // 读取右传感器的值
        SL = digitalRead(LEFT); // 读取左传感器的值

        // 根据传感器的值，控制小车的运动方向
        if(SL == LOW && SR == LOW){
            printf("BACK");
            back();
            delay(300);
            left();
            delay(601);
        } else if(SL == HIGH && SR == LOW){
            printf("RIGHT");
            left();
        } else if(SR == HIGH && SL == LOW){
            printf("LEFT");
            right();
        } else{
            printf("GO");
            run();
        }
    }
    return 0;
}


























