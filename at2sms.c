// author blueicesir@gmail.com
// test on raspberrypi debian wheezy.
// initial version 2013/03/07
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<errno.h>
#include<string.h>
#include<time.h>
#include<iconv.h>

int put2Serial(int fd,char* atcmd)
{
	printf("SEND:%s\n",atcmd);
	tcflush(fd,TCIFLUSH);
	return write(fd,atcmd,strlen(atcmd));
}



int get4Serial(int fd)
{
	char respbuff[1024];
	bzero(respbuff,sizeof(respbuff));
	int readlen=read(fd,respbuff,sizeof(respbuff));
	if(readlen!=0)
	{
		respbuff[readlen+1]=0x0;
		char outbuff[1024]={0};
		bzero(outbuff,sizeof(outbuff));
		int tag=0;
		int l=0;
		while(l<strlen(respbuff))
		{
			if(respbuff[l]!=0x0d&&respbuff[l]!=0x0a)
			{
				outbuff[tag]=respbuff[l];
				tag++;
			}
			l++;
		}
		printf("RECV:%s\n",outbuff);
	}
	else
	{
		printf("RECV-ZERO\n");
	}
	return readlen;
}

int main()
{
	int fd;
	struct termios options;

	fd = open("/dev/ttyUSB1", O_RDWR);
	// fcntl(fd, F_SETFL, 0);

	tcgetattr(fd, &options);
	options.c_cflag &= ~PARENB;	// 无奇偶校验
	options.c_cflag &= ~CSTOPB;	// 1位停止位
	options.c_cflag &= ~CSIZE;	// 数据掩码
	options.c_cflag |= CS8;	// 8位数据位
	options.c_lflag &=~(ICANON | ECHO | ECHOE |ISIG);	// 标准输入输出，不回显，不转换特殊控制字符
	options.c_cc[VMIN]  = 0;
	options.c_cc[VTIME] = 50;	// Time 5 second

	cfsetispeed(&options,B460800);	// 设置输入速率
	cfsetospeed(&options,B460800);	// 设置输出速率

	tcsetattr(fd, TCSANOW, &options);	// 修改终端属性，立即生效


	// 初始化Modem
	put2Serial(fd,"ATZ\r");
	get4Serial(fd);

	put2Serial(fd,"ATE0\r");
	get4Serial(fd);


	// 获取IMEI号
	put2Serial(fd,"AT+CIMI\r");
	get4Serial(fd);

	// 设置短信中心号码
	put2Serial(fd,"AT+CSCA=\"+8613010888500\"\r");
	get4Serial(fd);

	// 设置发送短信格式TEXT模式
	put2Serial(fd,"AT+CMGF=1\r");
	get4Serial(fd);

	// put2Serial(fd,"AT&V\r");
	// get4Serial(fd);

	// 设置接收短信的号码
	put2Serial(fd,"AT+CMGS=\"18675330000\"\r");
	get4Serial(fd);

	char message[140];
	bzero(message,sizeof(message));
	time_t timer;
	struct tm* stime;
	timer=time(NULL);
	stime=gmtime(&timer);
	sprintf(message,"RaspberryPi Booting on %04d-%02d-%02d %02d:%02d:%02d ...\r",stime->tm_year+1900,stime->tm_mon,stime->tm_mday,(stime->tm_hour+8)%24,stime->tm_min,stime->tm_sec);
	put2Serial(fd,message);
	get4Serial(fd);

	char sEnd[5]={0};
	sEnd[0]=0x1a;
	put2Serial(fd,sEnd);
	get4Serial(fd);

	close(fd);
}
