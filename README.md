# RaspberryPi-Smart-Car
Raspberry Pi-based Intelligent Car

# 树莓派智能小车项目

本项目是一个基于树莓派的智能小车，实现了以下功能：LED闪烁、按键处理、蜂鸣器、寻迹行驶、红外避障、超声波测距、超声波避障行驶、超声波传感器舵机控制、红外遥控、Wi-Fi控制小车、云台舵机。

## 环境配置

### 1. 安装wiringPi库

由于WiringPi的原始仓库已经无法访问，但你仍然可以从GitHub上的镜像仓库中获取。以下是安装步骤：

1.1 首先克隆WiringPi的GitHub仓库到你的树莓派上：

   ```
   git clone https://github.com/WiringPi/WiringPi.git
   ```

1.2 然后进入到克隆下来的WiringPi目录：
  ```
  cd WiringPi
  ```

1.3 在该目录中，有一个名为build的脚本用于编译和安装WiringPi，你可以运行此脚本来安装WiringPi：
```
./build
```
1.4 等待编译和安装过程完成。完成后，你可以运行以下命令来检查WiringPi是否已经成功安装。如果安装成功，这个命令应该会输出WiringPi的版本信息：
```
gpio -v
```

### 2. 配置網絡攝像頭

請按照以下步驟操作：

開啟並設置攝像頭：

2.1 首先開啟您的樹莓派並確保它已連接到互聯網。

2.2 通過運行以下命令來開啟樹莓派的攝像頭：sudo raspi-config，然後在界面中找到並啟用攝像頭。

2.3 更新您的樹莓派：
```
sudo apt-get update
```
2.4 安裝必要的軟件：
```
sudo apt-get install subversion libjpeg8-dev imagemagick libv4l-dev cmake git
```
2.5 從github上clone mjpg-streamer項目：
```
sudo git clone https://github.com/jacksonliam/mjpg-streamer.git
```
2.6 進入到該項目的目錄：
```
cd mjpg-streamer/mjpg-streamer-experimental
```
2.7 編譯並安裝該項目：
```
make all 和 sudo make install
```
2.8 啟動攝像頭：
```
./mjpg_streamer -i "./input_uvc.so" -o "./output_http.so -w ./www"
```

這樣，您的攝像頭服務就可以運行了，然後你可以在浏览器中通过```http://192.168.43.116:8080/?action=stream ```訪問摄像头的實時視頻

### 3. 紅外配置
3.1 更新并安装LIRC: 使用命令 ```sudo apt-get update -y ```和 ```sudo apt-get upgrade -y ```更新系统，然后使用命令 ```sudo apt-get install lirc -y``` 安装LIRC。

3.2 设置红外接收管脚: 编辑 ```/boot/config.txt``` 文件，将 dtoverlay=gpio-ir,gpio_pin=4 这一行取消注释，设置红外接收器的GPIO脚为4。

3.3 配置LIRC硬件接口: 编辑 ```/etc/lirc/lirc_options.conf```文件，将 driver=default 和 device=/dev/lirc0 这两行取消注释。然后重启树莓派。

3.4 测试红外接收: 使用命令 ```mode2 -d /dev/lirc0``` 来测试红外接收，如果按下遥控器按钮终端有数据产生，说明连接成功。

3.5 录制红外按键信号: 首先，停止LIRC服务，然后使用命令 ```irrecord -d /dev/lirc0 ~/lircd.conf``` 来开始录制红外信号。按照提示进行操作，录制所有需要的按键。

3.6 配置录制的按键: 将录制的文件复制到 ```/etc/lirc/ ```目录下，并将原有的 lircd.conf 文件改名为 lircd.conf.dist，将新的配置文件命名为 lircd.conf。编辑该文件，将每个按键的配置设置好。

3.7 设置按键的响应操作: 编辑 lircrc 文件，设置每个按键的响应操作。例如，当接收到 KEY_PREVIOUS 按键信号时，执行 echo "put previous" 操作。

3.8 重启LIRC服务: 使用命令 ```sudo service lircd restart``` 重启LIRC服务。

以上步骤完成之后，你应该可以通过红外接收器接收并解析遥控器的按键信号，然后执行相应的操作。

```**關於紅外遙控詳細的配置步驟和文件在《環境配置》目錄中。**```

## 安装指南

此项目需要树莓派和其他一些硬件设备:
超聲波模塊HC-SR04、舵機SG90

关于软件部分，你需要安装以下软件:
FinalShell
VNC Viewer
WI-FI控制上位機（小R科技WIFIRobotV2.1）

## 功能描述和使用方法

