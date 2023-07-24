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
#include "netinet/in.h"
#include "softPwm.h"
#include "pca9685.h"
#include "lirc/lirc_client.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "softServo.h"
#include "softTone.h"

using namespace std;

#define PIN_BASE 300 // PCA9685的基准pin编号，用于通过该基准编号来控制PCA9685上的各个pin
#define HERTZ 50 //PWM频率，50Hz的频率，可以用来驱动舵机
#define BUFSIZE 512 // 缓冲区大小
#define BuzPin 0   // 定义一个宏，BuzPin对应的值为0，即蜂鸣器连接的GPIO接口号为0
#define BtnPin 24  // 定义一个宏，BtnPin对应的值为24，即按钮连接的GPIO接口号为24
#define Gpin 21    // 定义一个宏，Gpin对应的值为21，即LED灯绿色连接的GPIO接口号为21
#define Rpin 22    // 定义一个宏，Rpin对应的值为22，即LED灯红色连接的GPIO接口号为22

//轉動角度計算常量
#define MAX_PWM 4096 // 最大PWM值，PCA9685是12位的，所以最大值是2^12-1=4095

//角度計算時糾偏
#define max(x,y) ((x)>(y)?(x):(y)) // 宏定义，求最大值
#define min(x,y) ((x)<(y)?(x):(y)) // 宏定义，求最小值

void PWM_write(int servonum,float x); // 舵机控制函数
int calcTicks(float impulseMs, int hertz);

int myservo1 = 0; // 定义舵机引脚连接到PCA9685的0通道


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

typedef struct CLIENT{
    int fd; // 客户端文件描述符
    struct sockaddr_in addr; // 客户端地址
}CLIENT;

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

void t_down(unsigned int speed,unsigned int t_time){
    digitalWrite(AIN2,1);
    digitalWrite(AIN1,0);
    softPwmWrite(PWMA,speed);
    digitalWrite(BIN2,1);
    digitalWrite(BIN1,0);
    softPwmWrite(PWMB,speed);
    delay(t_time);
}

void t_left(unsigned int speed,unsigned int t_time){
    digitalWrite(AIN2,1);
    digitalWrite(AIN1,0);
    softPwmWrite(PWMA,speed);
    digitalWrite(BIN2,0);
    digitalWrite(BIN1,1);
    softPwmWrite(PWMB,speed);
    delay(t_time);
}

void t_right(unsigned int speed,unsigned int t_time){
    digitalWrite(AIN2,0);
    digitalWrite(AIN1,1);
    softPwmWrite(PWMA,speed);
    digitalWrite(BIN2,1);
    digitalWrite(BIN1,0);
    softPwmWrite(PWMB,speed);
    delay(t_time);
}

// 用于停止电机运动
void t_stop(unsigned int t_time){
    digitalWrite(AIN2,0);  // 停止电机A
    digitalWrite(AIN1,0);  // 停止电机A
    softPwmWrite(PWMA,0);  // 停止电机A
    digitalWrite(BIN2,0);  // 停止电机B
    digitalWrite(BIN1,0);  // 停止电机B
    softPwmWrite(PWMB,0);  // 停止电机B
    delay(t_time);  // 等待指定的时间
}

// 舵机控制函数，servonum为PWM輸出通道（舵机编号），x为舵机转动角度
void PWM_write(int servonum,float x)
{
    float y;
    int tick;
    y=x/90.0+0.5; // 将角度转化为占空比（PWM信號的高電平時間y）
    //防止高電平時間y超出0.5ms~2.5ms的範圍
    y= max(y,0.5); // 保证占空比y的最小值不小于0.5，防止舵机工作在极限角度，导致舵机损坏
    y= min(y,2.5); // 保证占空比y的最大值不大于2.5，防止舵机工作在极限角度，导致舵机损坏
    tick = calcTicks(y, HERTZ); // 将占空比转化为PCA9685的寄存器值，即脉冲宽度（将占空比y和频率HERTZ转换为脉冲宽度tick）
    pwmWrite(PIN_BASE+servonum,tick); // 设置舵机的脉冲宽度（将计算得到的脉冲宽度tick写入到舵机对应的PWM信号中）
}

