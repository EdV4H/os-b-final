#ifndef _CLIENT_h_
#define _CLIENT_h_

/* ヘッダファイルをインクルードし、必要な変数を用意する */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>

#include "game.h"

/*  必要な定数を定義する  */
#define PLAYER_HEIGHT 12
#define ENEMY_HEIGHT 9

/*  必要な構造体を定義する  */
typedef struct {
    int sockfd;
	int len;
	struct sockaddr_in address;
	int result;
} SocketInfo;

typedef struct {
    char* array;
    int len, index;
} Buffer;

/*  必要な関数を定義する  */
int game_loop (SocketInfo* s, Character* player);

char get_key (Buffer* buf);
char put_key (Buffer* buf);

void draw_rectangle(int x, int y, int w, int h);
void draw_player(Character* p, int x, int y, int w, int h, Buffer* buf);
void draw_enemy(Character* e, int index, int x, int y, int w, int h);
void draw_information(char* info, int x, int y, int w, int h);

void init_character (Character* c);
void init_player (Character* p);
void update_player (Character* p, char* info);
void update_enemy (Character* e, char* info);
void init_curses ();
int connect_server (SocketInfo* s);

#endif