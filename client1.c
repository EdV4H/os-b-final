/* ヘッダファイルをインクルードし、必要な変数を用意する */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>

#include "define.h"

char *replace_all(char *str, const char *what, const char *with)
{
	if (!*what) { return str; }
	char *what_pos = str;
	const size_t what_len = strlen(what), with_len = strlen(with);
	while ((what_pos = strstr(what_pos, what)) != NULL) {
		const char *remain = what_pos + what_len;
		memmove(what_pos + with_len, remain, strlen(remain) + 1);
		memcpy(what_pos, with, with_len);
	}
	return str;
}

void init_player (Player* p) {
	printf("Input player name : ");
	scanf("%s", p->name);
	p->status.hp = 100;
	p->status.max_hp = p->status.hp;
	p->a_ct=0;p->s_ct=0;p->d_ct=0;p->z_ct=0;p->x_ct=0;p->c_ct=0;
	p->wait = -1; // 初期値は待機状態
	/* スキル設定 */
}

void update_player (Player *p, char* str) {
	char head[5];
	sscanf(str, "%s %d %s %d %d %d %d %d %d %d",
		p->name, &(p->status.hp), p->cmd, &(p->wait),
		&(p->a_ct), &(p->s_ct), &(p->d_ct), &(p->z_ct), &(p->x_ct), &(p->c_ct));
}

void draw_rectangle (int x, int y, int w, int h) {
	move(x,y);
	for (int i=0;i<w;i++) addch('=');
	for (int i=1;i<h-1;i++) {
		mvaddch(x+i,y,'|');
		mvaddch(x+i,y+w-1,'|');
	}
	move(x+h-1,y);
	for (int i=0;i<w;i++) addch('=');
}

void draw_player (char *info, int offset_x, int offset_y) {
	char name[10], cmd[10];
	int hp, max_hp, atk, a_up, target, wait;
	int a_ct, s_ct, d_ct, z_ct, c_ct;
	int wait_buf, stun_debuff;
}

void draw_player (Player p, int offset_x, int offset_y) {
	draw_rectangle(offset_x, offset_y, COLS/2, 8);
	/* プレイヤー情報 */
	mvprintw(offset_x, offset_y+2, "[ PLAYER DATA ]");
	mvprintw(offset_x+1, offset_y+2, "NAME : %s", p.name);
	mvprintw(offset_x+2, offset_y+2, "HP : %d / %d", p.status.hp, p.status.max_hp);
	mvprintw(offset_x+3, offset_y+2, "COMMAND : %s", p.cmd);
	for (int i=0; i<p.wait; i++) mvprintw(offset_x+4, offset_y+i+2, "#");
	mvprintw(offset_x+5, offset_y+2, "A(%d) S(%d) D(%d)", p.a_ct, p.s_ct, p.d_ct);
	mvprintw(offset_x+6, offset_y+2, "Z(%d) X(%d) C(%d)", p.z_ct, p.x_ct, p.c_ct);
}

void draw_enemy (char* info,  int offset_x, int offset_y) {
	draw_rectangle(offset_x, offset_y, COLS/2-1, 6);
	char name[10], cmd[10];
	int hp, max_hp, wait;

	sscanf(info, "%s %d %d %s %d", name, &hp, &max_hp, cmd, &wait);

	mvprintw(offset_x, offset_y+2, "[ ENEMY DATA ]");
	mvprintw(offset_x+1, offset_y+2, "NAME : %s", name);
	mvprintw(offset_x+2, offset_y+2, "HP : %d / %d", hp, max_hp);
	mvprintw(offset_x+3, offset_y+2, "COMMAND : %s", cmd);
	for (int i=0; i<wait; i++) mvprintw(offset_x+4, offset_y+i+2, "#");
}

void draw_info (char* info, int offset_x, int offset_y) {
	draw_rectangle(offset_x, offset_y, COLS, 7);
	char msg[5][50];
	sscanf(info, "%s %s %s %s %s", msg[0], msg[1], msg[2], msg[3], msg[4]);
	mvprintw(offset_x, offset_y+2, "[ INFORMATION ]");
	for (int i=0; i<5; i++) mvprintw(offset_x+1+i, offset_y+2, "%s", replace_all(msg[i], "_", " "));
}

char* put_cmd (char c) {
	if (c == 'a') return "ATTACK";
	else if (c == 's') return "STUN";
	else if (c == 'd') return "DEFENCE";
	else if (c == 'z') return "ZONE";
	else if (c == 'x') return "EXTREME";
	else if (c == 'c') return "CHARGE";
	else return "NULL";
}

int main (void) {
	int sockfd;
	int len;
	struct sockaddr_in address;
	int result;
	char adress[20];

	Player player;

	printf("Input adress : ");
	scanf("%s", adress);

	/* クライアント用にソケットを作成する */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* サーバ側と同じ名前でソケットの名前をつける */
	/* 127.0.0.1はループバックアドレス（ローカルマシンとの通信だけに制限） */
	address.sin_family = AF_INET;
	//address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_addr.s_addr = inet_addr(adress);
	address.sin_port = htons(9734);
	len = sizeof(address);

	/* 初期情報を入力する */
	init_player(&player);

	/* curses */
	initscr();           //curses
	noecho();            //curses
	cbreak();            //curses
	keypad(stdscr,TRUE); //curses

	timeout(0);

	/* クライアントのソケットをサーバのソケットに接続する */
	result = connect(sockfd, (struct sockaddr *)&address, len);

	if (result == -1) {
		perror("oops: client2");
		exit(1);
	}
	
	write(sockfd, player.name, 10);
	char str_hp[12];
	sprintf(str_hp, "%d", player.status.hp);
	write(sockfd, str_hp, 12);

	printw("Waiting for other players...");

	char ch;
	/* sockfdを介して読み書きを行う */
	while ((ch = getch()) != 'q') {
		char p_info[101], e_info[101], i_info[251];
		int rc;
		/* 多重入力防止？ */
		for (int i=0; i<10; i++) getch();
		rc = read(sockfd, p_info, 100);
		p_info[rc] = '\0';
		rc = read(sockfd, e_info, 100);
		e_info[rc] = '\0';
		rc = read(sockfd, i_info, 250);
		i_info[rc] = '\0';
		//addstr(put_cmd(ch));
		//strcpy(player.cmd, put_cmd(ch)); 
		write(sockfd, put_cmd(ch), 10);

		clear();
		//printw("receive msg : %s\n", p_info);
		update_player(&player, p_info);
		draw_player(&player, 0, 0);
		draw_enemy(e_info, 0, COLS/2+1);
		draw_info(i_info, LINES-7, 0);
		move(0, 0);
	}
	close(sockfd);
	endwin(); //curses
	exit(0);
}
