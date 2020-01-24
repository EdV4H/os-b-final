/*  ヘッダファイルをインクルードし、必要な変数を用意する */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "define.h"

#define FOR_ALL_PLAYER for (int i=0; i<P_NUM; i++)
#define P_NUM 2

#define STUN_FACT 0.3
#define EX_FACT 5.0

typedef struct {
	int sockfd, len;
	struct sockaddr_in address;
} IPinfo;

char* gen_info_message (Information *info);
void add_info (Information* info, char* str);
char* gen_player_msg (Player* p);
char* gen_enemy_msg (Player* p);
void init_player (Player* p);
void update_player (Player* p, Player* e, Information* info);

int main (void) {
	IPinfo server, client[P_NUM];

	/* サーバ用に名前のないソケットを用意する */
	server.sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* bindを呼び出して、ソケットに名前をつける */
	/* 127.0.0.1はループバックアドレス（ローカルマシンとの通信だけに制限） */
	server.address.sin_family = AF_INET;
	server.address.sin_addr.s_addr = htonl(INADDR_ANY);
	server.address.sin_port = htons(9734);
	server.len = sizeof(server.address);
	bind(server.sockfd, (struct sockaddr *)&server.address, server.len);

	/* 接続キューを生成し、クライアントからの接続を待つ */
	listen(server.sockfd, 5);

	Player player[P_NUM];
	Information information;
	for (int i=0; i<5; i++) strcpy(information.content[i], "_");
	int flag = 1;

	FOR_ALL_PLAYER {
		char str[20];
		int rc;

		printf("Player %d waiting ...\n", i);
		/* 接続を受け入れる */		
		client[i].len = sizeof(client[i].address);
		client[i].sockfd = accept(server.sockfd, (struct sockaddr *)&client[i].address, &client[i].len);
		/* 初期情報を取得する */
		rc = read(client[i].sockfd, &str, 10);
		str[rc] = '\0';
		strcpy(player[i].name, str);
		rc = read(client[i].sockfd, &str, 12);
		str[rc] = '\0';
		player[i].status.hp = atoi(str);
		printf("[ Player[%d] ]\nName : %s\nHp : %d\n", i, player[i].name, player[i].status.hp);
		init_player(&(player[i]));
	}

	/* （2人用仮）ターゲットをお互いに設定 */
	player[0].target = 1;
	player[1].target = 0;

	add_info (&information, "GAME_START!!!");

	/* sockfdを介して読み書きを行う */
	while (flag) {
		/* 終了判定 */
		FOR_ALL_PLAYER {
			int win = 1;
			for(int j=0; j<P_NUM; j++) {
				if (i != j && player[j].status.dead != 1) win = 0;
			}
			if (win) {
				char msg[50];
				sprintf(msg, "THE_WINNER_IS_%s!!!!", player[i].name);
				add_info(&information, "_");add_info(&information, "_");
				add_info(&information, msg);
				add_info(&information, "_");add_info(&information, "_");
				flag = 0;
			}
		}
		FOR_ALL_PLAYER {
			write(client[i].sockfd, gen_player_msg(&(player[i])), 100);
			for (int j=0; j<P_NUM; j++) {
				if (i!=j) write(client[i].sockfd, gen_enemy_msg(&(player[j])), 100);
			}
			/* インフォメーション */
			//printf("%s\n", gen_info_message(&(information)));
			write(client[i].sockfd, gen_info_message(&(information)), 250);
		} 
		FOR_ALL_PLAYER {
			read(client[i].sockfd, player[i].next_cmd, 10);
			//printf("receive cmd : %s\n", player[i].next_cmd);
			update_player(&(player[i]), &(player[player[i].target]), &information);
		}
		/* 処理は1秒毎に行う */
		sleep(1);
	}
	FOR_ALL_PLAYER close(client[i].sockfd);
}

char* gen_info_message (Information *info) {
	char *msg = (char *)malloc(250*sizeof(char));
	sprintf(msg, "%s %s %s %s %s", info->content[0], info->content[1], info->content[2], info->content[3], info->content[4]);
	return msg;
}

void add_info (Information* info, char* str) {
	for (int i=4; i>0; i--) {
		strcpy(info->content[i], info->content[i-1]);
	}
	strcpy(info->content[0], str);
	//printf("add : %s\ninfo index : %d\n", str, info->index);
}

char* gen_player_msg (Player* p) {
	char *msg = (char *)malloc(100*sizeof(char));
	sprintf(msg, "%s %d %s %d %d %d %d %d %d %d",
		p->name, p->status.hp, p->cmd, p->wait,
		p->a_ct, p->s_ct, p->d_ct, p->z_ct, p->x_ct, p->c_ct);
	return msg;
}