// 计算脉冲宽度，impulseMs为占空比（脈衝寬度（高電平時間）），hertz为频率50Hz
int calcTicks(float impulseMs, int hertz)
{
    float cycleMs=1000.0f/(float)hertz; // 计算一个PWM周期的时间，单位是ms
    return (int)(MAX_PWM*impulseMs/cycleMs+0.5f); // 将脉冲宽度转换为脉冲数，这是一个线性转换公式，用于计算给定脉冲宽度和频率下的脉冲数
}



int main(int argc,char *argv[]){
    float qh_detection =0 ;
    float lr_detection =0 ;

    int sockfd; // 服务器端套接字
    int listenfd; // 监听描述符
    int connectfd; // 客户端连接（請求連結）描述符
    int ret;
    int maxfd =-1; // 最大文件描述符
    struct timeval tv; // 保存select超时时间
    struct sockaddr_in server_addr; // 服务器端地址
    struct sockaddr_in client_addr; // 客户端地址
    socklen_t len; // 保存客户端地址长度
    int portnumber; // 端口号
    char buf[BUFSIZE]={0xff,0x00,0x00,0x00,0xff}; // 缓冲区，保存接受到的数据
    int z,i,maxi=-1; // z用于保存select返回值（讀取到的字節數），i用于循环，maxi用于保存最大文件描述符（當前最大連接）
    int k; // 用于保存客户端地址结构体数组下标
    fd_set rset,allset; // rset用于保存读文件描述符集合，allset用于保存所有文件描述符集合
    CLIENT client[FD_SETSIZE]; // 保存所有客户端的信息

    wiringPiSetup();

    // 设置GPIO口为输出模式
    pinMode(1,OUTPUT);
    pinMode(2,OUTPUT);
    pinMode(3,OUTPUT);
    pinMode(4,OUTPUT);
    pinMode(5,OUTPUT);
    pinMode(6,OUTPUT);

    // 创建两个软件控制的PWM信号，用于控制电机转速
    softPwmCreate(PWMA, 0, 100); // 设置电机A的PWM范围为0到100
    softPwmCreate(PWMB, 0, 100); // 设置电机B的PWM范围为0到100

    // 初始化PCA9685，设置I2C地址为0x40，PWM频率为50Hz
    int fd = pca9685Setup(PIN_BASE,0x40,HERTZ);
    if(fd < 0) // 如果初始化失败，输出错误信息
    {
        printf("Error in setup\n");
        return fd;
    }

    pca9685PWMReset(fd); // 重置PCA9685
    printf("OK"); // 输出OK表示初始化成功

    //argc 是一个程序启动时的参数数量，包括程序本身的名字。所以当 argc != 2 时，表示用户并未按照要求提供足够的参数
    //（在这个程序中，只需要一个参数，也就是端口号）。如果用户没有提供端口号，则程序会打印错误信息并退出。
    if(argc!=2){
        printf("Please add portnumber!");
        exit(1);
    }
    if((portnumber=atoi(argv[1]))<0){ // 如果端口号小于0，打印错误信息并退出
        printf("Enter Error!");
        exit(1);
    }

    //參數：IP協議（協議域），TCP協議（套接字類型），0（協議號）
    if((listenfd = socket(PF_INET,SOCK_STREAM,0))==-1){ // 创建套接字（一個未連接的網絡接口）
        printf("socket Error!");
        exit(1);
    }
    memset(&server_addr,0,sizeof(server_addr)); // 初始化服务器地址，将server_addr结构体中的所有成员初始化为0
    server_addr.sin_family = AF_INET; // 设置地址族为IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 设置服务器IP地址，INADDR_ANY表示可以接受任意IP地址的数据。htonl函数用于将主机字节顺序转换为网络字节顺序。
    server_addr.sin_port=htons(portnumber); // 设置服务器端口号，htons函数用于将主机字节顺序转换为网络字节顺序。

    // 将套接字 listenfd 绑定到（server_addr 结构体包含的）特定的 IP 地址和端口号
    if((bind(listenfd,(struct sockaddr *)(&server_addr),sizeof server_addr)) == -1){ // 绑定套接字
        printf("Bind Error!");
        exit(1);
    }
    if(listen(listenfd,128)==-1){ // 监听套接字，最大连接数为128
        printf("Listen Error!");
        exit(1);
    }

    for(i=0;i<FD_SETSIZE;i++){ // 初始化客户端信息
        client[i].fd = -1; // 将所有客户端的文件描述符设置为-1
    }

    // 初始化文件描述符集合，準備進行select系統調用
    FD_ZERO(&allset); // 清空allset文件描述符集合
    FD_SET(listenfd,&allset); // 将listenfd加入allset文件描述符集合
    maxfd = listenfd; // 设置最大文件描述符为listenfd（指定被监听的文件描述符的范围）
    while(1){
        rset = allset;
        // 设置select超时时间为1微秒
        tv.tv_sec = 0; // 设置select超时时间为0秒
        tv.tv_usec = 1; // 设置select超时时间为1微秒
        ret = select(maxfd+1,&rset,NULL,NULL,&tv); // 使用select监听文件描述符集合中的所有文件描述符，当有文件描述符可读时，select返回
        if(ret  == 0) { // select超时，没有文件描述符准备好，继续监听
            continue;
        } else if(ret < 0){ // select出错，退出程序
            printf("select failed!");
            break;
        } else { // select返回，说明有文件描述符准备好了
            if (FD_ISSET(listenfd, &rset)) { // 如果listenfd准备好了，说明有新的客户端请求连接
                len = sizeof(struct sockaddr_in); // 保存客户端地址结构体长度
                if ((connectfd = accept(listenfd, (struct sockaddr *) &client_addr, &len)) == -1) { // 接受客户端连接
                    printf("Accept Error!");
                    continue;
                }
                for (i = 0; i < FD_SETSIZE; i++) { // 查找空闲位置，将客户端信息保存到client数组中
                    if (client[i].fd < 0) { // 找到空闲位置，保存客户端信息
                        client[i].fd = connectfd; // 保存客户端套接字
                        client[i].addr = client_addr; // 保存客户端地址结构体
                        printf("Yout got a connection from %s\n", inet_ntoa(client[i].addr.sin_addr)); // 打印客户端IP地址
                        break;
                    }
                }
                if (i == FD_SETSIZE) { // 如果客户端连接数达到最大值，退出程序
                    printf("Overfly connections!");
                    //exit(0);
                }
                FD_SET(connectfd, &allset); // 将新的客户端套接字加入文件描述符集合
                if (connectfd > maxfd) { // 如果新的连接请求的文件描述符大于maxfd
                    maxfd = connectfd; // 更新maxfd
                }
                if (i > maxi) { // 更新客户端连接数（如果新的连接请求的序号大于maxi）
                    maxi = i;
                }
                /*if(--ret == 0){
                    continue;
                }*/
            } else { // 如果不是listenfd准备好了，说明是已经连接的客户端有数据过来了
                for (i = 0; i <= maxi; i++) { // 遍历所有的连接请求，找到已经连接的客户端
                    if ((sockfd = client[i].fd) < 0) { // 如果客户端套接字小于0，说明当前位置没有客户端连接，继续遍历
                        continue;
                    }
                    if(FD_ISSET(sockfd,&rset)){ // 如果客户端套接字准备好了（客户端的文件描述符在rset集合中），说明有客户端数据过来了（表示有数据可读）
                        bzero(buf,BUFSIZE + 1); // 清空buf缓存区
                        if((z = read(sockfd,buf,sizeof buf))>0){ // 读取客户端数据，如果讀到數據
                            buf[z] = '\0'; // 在字符串末尾添加结束符
                            printf("num = %d received data:%s\n",z,buf); // 打印客户端数据
                            // FF000300FF
                            if(z == 5){ // 如果接收到的数据长度为5
                                if(buf[1]==0x00){ // 如果接收到的前兩個字節是0x00
                                    switch (buf[2]) { // 判斷第三個字節
                                        case 0x03:t_up(50,0);
                                            printf("forward\n");
                                            break;
                                        case 0x04:t_down(50,0);
                                            printf("back\n");
                                            break;
                                        case 0x01:t_left(50,0);
                                            printf("left\n");
                                            break;
                                        case 0x02:t_right(50,0);
                                            printf("right\n");
                                            break;
                                        case 0x00:t_stop(0);
                                            printf("stop\n");
                                            break;
                                        case 0x06:qh_detection += 10;
                                            if(qh_detection <= 0)
                                                qh_detection = 0;
                                            if(qh_detection >= 180)
                                                qh_detection = 180;
                                            PWM_write(14,qh_detection);
                                            break;
                                        case 0x05:qh_detection -= 10;
                                            if(qh_detection <= 0)
                                                qh_detection = 0;
                                            if(qh_detection >= 180)
                                                qh_detection = 180;
                                            PWM_write(14,qh_detection);
                                            break;
                                        case 0x08:lr_detection += 10;
                                            if(lr_detection <= 0)
                                                lr_detection = 0;
                                            if(lr_detection >= 180)
                                                lr_detection = 180;
                                            PWM_write(13,lr_detection);
                                            break;
                                        case 0x07:lr_detection -= 10;
                                            if(lr_detection <= 0)
                                                lr_detection = 0;
                                            if(lr_detection >= 180)
                                                lr_detection = 180;
                                            PWM_write(13,lr_detection);
                                            break;
                                        default:t_stop(0);
                                            printf("stop\n");
                                            break;
                                    }
                                } else{ // 如果接收到的前兩個字節不是0x00，讓小車停止
                                    t_stop(0);
                                }
                            } else if(z == 6){ // 如果接收到的数据长度为6
                                if(buf[2] == 0x00){ // 如果接收到的第三个字节是0x00
                                    switch (buf[3]) { // 判断第四个字节
                                        case 0x01:t_up(50,0); // 如果第四个字节是0x01，小车前进
                                            printf("forward\n");
                                            break;
                                        case 0x02:t_down(50,0);
                                            printf("back\n");
                                            break;
                                        case 0x03:t_left(50,0);
                                            printf("left\n");
                                            break;
                                        case 0x04:t_right(50,0);
                                            printf("right\n");
                                            break;
                                        case 0x00:t_stop(0);
                                            printf("stop\n");
                                            break;
                                        default:
                                            break;
                                    }
                                } else{ // 如果接收到的第三个字节不是0x00，让小车停止
                                    t_stop(0);
                                }
                            } else{ // 如果接收到的数据长度不是3或6，让小车停止
                                t_stop(0);
                            }
                        } else{ // 如果客户端套接字准备好了，但是没有读到数据，说明客户端断开连接了
                            printf("disconnected by client!\n"); // 打印客户端断开连接
                            close(sockfd); // 关闭客户端套接字（關閉文件描述符）
                            FD_CLR(sockfd,&allset); // 将客户端套接字从allset文件描述符集合中清除
                            client[i].fd = -1; // 将客户端套接字设置为-1，表示当前位置没有客户端连接（將客戶端数组中的位置设为空闲）
                        }
                    }
                }
            }
        }
    }
    close(listenfd); // 关闭监听套接字
    return 0;
}






















