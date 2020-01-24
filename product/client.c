#include "client.h"

int main()
{
	SocketInfo client;
	Character player;

	init_player(&player);
	connect_server(&client);

	/* sockfdを介して読み書きを行う */
	game_loop(&client, &player);

	close(client.sockfd);
	exit(0);
}

int game_loop (SocketInfo* s, Character* player) {
	init_curses();

	int rc, e_num;
	char key, p_info[101], e_info[101], i_info[1001];
	Character *enemys;
	Buffer buffer;
	buffer.index = 0;
	buffer.len = COLS/2-13;
	buffer.array = (char *)malloc(buffer.len*sizeof(char));
	char cmds[] = {'a','s','d','z','x','c','f'};

	strcpy(i_info, "Waiting other players ..., , , , , , , , , , , , , , , , , , , ,");

	/*  初期情報をサーバから受け取る  */
	char ch;
	rc = read(s->sockfd, &ch, 1);
	e_num = ch - '0';
	enemys = (Character *)malloc(e_num*sizeof(Character));
	rc = read(s->sockfd, &ch, 1);
	player->id = ch - '0';


	/*  初期情報をサーバに送る  */
	sprintf(p_info, "%s %d %d", player->name, player->hp, player->atk);
	write(s->sockfd, p_info, 100);

	/*  画面に情報を描画する  */
	draw_player(player, 0, 0, COLS/2, PLAYER_HEIGHT, &buffer);
	for (int i=0; i<e_num; i++) {
		init_character(&(enemys[i]));
		draw_enemy(&(enemys[i]), i+1, ENEMY_HEIGHT*i, COLS/2+1, COLS/2-1, ENEMY_HEIGHT);
	}
	draw_information(i_info, PLAYER_HEIGHT, 0, COLS/2, LINES-PLAYER_HEIGHT);
	move(0,0);

	while (1) {
		for (int i=0; i<buffer.len; i++) if (get_key(&buffer)=='q') break;
		/*  キー情報を出力  */
		key = put_key(&buffer);
		write(s->sockfd, &key, 1);

		/*  インフォメーションを取得する  */
		rc = read(s->sockfd, i_info, 1000); i_info[rc] = '\0';
		
		/*  プレイヤー情報を取得する  */
		rc = read(s->sockfd, p_info, 100); p_info[rc] = '\0';
		update_player(player, p_info);

		/*  敵情報を取得する  */
		for (int i=0; i<e_num; i++) {
			rc = read(s->sockfd, e_info, 100); e_info[rc] = '\0';
			update_enemy(&(enemys[i]), e_info);
		}

		/*  画面に情報を描画する  */
		clear();
		draw_player(player, 0, 0, COLS/2, PLAYER_HEIGHT, &buffer);
		for (int i=0; i<e_num; i++) {
			int id = i;
			if (i >= player->id) id = i+1;
			draw_enemy(&(enemys[i]), id, ENEMY_HEIGHT*i, COLS/2+1, COLS/2-1, ENEMY_HEIGHT);
		}
		draw_information(i_info, PLAYER_HEIGHT, 0, COLS/2, LINES-PLAYER_HEIGHT);
		move(0,0);
	}

	endwin(); //curses
}

char get_key (Buffer* buf) {
	char ch = getch();
	if (ch == -1) return ch;
	else if (ch == 'e') {
		if (buf->index > 0) buf->index--;
		return ch;
	}
	else if (ch == 'r') {
		buf->index = 0;
		return ch;
	}
	if (buf->index >= buf->len) {
		for (int i=0; i<buf->len-1; i++) {
			buf->array[i] = buf->array[i+1];
		}
		buf->index--;
	}
	buf->array[buf->index] = ch;
	buf->index++;
	return ch;
}

