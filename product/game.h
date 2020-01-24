#ifndef _GAME_h_
#define _GAME_h_

/*  必要な定数を定義する  */
#define SEC_OF_TURN 1
#define I_INFO_SIZE 1001
#define P_INFO_SIZE 101

#define BASE_MAGNIFICATION 1.0
#define STUN_MAGNIFICATION 0.3
#define EX_MAGNIFICATION 5.0

/*  バフ情報  */
typedef struct {
    double amount;
    int level, time;
} Buff;

/*  コマンド関連  */
#define ATK_WAIT 3
#define ATK_CT 10
#define STUN_WAIT 3
#define STUN_CT 15
#define DEF_WAIT 2
#define DEF_CT 10
#define ZONE_WAIT 6
#define ZONE_CT 20
#define EX_WAIT 10
#define EX_CT 30
#define CHARGE_WAIT 3
#define CHARGE_CT 10

#define FEVER_WAIT 1
#define STUN_TIME 3 

typedef enum {
    Attack,
    Stun,
    Defence,
    Zone,
    Extreme,
    Charge
} Command;

/*  プレイヤー情報  */
#define FEVER_TIME 60

typedef struct {
    char name[10], cmd[10];
    int id, hp, max_hp, atk;
    int target, wait, stun, fever, fever_time;
    int cool_time[6];
    Buff buff_atk, buff_wait;
} Character;

/*  必要な関数を定義する  */

#endif