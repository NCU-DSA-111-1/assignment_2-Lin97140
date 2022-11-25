typedef struct Node
{
    char name;
    int chess;
    int original_x;
    int original_y;
    int position_x;
    int position_y;
    int die;
    struct Node *next;
    struct Node *previous;
} node;

int isempty(const node *);
void push(node **, char, int, int, int, int, int, int);
void pop(node **, int *, int *, int *, int *, int *, int *, char *);
void free_stack(node **);
void show_stack(node *);