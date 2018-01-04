/**
 * Tcp Server program, It is a simple example only.
 * zhengsh 200520602061 2
 * when client connect to server, send a welcome message and timestamp in server.
 */

#include  <stdio.h> 
#include  <sys/socket.h> 
#include  <unistd.h> 
#include  <sys/types.h> 
#include  <netinet/in.h> 
#include  <stdlib.h> 
#include  <time.h> 
#include <string.h>
#include <thread>
#include <string>
#include <map>
#include <netinet/tcp.h>
using namespace std;

#pragma warning (disable:4996)

typedef void(*ext)();

int PLAYER_SOCKET = -1;
int ROOM_SOCKET = -1;

bool stopRunning = false;


//for debug print time only
void PrintTime() 
{
	time_t t;
	struct tm * lt;
	time(&t);//
	lt = localtime(&t);
	printf("[%d:%d:%d]",  lt->tm_hour, lt->tm_min, lt->tm_sec);
}

//��֤��Ϣ����������
bool recv_unti(int socketFD, char* buffer, int len) 
{
	int total_recved = 0;

	int recv_len = recv(socketFD, buffer + total_recved, len- total_recved, 0);
	if (recv_len <= 0) 
	{
		printf("recv_len<=0:%d\r\n", recv_len);
		return false;
	} 
	total_recved += recv_len;

	while (total_recved < len) 
	{
		recv_len = recv(socketFD, buffer + total_recved, len - total_recved, 0);
		if (recv_len <= 0)
		{
			printf("recv_len1<=0");
			printf("recv_len<=0:%d\r\n", recv_len&0xffff);
			return false;
		}
		total_recved += recv_len;
	}

	return true;

}

bool check_com_data(char* data, int len) {
	if (len < 6) return false;

	//calc the sum
	int check_total = 0;
	for (int i = 0; i < len; i++) {
		if ((i >= 6) && (i < len - 1))
			check_total += (data[i] & 0xff);
	}

	if (data[0] != (unsigned char)((~data[3]) & 0xff)
		&& data[1] != (unsigned char)((~data[4]) & 0xff)
		&& data[2] != (unsigned char)((~data[5]) & 0xff))
		return false;

	//check sum
	if (check_total % 100 != (data[len - 1] & 0xff)) {
		return false;
	}

	return true;
}

void player_recv()
{
	while (stopRunning == false)
	{
		char bu_len[7] = { 0 };
		if(recv_unti(PLAYER_SOCKET, bu_len, 7) == false)
			break;

		int data_len = (unsigned char)bu_len[6];

		if ((bu_len[0] & 0xff) != 0xfe)
			break;

		char* pData = new char[data_len];
		memset(pData, 0, data_len);
		memcpy(pData, bu_len, 7);
		bool is_ok = recv_unti(PLAYER_SOCKET, pData + 7, data_len - 7);
		if (is_ok == false) 
		{
			delete[]pData;
			break;
		}

		PrintTime();
		printf("player_data:");
		for (int i = 0; i < data_len; i++) 
		{
			printf("%02X ", pData[i]&0xff);
		}
		printf("\r\n");

		if (check_com_data(pData, data_len) == false) 
		{
			printf("data check error.\r\n");
			delete[]pData;
			continue;
		}

		//��ҵ��˿��ְ�ť
		 if (pData[7] == 0x31)// new game recv. you should decide whether to grasp or not. here ,i simply translate to the dool machine.
		{
			PrintTime();
			printf("cmd:request new game.tranlating to room ..\r\n");

			if (ROOM_SOCKET != -1) //�����޻� ֱ��ת�������޻�
			{
				send(ROOM_SOCKET, pData, data_len, 0);//send to the doll machine.let it begin!
			}

			send(PLAYER_SOCKET, pData, data_len, 0);//send back to the player to enable the gui button. 
			//In fact you should wait the reply from the doll machine . Translate it's replay to player. You should check to make sure the doll machine is ok.
		}
		else if (ROOM_SOCKET != -1)//�κ���Ϣ��ֱ��ת�������޻�.����ʵ�ʳ�����Ҫ�����¼ ��ȡ�����б�ȵ�һ�Ѷ�������Щ�����ǲ���ת�������޻�������Ҫ���Լ�����
		{
			PrintTime();
			printf(" tranlating to room ..\r\n");

			send(ROOM_SOCKET, pData, data_len, 0);//operation from the player(APP). translate to the doll machine directly.
			//well in real scene ,the player cmd is much more than operation cmd. you should handle it by yourself. Not in this simple server!
		}

		 delete[]pData;
	}

	printf("\nplayer close.\n");
	close(PLAYER_SOCKET); PLAYER_SOCKET = -1;
}

