#include <AT89x51.H>		//器件配置文件
#include <intrins.h>

#define TRIG P1_1	//分别给超声波发射口和接受口设置别名，
#define ECHO P1_2	//这里11 12引脚可以随便使用，只要是P1~P3间的IO口就行

//下面是对1602显示频引脚的定义
//这里较为固定，因为我们接的就是最后一排那个接口，
//所以这里需要查看一下开发板的原理图
#define LCM_EN P0_5  //显示器的使能端
#define LCM_RS P0_7	 //寄存器选择
#define LCM_RW P0_6  //读写选择线
#define LCM_DATA P2	 //数据选择线，共有P20-P28个数据选择端

#define BUSY 0x80   //用来检测LCM是否是busy表示，读取BF字段的值
typedef bit BOOL;

/*
1602基本的引脚功能：
第1脚：VSS为电源地
第2脚：VCC接5V电源正极
第4脚：RS为寄存器选择，高电平1时选择数据寄存器、低电平0时选择指令寄存器。
第5脚：RW为读写信号线，高电平(1)时进行读操作，低电平(0)时进行写操作。
第6脚：E(或EN)端为使能(enable)端,高电平（1）时读取信息
第7～14脚：D0～D7为8位双向数据端。
*/

//因为单片机执行是有时间延迟的，所以很多时候都要加一个延时的设置
//延迟时间和晶振频率有关，（具体不知道）ps至少延时大于100个执行时间
void delay_ms(int time)
{
	unsigned int time1 = time;
	unsigned int time2 = 100;
	while(time1) time1--;
	while(time2--);
}

BOOL get_busy_status(void)
{
	BOOL result;
	LCM_DATA = 0xff;
	LCM_RS = 0;     //选择指令寄存器
	LCM_RW = 1;    //读操作
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
	result = LCM_DATA & BUSY; //检测忙信号,如果忙碌就一直等待
	return result;
}


//写指令
void write_command(unsigned char WCLCM, busy)//当busy为0的时候不检查是否忙碌
{
	while(get_busy_status());
	LCM_DATA = WCLCM;
	LCM_RS = 0;    //选择指令寄存器
	LCM_RW = 0;    //选择写操作，这里是写操作字
	LCM_EN = 0;		//在这里起一个延时的作用
	delay_ms(50);   //作用和上面其实是一样的
	LCM_EN = 1;    //执行指令
}

//写数据
void write_data(unsigned char WRLCM)
{
	while(get_busy_status());
	LCM_DATA = WRLCM;
	LCM_RS = 1;  //因为是写操作，所以要选择数据寄存器
	LCM_RW = 0;  //写操作
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
}


//读数据
unsigned char read_data(void)
{
	LCM_RS = 1;
	LCM_RW = 1;  //读操作
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
	return LCM_DATA;
}

//初始化
void LCM_init(void)
{
	LCM_DATA = 0;
	LCM_EN = 0;
	/*0x38  00111000  设置16*2显示  5*7点阵  8位数据接口*/  
	write_command(0x38, 0);
	delay_ms(50);

	write_command(0x38, 1);
	delay_ms(50);
	write_command(0x0e, 1); //设置显示光标
	write_command(0x01, 1); //清屏一波
	write_command(0x06, 1); //地址指针自动+1且光标+1 
	write_command(0x80, 1); //设置初始显示位置，这一步好像可以不要，
							//后面肯定还要设置地址。
}


void display_one_char(unsigned char x, unsigned char y, unsigned char data_)
{
	y &= 0x01;
	x &= 0x0f;		//限制x不能大于15
	if (y) x |= 0x40; //要显示第二行的时候
	x |= 0x80;
	write_command(x, 1);
	write_data(data_);
}

int main()
{
	delay_ms(400);
	LCM_init();
	delay_ms(400);
	display_one_char(0, 0, 'c');
	while(1);
	return 0;
}

                