char* gen_enemy_msg (Player* p) {
	char *msg = (char *)malloc(100*sizeof(char));
	sprintf(msg, "%s %d %d %s %d",
		p->name, p->status.hp, p->status.max_hp, p->cmd, p->wait);
	return msg;
}

void init_player (Player* p) {
	p->status.max_hp = p->status.hp;
	p->status.atk = 10;
	p->status.atk_up = 1.0;
	p->a_ct=0;p->s_ct=0;p->d_ct=0;p->z_ct=0;p->x_ct=0;p->c_ct=0;
	p->wait = -1; // 初期値は待機状態
	p->status.stun = 0;
	p->status.dead = 0;
	strcpy(p->cmd, "NULL");
	return ; 
}

void update_player (Player* p, Player* e, Information* info) {
	if (p->status.dead == 1) return; //死んでいるなら更新しない
	if (p->status.stun > 0) {
		/* スタン状態 */
		p->status.stun--;
	}
	else if (p->wait == -1) {
		/* コマンドを受け付ける */
		strcpy(p->cmd, p->next_cmd);
		if (strcmp(p->cmd, "ATTACK")==0 && p->a_ct==0) {
			p->wait = 4;
			p->a_ct = 10;
		}
		else if (strcmp(p->cmd, "STUN")==0 && p->s_ct==0) {
			p->wait = 3;
			p->s_ct = 15;
		}
		else if (strcmp(p->cmd, "DEFENCE")==0 && p->d_ct==0) {
			p->wait = 2;
			p->d_ct = 10;
		}
		else if (strcmp(p->cmd, "ZONE")==0 && p->z_ct==0) {
			p->wait = 4;
			p->z_ct = 10;
		}
		else if (strcmp(p->cmd, "EXTREME")==0 && p->x_ct==0) {
			p->wait = 9;
			p->x_ct = 30;
		}
		else if (strcmp(p->cmd, "CHARGE")==0 && p->c_ct==0) {
			p->wait = 3;
			p->c_ct = 10;
		}

		/* コマンドを消去 */
		strcpy(p->next_cmd, "NULL");
	}
	/* 1ウエイト進める */
	else if (p->wait > 0) p->wait--;
	if (p->a_ct>0) p->a_ct--;
	if (p->s_ct>0) p->s_ct--;
	if (p->d_ct>0) p->d_ct--;
	if (p->z_ct>0) p->z_ct--;
	if (p->x_ct>0) p->x_ct--;
	if (p->c_ct>0) p->c_ct--;

	if (p->wait == 0) {
		printf("Player action : %s\n", p->cmd);

		if (strcmp(p->cmd, "ATTACK")==0) {
			int dmg = p->status.atk * p->status.atk_up;
			e->status.hp -= dmg;
			char msg[50];
			add_info(info, "_");
			sprintf(msg, "%s_hp_decreased_by_%d.", e->name, dmg);
			add_info(info, msg);
			sprintf(msg, "%s_attacks!", p->name);
			add_info(info, msg);
		}
		else if (strcmp(p->cmd, "STUN")==0) {
			int dmg = p->status.atk * p->status.atk_up * STUN_FACT;
			e->status.hp -= dmg;
			e->status.stun = 3;
			strcpy(e->cmd, "NULL");
			e->wait = -1;
			char msg[50];
			add_info(info, "_");
			sprintf(msg, "%s_hp_decreased_by_%d_and_stuned!!", e->name, dmg);
			add_info(info, msg);
			sprintf(msg, "%s_uses_stun!", p->name);
			add_info(info, msg);
		}
		else if (strcmp(p->cmd, "DEFENCE")==0) {

		}
		else if (strcmp(p->cmd, "ZONE")==0) {
			
		}
		else if (strcmp(p->cmd, "EXTREME")==0) {
			int dmg = p->status.atk * p->status.atk_up * EX_FACT;
			e->status.hp -= dmg;
			char msg[50];
			add_info(info, "_");
			sprintf(msg, "%s_hp_decreased_by_%d.", e->name, dmg);
			add_info(info, msg);
			sprintf(msg, "%s_uses_extrem_attack!", p->name);
			add_info(info, msg);
		}
		else if (strcmp(p->cmd, "CHARGE")==0) {
			p->status.atk_up *= 1.5;
		}

		if (e->status.hp < 1) {
			e->status.hp = 0;
			e->status.dead = 1;
			char msg[50];
			add_info(info, "_");
			sprintf(msg, "%s_is_dead.", e->name);
			add_info(info, msg);
		}

		/* コマンドを消去 */
		strcpy(p->cmd, "NULL");
		p->wait--;
	}
}