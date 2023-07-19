//使用树莓派的GPIO接口控制一个LED灯（具有红绿两种颜色）和一个按钮
//当按下按钮时，LED灯变为红色，松开按钮后，LED灯变为绿色

#include "iostream"
#include "stdio.h"
#include "wiringPi.h"

using namespace std;

#define BtnPin 24  // 定义一个宏，BtnPin对应的值为24，即按钮连接的GPIO接口号
#define Gpin 21  // 定义一个宏，Gpin对应的值为21，即LED灯的绿色部分连接的GPIO接口号
#define Rpin 22  // 定义一个宏，Rpin对应的值为22，即LED灯的红色部分连接的GPIO接口号

// 创建一个函数用于控制LED灯的颜色
void LED(char *color){
    pinMode(Gpin, OUTPUT);  // 设置Gpin为输出模式
    pinMode(Rpin, OUTPUT);  // 设置Rpin为输出模式

    if(color == "RED"){  // 如果输入的颜色为红色
        digitalWrite(Rpin, HIGH);  // 点亮红色LED
        digitalWrite(Gpin, LOW);  // 熄灭绿色LED
    }
    else if(color == "GREEN"){  // 如果输入的颜色为绿色
        digitalWrite(Rpin, LOW);  // 熄灭红色LED
        digitalWrite(Gpin, HIGH);  // 点亮绿色LED
    }
    else  // 如果输入的颜色既不是红色也不是绿色，打印错误信息
        printf("LED ERROR!\n");
}


int main(){
    // 初始化wiringPi库，如果失败则输出错误信息并返回1
    if(wiringPiSetup() == -1){
        cout << "Setup wiringPi failed!" << endl;
        return 1;
    }

    pinMode(BtnPin, INPUT);  // 设置按钮接口为输入模式
    LED("GREEN");  // 设置初始的LED灯颜色为绿色

    while(1){ // 创建一个无限循环，用于检测按钮状态并更新LED灯颜色
        if(0 == digitalRead(BtnPin)){ // 如果按钮被按下
            delay(10); // 等待10ms以消除抖动
            if(0 == digitalRead(BtnPin)){ // 如果按钮仍然被按下
                LED("RED"); // 改变LED灯颜色为红色
            }
        } else if(1 == digitalRead(BtnPin)){ // 如果按钮没有被按下
            delay(10); // 等待10ms以消除抖动
            if(1 == digitalRead(BtnPin)){ // 如果按钮仍然没有被按下
                while(!digitalRead(BtnPin)); // 等待按钮被按下
                LED("GREEN"); // 改变LED灯颜色为绿色
                printf("Button ERROR!\n");
            }
        }
    }

    return 0;
}