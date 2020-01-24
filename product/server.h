#ifndef _SERVER_h_
#define _SERVER_h_

/*  ヘッダファイルをインクルードし、必要な変数を用意する */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "game.h"

/*  必要な定数を定義する  */
#define FOR_ALL_PLAYER for (int i=0; i<PLAYER_NUM; i++)

#define PLAYER_NUM 2

/*  必要な構造体を定義する  */
typedef struct {
    int sockfd;
	int len;
	struct sockaddr_in address;
	int result;
} SocketInfo;

typedef struct {
	char msg[20][50];
	int len, index;
} Information;

/*  使用する関数を定義する  */
int game_loop (SocketInfo* s);

void add_info_msg (Information* info, char* msg);
char* gen_info_msg (Information* info);
char* gen_player_msg (Character* p);


void update_player (Character p[PLAYER_NUM], int p_index, char cmd, Information* info);
int damage (Character* attacker, Character* target, double magnification);
void accept_player (int server_sockfd, int index, SocketInfo* s, Character* p);
//void init_player (Character* p, char* info);
int init_server (SocketInfo* s);

#endif