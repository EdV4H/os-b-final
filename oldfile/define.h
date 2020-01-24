#ifndef _DEFINE_h_
#define  _DEFINE_h_

typedef struct {
    int hp, max_hp, atk, def, dex;
    double atk_up, def_up, dex_up;
    int stun, dead;
} Status;

// typedef struct {
//     char name[10];
//     int wait, cool_time;
//     int dmg=0, atk_up=0, def_up=0, dex_up=0;
//     int stun=0, barrier=0;
// } Skill;

typedef struct {
	char name[10];
	Status status;
    char cmd[10], next_cmd[10];
    int a_ct, s_ct, d_ct, z_ct, x_ct, c_ct;
    int wait, target;
} Player;

typedef struct {
    char content[5][50];
    int index;
} Information;

#endif