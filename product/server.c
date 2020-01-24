#include "server.h"

int main() {
	SocketInfo server;

	init_server(&server);

	while(1) {
		/* sockfdを介して読み書きを行う */
		game_loop(&server);
	}
}

int game_loop (SocketInfo* s) {
	SocketInfo client[PLAYER_NUM];
	Character player[PLAYER_NUM];
	Information information;
	information.index=0;
	for (int i=0; i<20; i++) strcpy(information.msg[i], " ");
	int endflg = 0;
	char cmd;
	char i_info[I_INFO_SIZE], p_info[PLAYER_NUM][P_INFO_SIZE];

	/* 接続を受け入れる */
	FOR_ALL_PLAYER accept_player(s->sockfd, i, &(client[i]), &(player[i]));

	/*  初期ターゲットの設定  */
	FOR_ALL_PLAYER {
		if (i == 0) player[i].target = PLAYER_NUM-1;
		else player[i]. target = i-1;
	}

	/*  ゲーム開始をプレイヤーに伝達  */
	add_info_msg(&information, "[ *** GAME START *** ]");

	while (endflg == 0) {
		FOR_ALL_PLAYER {
			/*  コマンド情報を取得する  */
			read(client[i].sockfd, &cmd, 1);
			/*  情報を元にプレイヤーを更新する  */
			update_player(player, i, cmd, &information);
		}

		/*  送信する情報を作成する  */
		strcpy(i_info, gen_info_msg(&information));
		FOR_ALL_PLAYER strcpy(p_info[i], gen_player_msg(&(player[i])));

		FOR_ALL_PLAYER {
			/*  インフォメーションを送信する  */
			write(client[i].sockfd, i_info, I_INFO_SIZE-1);

			/*  プレイヤー情報を送信する  */
			write(client[i].sockfd, p_info[i], P_INFO_SIZE-1);

			/*  敵情報を送信する  */
			for (int j=0; j<PLAYER_NUM; j++) {
				if (i!=j) write(client[i].sockfd, p_info[j], P_INFO_SIZE-1);
			}

		}

		/* 処理は定義秒毎に行う */
		sleep(SEC_OF_TURN);
	}

	FOR_ALL_PLAYER close(client[i].sockfd);
}

