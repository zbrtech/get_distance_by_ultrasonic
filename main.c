#include <AT89x51.H>		//���������ļ�
#include <intrins.h>

#define TRIG P1_1	//�ֱ������������ںͽ��ܿ����ñ�����
#define ECHO P1_2	//����11 12���ſ������ʹ�ã�ֻҪ��P1~P3���IO�ھ���

//�����Ƕ�1602��ʾƵ���ŵĶ���
//�����Ϊ�̶�����Ϊ���ǽӵľ������һ���Ǹ��ӿڣ�
//����������Ҫ�鿴һ�¿������ԭ��ͼ
#define LCM_EN P0_5  //��ʾ����ʹ�ܶ�
#define LCM_RS P0_7	 //�Ĵ���ѡ��
#define LCM_RW P0_6  //��дѡ����
#define LCM_DATA P2	 //����ѡ���ߣ�����P20-P28������ѡ���

#define BUSY 0x80   //�������LCM�Ƿ���busy��ʾ����ȡBF�ֶε�ֵ
typedef bit BOOL;

/*
1602���������Ź��ܣ�
��1�ţ�VSSΪ��Դ��
��2�ţ�VCC��5V��Դ����
��4�ţ�RSΪ�Ĵ���ѡ�񣬸ߵ�ƽ1ʱѡ�����ݼĴ������͵�ƽ0ʱѡ��ָ��Ĵ�����
��5�ţ�RWΪ��д�ź��ߣ��ߵ�ƽ(1)ʱ���ж��������͵�ƽ(0)ʱ����д������
��6�ţ�E(��EN)��Ϊʹ��(enable)��,�ߵ�ƽ��1��ʱ��ȡ��Ϣ
��7��14�ţ�D0��D7Ϊ8λ˫�����ݶˡ�
*/

//��Ϊ��Ƭ��ִ������ʱ���ӳٵģ����Ժܶ�ʱ��Ҫ��һ����ʱ������
//�ӳ�ʱ��;���Ƶ���йأ������岻֪����ps������ʱ����100��ִ��ʱ��
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
	LCM_RS = 0;     //ѡ��ָ��Ĵ���
	LCM_RW = 1;    //������
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
	result = LCM_DATA & BUSY; //���æ�ź�,���æµ��һֱ�ȴ�
	return result;
}


//дָ��
void write_command(unsigned char WCLCM, busy)//��busyΪ0��ʱ�򲻼���Ƿ�æµ
{
	while(get_busy_status());
	LCM_DATA = WCLCM;
	LCM_RS = 0;    //ѡ��ָ��Ĵ���
	LCM_RW = 0;    //ѡ��д������������д������
	LCM_EN = 0;		//��������һ����ʱ������
	delay_ms(50);   //���ú�������ʵ��һ����
	LCM_EN = 1;    //ִ��ָ��
}

//д����
void write_data(unsigned char WRLCM)
{
	while(get_busy_status());
	LCM_DATA = WRLCM;
	LCM_RS = 1;  //��Ϊ��д����������Ҫѡ�����ݼĴ���
	LCM_RW = 0;  //д����
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
}


//������
unsigned char read_data(void)
{
	LCM_RS = 1;
	LCM_RW = 1;  //������
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
	return LCM_DATA;
}

//��ʼ��
void LCM_init(void)
{
	LCM_DATA = 0;
	LCM_EN = 0;
	/*0x38  00111000  ����16*2��ʾ  5*7����  8λ���ݽӿ�*/  
	write_command(0x38, 0);
	delay_ms(50);

	write_command(0x38, 1);
	delay_ms(50);
	write_command(0x0e, 1); //������ʾ���
	write_command(0x01, 1); //����һ��
	write_command(0x06, 1); //��ַָ���Զ�+1�ҹ��+1 
	write_command(0x80, 1); //���ó�ʼ��ʾλ�ã���һ��������Բ�Ҫ��
							//����϶���Ҫ���õ�ַ��
}


void display_one_char(unsigned char x, unsigned char y, unsigned char data_)
{
	y &= 0x01;
	x &= 0x0f;		//����x���ܴ���15
	if (y) x |= 0x40; //Ҫ��ʾ�ڶ��е�ʱ��
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

                