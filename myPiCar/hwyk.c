#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <time.h>
#include <arpa/inet.h>

#include <wiringPi.h>
#include <softPwm.h>
#include <lirc/lirc_client.h>


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

// 遥控器按键映射数组
char *keymap[21]={
        " KEY_CHANNELDOWN ",
        " KEY_CHANNEL ", // 前進
        " KEY_CHANNELUP ",
        " KEY_PREVIOUS ", // 左轉
        " KEY_NEXT ", // 停止
        " KEY_PLAYPAUSE ", // 右轉
        " KEY_VOLUMEDOWN ",
        " KEY_VOLUMEUP ", // 後退
        " KEY_EQUAL ",
        " KEY_NUMERIC_0 ",
        " BTN_0 ",
        " BTN_1 ",
        " KEY_NUMERIC_1 ",
        " KEY_NUMERIC_2 ",
        " KEY_NUMERIC_3 ",
        " KEY_NUMERIC_4 ",
        " KEY_NUMERIC_5 ",
        " KEY_NUMERIC_6 ",
        " KEY_NUMERIC_7 ",
        " KEY_NUMERIC_8 ",
        " KEY_NUMERIC_9 "
};

// 控制电机向前运动
void t_up(unsigned int speed,unsigned int t_time)
{
    digitalWrite(AIN2,0);  // 设置AIN2为低电平，确定电机方向
    digitalWrite(AIN1,1);  // 设置AIN1为高电平，确定电机方向
    softPwmWrite(PWMA,speed);  // 控制左电机PWM值，即速度
    digitalWrite(BIN2,0);  // 设置BIN2为低电平，确定电机方向
    digitalWrite(BIN1,1);  // 设置BIN1为高电平，确定电机方向
    softPwmWrite(PWMB,speed);  // 控制右电机PWM值，即速度
    delay(t_time);  // 延时t_time毫秒
}

// 控制电机向后运动
void t_down(unsigned int speed,unsigned int t_time){
    digitalWrite(AIN2,1);
    digitalWrite(AIN1,0);
    softPwmWrite(PWMA,speed);
    digitalWrite(BIN2,1);
    digitalWrite(BIN1,0);
    softPwmWrite(PWMB,speed);
    delay(t_time);
}

// 创建一个函数用于控制小车向左转
void t_left(unsigned int speed,unsigned int t_time){
    digitalWrite(AIN2,1);
    digitalWrite(AIN1,0);
    softPwmWrite(PWMA,speed);
    digitalWrite(BIN2,0);
    digitalWrite(BIN1,1);
    softPwmWrite(PWMB,speed);
    delay(t_time);
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

// 控制小车运动的函数，根据红外遥控器发送的指令，控制小车的前进、后退、左转、右转和停止
void ircontrol(char *code){
    if(strstr(code,keymap[1])){
        t_up(50,10); // 以50的速度向前运动10毫秒
    } else if(strstr(code,keymap[7])){
        t_down(50,10);
    } else if(strstr(code,keymap[3])){
        t_left(50,10);
    } else if(strstr(code,keymap[5])){
        t_right(50,10);
    } else if(strstr(code,keymap[4])){
        t_stop(10);
    }
}

int main(void){

    struct lirc_config *config; //声明一个lirc_config结构体指针变量，用于存储红外遥控的配置信息
    int buttonTimer = millis(); // 存储当前时间
    char *code; // 用于存储红外遥控器发送的指令
    char *c;

    if(wiringPiSetup() == -1){
        printf("setup wiringPi failed !");
        return 1;
    }

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

    if(lirc_init("lirc",1)==-1){ //使用lirc库进行红外遥控初始化
        exit(EXIT_FAILURE);
    }

    // 第一个参数是一个文件名字符串。如果给出一个文件名，lirc_readconfig 将尝试从那个文件中读取配置信息。在这个代碼中，传递的是 NULL，那么它将会使用默认的 LIRC 配置文件。
    // 第二个参数是一个指向 struct lirc_config * 类型的指针。这个结构用于存储读取的配置信息。
    // 第三个参数是一个可选的日志回调函数，如果给定，这个函数会在解析配置文件时被调用以报告任何警告或错误。在这里，这个参数为 NULL，意味着没有指定回调函数。
    if(lirc_readconfig(NULL,&config,NULL)==0){ // 读取红外遥控的配置信息
        while(lirc_nextcode(&code)==0){ // 循环接收红外遥控器的信号
            if(code==NULL) continue; // 如果没有接收到信号，则继续循环
            {
                if (millis() - buttonTimer > 400) { // 每400毫秒处理一次遥控器的信号。判断两次按键的时间间隔，如果大于400毫秒，则执行下面的代码
                    ircontrol(code); // 根据收到的信号控制小车
                    //buttonTimer = millis();
                }
            }
            free(code); // 释放存储信号的内存
        }
        lirc_freeconfig(config); // 释放存储配置信息的内存
    }
    lirc_deinit(); // 关闭lirc
    exit(EXIT_SUCCESS); // 结束程序，返回成功状态
    return 0;
}