void update_player (Character p[PLAYER_NUM], int p_index, char cmd, Information* info) {
	int target;
	char msg[50];

	/*  クールタイム処理  */
	for (int i=0; i<6; i++) if (p[p_index].cool_time[i] > 0) p[p_index].cool_time[i]--;

	/*  バフ処理  */
	if (p[p_index].buff_atk.time > 0) {
		p[p_index].buff_atk.time--;
		if (p[p_index].buff_atk.time <= 0) p[p_index].buff_atk.level = 0;
	}
	if (p[p_index].buff_wait.time > 0) {
		p[p_index].buff_wait.time--;
		if (p[p_index].buff_wait.time <= 0) p[p_index].buff_wait.level = 0;
	}

	/*  フィーバー処理  */
	if (p[p_index].fever == 0 && p[p_index].fever_time <= FEVER_TIME) p[p_index].fever_time++;
	if (p[p_index].fever == 1 && p[p_index].fever_time > 0) {
		p[p_index].fever_time -= 2;
		if (p[p_index].fever_time <= 0) p[p_index].fever = 0;
	}

	/*  コマンド処理  */

	/*  ダメージ補正の計算  */
	double magnification = BASE_MAGNIFICATION;
	if (p[p_index].fever == 1) magnification *= 2.0;
	if (p[p_index].buff_atk.level != 0) magnification *= p[p_index].buff_atk.level * p[p_index].buff_atk.amount;

	if (p[p_index].stun > 0) {
		p[p_index].stun--; return ;
	}
	if (p[p_index].wait > 0) p[p_index].wait--;
	if (p[p_index].wait == 0) {
		/*  コマンドを実行する  */
		// sprintf(msg, "%s use %s", p[p_index].name, p[p_index].cmd);
		// add_info_msg(info, msg);
		if (strcmp(p[p_index].cmd, "ATTACK")==0) {
			int dmg = damage(&(p[p_index]), &(p[p[p_index].target]), magnification);
			sprintf(msg, "[ATTACK] %s -> %s (%d dmg)", p[p_index].name, p[p[p_index].target].name, dmg);
			add_info_msg(info, msg);
		}
		if (strcmp(p[p_index].cmd, "STUN")==0) {
			int dmg = damage(&(p[p_index]), &(p[p[p_index].target]), magnification * STUN_MAGNIFICATION);
			p[p[p_index].target].stun = STUN_TIME;
			p[p[p_index].target].wait = -1;
			strcpy(p[p[p_index].target].cmd, "NULL");
			sprintf(msg, "[STUN] %s -> %s (%d dmg)", p[p_index].name, p[p[p_index].target].name, dmg);
			add_info_msg(info, msg);
		}
		if (strcmp(p[p_index].cmd, "DEFENCE")==0) {
			// sprintf(msg, "[DEFENCE] %s", p[p_index].name);
			// add_info_msg(info, msg);
			for (int i=0; i<15; i++)  add_info_msg(info, "debug");
		}
		if (strcmp(p[p_index].cmd, "ZONE")==0) {
			p[p_index].buff_wait.level++;
			p[p_index].buff_wait.time = 60;
			sprintf(msg, "[ZONE] %s", p[p_index].name);
			add_info_msg(info, msg);
		}
		if (strcmp(p[p_index].cmd, "EXTREME")==0) {
			int dmg = damage(&(p[p_index]), &(p[p[p_index].target]), magnification * EX_MAGNIFICATION);
			sprintf(msg, "[EXTREME] %s -> %s (%d dmg)", p[p_index].name, p[p[p_index].target].name, dmg);
			add_info_msg(info, msg);
		}
		if (strcmp(p[p_index].cmd, "CHARGE")==0) {
			p[p_index].buff_atk.level++;
			p[p_index].buff_atk.time = 60;
			sprintf(msg, "[CHARGE] %s", p[p_index].name);
			add_info_msg(info, msg);
		}
		else if (strcmp(p[p_index].cmd, "FEVER")==0) {
			p[p_index].fever = 1;
			sprintf(msg, "[FEVER] %s", p[p_index].name);
			add_info_msg(info, msg);
		}

		/*  コマンドをリセットする  */
		p[p_index].wait = -1;
		strcpy(p[p_index].cmd, "NULL"); 
	}
	else if (p[p_index].wait == -1) {
		/*  waitバフ計算  */
		int minus_wait = 0;
		if (p[p_index].fever == 1) minus_wait += 2;
		if (p[p_index].buff_wait.level != 0) minus_wait += p[p_index].buff_wait.level * p[p_index].buff_wait.amount;
		/*  コマンドを受け付ける  */
		switch (cmd) {
		case 'a':
			if (p[p_index].cool_time[Attack] == 0) {
				strcpy(p[p_index].cmd, "ATTACK");
				p[p_index].wait = ATK_WAIT - minus_wait;
				p[p_index].cool_time[Attack] = ATK_CT;
			}
			break;
		case 's':
			if (p[p_index].cool_time[Stun] == 0) {
				strcpy(p[p_index].cmd, "STUN");
				p[p_index].wait = STUN_WAIT - minus_wait;
				p[p_index].cool_time[Stun] = STUN_CT;
			}
			break;
		case 'd':
			if (p[p_index].cool_time[Defence] == 0) {
				strcpy(p[p_index].cmd, "DEFENCE");
				p[p_index].wait = DEF_WAIT - minus_wait;
				p[p_index].cool_time[Defence] = DEF_CT;
			}
			break;
		case 'z':
			if (p[p_index].cool_time[Zone] == 0) {
				strcpy(p[p_index].cmd, "ZONE");
				p[p_index].wait = ZONE_WAIT - minus_wait;
				p[p_index].cool_time[Zone] = ZONE_CT;
			}
			break;
		case 'x':
			if (p[p_index].cool_time[Extreme] == 0) {
				strcpy(p[p_index].cmd, "EXTREME");
				p[p_index].wait = EX_WAIT - minus_wait;
				p[p_index].cool_time[Extreme] = EX_CT;
			}
			break;
		case 'c':
			if (p[p_index].cool_time[Charge] == 0) {
				strcpy(p[p_index].cmd, "CHARGE");
				p[p_index].wait = CHARGE_WAIT - minus_wait;
				p[p_index].cool_time[Charge] = CHARGE_CT;
			}
			break;
		case 'f':
			if (p[p_index].fever_time >= FEVER_TIME) {
				strcpy(p[p_index].cmd, "FEVER");
				p[p_index].wait = FEVER_WAIT;
			}
			break;
		default :
			target = cmd - '0';
			if (target>=0 && target<PLAYER_NUM) p[p_index].target = target;
			break;
		}
	}

}