- **LED閃爍**：
工作原理:
通过使用wiringPi库控制GPIO接口，实现了在树莓派上使LED灯交替闪烁的功能。

    - **編譯命令**：
     
        ```
        g++ led.cpp -o led -lwiringPi
        ```
        
- **按鍵處理**：
   - **編譯命令**：
     
        ```
        g++ button.cpp -o button -lwiringPi
        ```
        
- **蜂鳴器**：
工作原理:
用于树莓派的GPIO接口控制蜂鸣器播放两段旋律。

    - **編譯命令**：
     
        ```
        g++ fmq.cpp -o fmq -lwiringPi
        ```
        
- **尋跡行駛**：
工作原理:
如果小车的两个传感器（LEFT 和 RIGHT）都没有触发（即它们都读取到LOW），小车将向前行驶，即执行run()函数。
如果左边的传感器（LEFT）触发了（读取到HIGH），而右边的传感器（RIGHT）没有触发（读取到LOW），小车将向右行驶，即执行left()函数。
如果右边的传感器（RIGHT）触发了（读取到HIGH），而左边的传感器（LEFT）没有触发（读取到LOW），小车将向左行驶，即执行right()函数。
如果两个传感器都触发了（即它们都读取到HIGH），小车将停止行驶，即执行brake()函数。

    - **編譯命令**：
     
        ```
        g++ xj.cpp -o xj -lwiringPi
        ```
        
- **红外避障**： 
   - **編譯命令**：
     
        ```
        g++ hwbz.cpp -o hwbz -lwiringPi -lpthread
        ```
        
- **超声波测距**：
   - **編譯命令**：
     
        ```
        gcc csbcj.c -o csbcj -lwiringPi
        ```
        
- **超声波避障行駛**：
工作原理:
使用超声波传感器来检测小车前方的距离，并根据测量的距离来控制小车的行驶方向。
在每次循环中，它会先测量小车前方的距离。如果这个距离小于30厘米，那么它会先让小车向后退，然后向左转。如果前方的距离大于30厘米，那么它会让小车向前行驶。

  
    - **編譯命令**：
     
        ```
        g++ csbbz.cpp -o csbgz -lwiringPi
        ```
        
- **超聲波傳感器舵機控制**：
工作原理:
小車首先检测前方的距离。如果距离小于40厘米，则向后移动一段距离，然后检测左侧和右侧的距离。如果左侧和右侧的距离都小于40厘米，则向左转。否则，选择距离较远的一侧转向。如果前方距离大于40厘米，则向前移动。

  
    - **編譯命令**：
     
        ```
        g++ csbdj.cpp -o csbdj pca9685.cpp -lwiringPi -lwiringPiDev
        ```
        
- **红外遥控**：
工作原理:
通过红外遥控器控制一个小车的前进、后退、左转、右转和停止。通过wiringPi库和lirc库，实现了对GPIO的操作和红外遥控器信号的接收。 
  
     - **編譯命令**：
     
        ```
        gcc hwyk.c -o hwyk -lwiringPi -llirc_client
        ```
        
- **Wi-Fi控制小车**：
工作原理:
实现了一个基于TCP的服务器，可以接受多个客户端的连接请求，并接收客户端发送的控制命令，然后根据接收到的命令，控制小车的运动。

    - **編譯命令**：
     
        ```
        g++ wifikzxc.cpp -o car -lwiringPi -lwiringPiDev
        ```
     - **運行命令**：
     
        ```
        ./car 2002(端口號(需要與上位機設置一致))
        ```
  
- **云台舵机**：
工作原理:
实现了一个基于TCP的服务器，可以接受多个客户端的连接请求，并接收客户端发送的控制命令，然后根据接收到的命令，控制小車行駛和雲台上下搖頭以及左右搖頭。
  
     - **編譯命令**：
     
        ```
        g++ main.cpp -o car pca9685.cpp -lwiringPi -lwiringPiDev
        ```
     - **運行命令**：
     
        ```
        ./car 2002(端口號(需要與上位機設置一致))
        ```

## 常见问题

1. **不知道PCA9685控制器上每一个PWM通道对应的舵机**：可以运行《舵机确认》目录中的代码。
2. **同时开启摄像头服务和小车程序，在手机端/PC端通过网络连接小车，和摄像头**：注意小车控制和摄像头是两个程序，互不影响。

    - **开启网络摄像头步骤**：
    
        ```
        cd mjpg-streamer/mjpg-streamer-experimental
        ./mjpg_streamer -i "./input_uvc.so" -o "./output_http.so -w ./www"
        ```

    - **运行小车程序步骤**：
    
        ```
        //编译小车代码
        g++ main.cpp -o car pca9685.cpp -lwiringPi -lwiringPiDev
        //运行小车程序
        sudo ./car 2002
        ```
        
