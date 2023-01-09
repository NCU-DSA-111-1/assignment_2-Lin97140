
typedef struct player
{
    char name;
    int position[20][2]; // [][x, y]位置
} player;

void initialize(player *);
int find_thesame(player *, int, int);
int find_enemy(player *, int, int);
void show(const player *);
void change(player **, player **);