char put_key (Buffer* buf) {
	if (buf->index == 0) return ' ';
	char ch = buf->array[0];
	for (int i=0; i<buf->index-1; i++) {
		buf->array[i] = buf->array[i+1];
	}
	if (buf->index > 0) buf->index--;
	return ch;
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

void draw_player(Character* p, int x, int y, int w, int h, Buffer* buf) {
	draw_rectangle(x,y,w,h);
	mvprintw(x, y+2, "[ PLAYER DATA ]");
	mvprintw(x+1, y+2, "NAME : %s (id=%d)", p->name, p->id);
	mvprintw(x+2, y+2, "HP:[");
	int hp_len = p->hp * (w-14) / p->max_hp;
	for (int i=0; i<hp_len; i++) mvprintw(x+2, y+6+i, "=");
	mvprintw(x+2, y+w-3-7, "]%3d/%3d", p->hp, p->max_hp);
	mvprintw(x+3, y+2, "ATK : %d", p->atk);
	mvprintw(x+4, y+2, "STATUS : ");
	if (p->stun > 0) printw("STUNNED!! ");
	if (p->fever == 1) printw("FEVER!!! ");
	if (p->buff_atk.level > 0) printw("A_UP*%d ", p->buff_atk.level);
	if (p->buff_wait.level > 0) printw("W_UP*%d ", p->buff_wait.level);
	mvprintw(x+5, y+2, "TARGET : %d", p->target);
	mvprintw(x+6, y+2, "COMMAND : %s", p->cmd);
	for (int i=0; i<(p->wait); i++) mvaddch(x+7, y+2+i, '#');
	mvprintw(x+8, y+2, "A[%2d] S[%2d] D[%2d] Z[%2d] X[%2d] C[%2d]",
			p->cool_time[0], p->cool_time[1], p->cool_time[2],
			p->cool_time[3], p->cool_time[4], p->cool_time[5]
	);
	int fever_len = p->fever_time * (w-12) / FEVER_TIME;
	mvprintw(x+9, y+2, "FEVER:[");
	for (int i=0; i<fever_len; i++) printw("=");
	mvprintw(x+9, y+w-3, "]");
	mvprintw(x+10, y+2, "BUFFER:[");
	for (int i=0; i<buf->index; i++) printw("%c", buf->array[i]);
	mvprintw(x+10, y+w-3, "]");
}

void draw_enemy(Character* e, int index, int x, int y, int w, int h) {
	draw_rectangle(x,y,w,h);
	mvprintw(x, y+2, "[ ENEMY(%d) DATA ]", index);
	mvprintw(x+1, y+2, "NAME : %s", e->name);
	mvprintw(x+2, y+2, "HP:[");
	int hp_len = e->hp * (w-14) / e->max_hp;
	for (int i=0; i<hp_len; i++) mvprintw(x+2, y+6+i, "=");
	mvprintw(x+2, y+w-3-7, "]%3d/%3d", e->hp, e->max_hp);
	mvprintw(x+3, y+2, "ATK : %d", e->atk);
	mvprintw(x+4, y+2, "STATUS : ");
	if (e->stun > 0) printw("STUNNED!! ");
	if (e->fever == 1) printw("FEVER!!! ");
	mvprintw(x+5, y+2, "TARGET : %d", e->target);
	mvprintw(x+6, y+2, "COMMAND : %s", e->cmd);
	for (int i=0; i<(e->wait); i++) mvaddch(x+7, y+2+i, '#');
}

void draw_information(char* info, int x, int y, int w, int h) {
	char msg[20][50];
	sscanf(info, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],",
			msg[0],msg[1],msg[2],msg[3],msg[4],msg[5],
			msg[6],msg[7],msg[8],msg[9],msg[10],
			msg[11],msg[12],msg[13],msg[14],msg[15],
			msg[16],msg[17],msg[18],msg[19]
	);

	draw_rectangle(x,y,w,h);
	mvprintw(x, y+2, "[ INFORMATION ]");
	int len = h-2; if (len > 20) len = 20;
	for (int i=0; i<len; i++) mvprintw(x+1+i, y+2, "%s", msg[i]);
}

void init_character (Character* c) {
	strcpy(c->name, "NONAME");
	c->hp = 100;
	c->atk = 10;
	c->max_hp = c->hp;
	c->target = 0;
	c->wait = -1;
	c->stun = 0;
	c->fever = 0;
	c->fever_time = 0;
	strcpy(c->cmd, "NULL");
	for (int i=0; i<6; i++) c->cool_time[i] = 0;
	c->buff_atk.amount = 1.5;
	c->buff_atk.level = 0;
	c->buff_atk.time = 0;
	c->buff_wait.amount = 1;
	c->buff_wait.level = 0;
	c->buff_wait.time = 0;
}

void init_player (Character* p) {
	char auto_init;
	init_character(p);
	printf("Input player name (without \",\" & \" \") : ");
	scanf("%s", p->name);
	getc(stdin);
	printf("Auto initialize ? <y/n> ");
	auto_init = getc(stdin);
	getc(stdin);
	if (auto_init == 'n') {
		printf("Input player hp : ");
		scanf("%d", &(p->hp));
		getc(stdin);
		printf("Input player attack : ");
		scanf("%d", &(p->atk));
		getc(stdin);
	}
	printf("Init Player complete.\n==============================\n");
}

void update_player (Character* p, char* info) {
	sscanf(info, "%s %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d %d %d",
			p->name,
			&(p->hp), &(p->max_hp),
			&(p->atk),
			&(p->stun),
			&(p->fever),
			&(p->target),
			p->cmd,
			&(p->wait),
			&(p->cool_time[0]),&(p->cool_time[1]),&(p->cool_time[2]),
			&(p->cool_time[3]),&(p->cool_time[4]),&(p->cool_time[5]),
			&(p->fever_time),
			&(p->buff_atk.level), &(p->buff_wait.level)
	);
}

void update_enemy (Character* e, char* info) {
	sscanf(info, "%s %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d %d %d",
			e->name,
			&(e->hp), &(e->max_hp),
			&(e->atk),
			&(e->stun),
			&(e->fever),
			&(e->target),
			e->cmd,
			&(e->wait),
			&(e->cool_time[0]),&(e->cool_time[1]),&(e->cool_time[2]),
			&(e->cool_time[3]),&(e->cool_time[4]),&(e->cool_time[5]),
			&(e->fever_time),
			&(e->buff_atk.level), &(e->buff_wait.level)
	);
}

void init_curses () {
	initscr();           //curses
	noecho();            //curses
	cbreak();            //curses
	keypad(stdscr,TRUE); //curses
	timeout(0);

	attron(A_BOLD);
}

int connect_server (SocketInfo* s) {
	char address[20];

	printf("Input server address : ");
	scanf("%s", address);
	getc(stdin);
	if (strcmp(address, "localhost")==0 || strcmp(address, "l")==0) strcpy(address, "127.0.0.1");
	printf("Connect to %s ...\n", address);

	/* クライアント用にソケットを作成する */
	s->sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* サーバ側と同じ名前でソケットの名前をつける */
	/* 127.0.0.1はループバックアドレス（ローカルマシンとの通信だけに制限） */
	s->address.sin_family = AF_INET;
	s->address.sin_addr.s_addr = inet_addr(address);
	s->address.sin_port = htons(9734);
	s->len = sizeof(s->address);

	/* クライアントのソケットをサーバのソケットに接続する */

	s->result = connect(s->sockfd, (struct sockaddr *)&s->address, s->len);

	if(s->result == -1) {
		perror("cannot connet server");
		return -1;
	}
}