int damage (Character* attacker, Character* target, double magnification) {
	/*  ダメージ処理を行う  */
	int dmg = attacker->atk * magnification;
	target->hp -= dmg;
	return dmg;
}

void add_info_msg (Information* info, char* msg) {
	if (info->index >= 20) info->index--;
	for (int i=(info->index); i>0; i--) {
		strcpy(info->msg[i], info->msg[i-1]);
	}
	if (info->index<20) info->index++;
	//printf("add info [index:%d]\n", info->index);
	strcpy(info->msg[0], msg);
}

char* gen_info_msg (Information* info) {
	char *msg = (char *)malloc((P_INFO_SIZE-1)*sizeof(char));
	for(int i=0; i<20; i++) {
		strcat(msg, info->msg[i]);
		strcat(msg, ",");
	}
	printf("%s\n", msg);
	return msg;
}

char* gen_player_msg (Character* p) {
	char *msg = (char *)malloc((P_INFO_SIZE-1)*sizeof(char));
	sprintf(msg, "%s %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d %d %d",
			p->name,
			p->hp, p->max_hp,
			p->atk,
			p->stun,
			p->fever,
			p->target,
			p->cmd,
			p->wait,
			p->cool_time[0],p->cool_time[1],p->cool_time[2],
			p->cool_time[3],p->cool_time[4],p->cool_time[5],
			p->fever_time,
			p->buff_atk.level, p->buff_wait.level
	);
	return msg;
}

void accept_player (int server_sockfd, int index, SocketInfo* s, Character* p) {
	printf("Player waiting ...\n");
	s->len = sizeof(s->address);
	s->sockfd = accept(server_sockfd, (struct sockaddr *)&s->address, &s->len);

	/*  初期情報をクライアントに送る  */
	char e_num = PLAYER_NUM - 1 + '0';
	write(s->sockfd, &e_num, 1);
	char p_id = index + '0';
	write(s->sockfd, &p_id, 1);

	/*  初期情報をクライアントから受け取る  */
	char info[101];
	int rc = read(s->sockfd, info, 100); info[rc] = '\0';
	sscanf(info, "%s %d %d", p->name, &(p->hp), &(p->atk));
	p->max_hp = p->hp;
	p->target = 0;
	p->wait = -1;
	p->stun = 0;
	p->fever = 0;
	p->fever_time = 0;
	strcpy(p->cmd, "NULL");
	for (int i=0; i<6; i++) p->cool_time[i] = 0;
	p->buff_atk.amount = 1.5;
	p->buff_atk.level = 0;
	p->buff_atk.time = 0;
	p->buff_wait.amount = 1;
	p->buff_wait.level = 0;
	p->buff_wait.time = 0;

	printf("[ Player accept! ]\nNAME:%s HP:%d ATK:%d\n", p->name, p->hp, p->atk);
}

int init_server (SocketInfo* s) {
	/* サーバ用に名前のないソケットを用意する */
  
	s->sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* bindを呼び出して、ソケットに名前をつける */
	/* 127.0.0.1はループバックアドレス（ローカルマシンとの通信だけに制限） */

	s->address.sin_family = AF_INET;
	s->address.sin_addr.s_addr = htonl(INADDR_ANY);
	s->address.sin_port = htons(9734);
	s->len = sizeof(s->address);
	bind(s->sockfd, (struct sockaddr *)&s->address, s->len);

	/* 接続キューを生成し、クライアントからの接続を待つ */
	listen(s->sockfd, 5);
}