//�������޻���������Ϣ
void room_recv()
{
	while (stopRunning == false)
	{
		char bu_len[7] = { 0 };
		if (recv_unti(ROOM_SOCKET, bu_len, 7) == false) 
		{
			printf("recv head error.");
			break;
		}


		printf("room_head:");
		for (int i = 0; i < 7; i++)
		{
			printf("%02X ", bu_len[i]&0xff);
		}

		printf("\r\n");


        	if((bu_len[0] &0xff) != 0xfe)
            	break;

		int data_len = (unsigned char)bu_len[6];

		char* pData = new char[data_len];
		memset(pData, 0, data_len);
		memcpy(pData, bu_len, 7);


		bool is_ok = recv_unti(ROOM_SOCKET, pData + 7, data_len - 7);
		if (is_ok == false)
		{
			delete[]pData;
			break;
		}

		PrintTime();
		printf("room_data:");
		for (int i = 0; i < data_len; i++)
		{
			printf("%02X ", pData[i]&0xff);
		}

		printf("\r\n");

		printf("cmd:%02X\r\n", pData[7]);

		if (check_com_data(pData, data_len) == false)
		{
			printf("data check error.Room Close\r\n");
			delete[]pData;pData = NULL;
			break;
		}

		//�����������ֱ��ԭ������
		if (pData[7] == 0x35)//heartbeat msg from the doll machine. you should flag this server as 'live'. More than 30s is dead...
		{
			//pData[8] - pData[19];
			//pData[20] = 0;

			//char* pStr = pData + 8;

			//printf("mac recv is:%s\r\n", pStr);


			send(ROOM_SOCKET, pData, data_len, 0);

		}
		else if (PLAYER_SOCKET != -1)//�����������ң��Ͱ��κ���Ϣת�������
		{//ʵ��Ӧ���У��㻹�ô�����Ϻ���Ϸ������Ļص���Ϣ��������ץ������..�ȵ���Ϣ�����Բ���ֻ������ת�������
			//������ֻ����ʾ���������߼�
			printf(" tranlating to client.\n");
			send(PLAYER_SOCKET, pData, data_len, 0);
		}

		delete[]pData;
	}

	PrintTime();
	printf("\nRoom close.\r\n");
	close(ROOM_SOCKET);
	ROOM_SOCKET = -1;
}

int g_doll_listen = -1;
int g_player_listen = -1;

void Comm(ext func, int Port)
{
	int  servfd, clifd;
	struct  sockaddr_in servaddr, cliaddr;

	if ((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		PrintTime();
		printf("create socket error!\n ");
		exit(1);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(Port);
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(servfd, (struct  sockaddr *) & servaddr, sizeof(servaddr)) < 0)
	{
		PrintTime();
		printf("bind to port %d failure!\n", Port);
		exit(1);
	}

	if (listen(servfd, 10) < 0)
	{
		PrintTime();
		printf("call listen failure!\n");
		exit(1);
	}

	if (Port == 7770) g_doll_listen = servfd;
	else if (Port == 7771) g_player_listen = servfd;

	PrintTime();
	printf("start listen at port: %d !\n", Port);
	while (stopRunning == false)
	{ // server loop will nerver exit unless any body kill the process 

		long  timestamp;
		socklen_t length = sizeof(cliaddr);
		clifd = accept(servfd, (struct  sockaddr *) & cliaddr, &length);

		//int on = 1;
		//setsockopt(clifd, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));
		//setsockopt(clifd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

		if (clifd < 0)
		{
			PrintTime();
			printf(" error comes when call accept!\n");
			break;
		}

		if (Port == 7771) //another client connected. by we only support one client in this simple server. 
			//so we close last player.You should write your own server code to support many player...
		{
			if (PLAYER_SOCKET != -1)
			{
				close(PLAYER_SOCKET);
				PLAYER_SOCKET = -1;
			}

			PLAYER_SOCKET = clifd;
		}
		else if (Port == 7770)//Same. This simple server is functionally demo. So ,it's not support many doll to make logic simple ,close the old doll machine.
		{//write you own server code to support many doll machine.
			if (ROOM_SOCKET != -1)
			{
				PrintTime();
				printf("Another room online Close Old Room.\r\n");
				close(ROOM_SOCKET);
				ROOM_SOCKET = -1;
			}

			printf("room connect.\r\n");
			ROOM_SOCKET = clifd;
		}

		thread th_recv(func);
		th_recv.detach();

	} // exit 

	close(servfd);
}


int main(int argc, char** argv)
{
	thread th_listen_room(Comm, room_recv, 7770);
	th_listen_room.detach();

	thread th_listen_player(Comm, player_recv, 7771);
	th_listen_player.detach();

	while (1)
	{
		char chIn[20] = { 0 };

		fgets(chIn, 20, stdin);
		printf("input is:%s\n", chIn);
		if (strstr(chIn, "exit") != NULL)
		{
			if (ROOM_SOCKET != -1) 
			{
				close(ROOM_SOCKET); ROOM_SOCKET = -1;
			}

			if (PLAYER_SOCKET != -1) 
			{
				close(PLAYER_SOCKET); PLAYER_SOCKET = -1;
			}

			close(g_doll_listen);
			close(g_player_listen);
			stopRunning = true;
			printf(" stop running now");
			break;
		}
	}

	return   0;
}
