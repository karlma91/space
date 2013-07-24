#ifndef HIGHSCORELIST_H_
#define HIGHSCORELIST_H_


typedef struct score scoreelement;
struct score{
	char name[5];
	int score;
	int time_used;
	int time;
	scoreelement *next;
};

typedef struct highscorelist{
	int elements;
	char filename[100];
	scoreelement *head;
}scorelist;

void highscorelist_create(scorelist *list);
int highscorelist_addscore(scorelist *list, char *name, int score, int time_used);
int highscorelist_getscore(scorelist *list, int position, scoreelement *score);
int highscorelist_readfile(scorelist *list, const char *file);
int highscorelist_writefile(scorelist *list);
void highscorelist_destroy(scorelist *list);

#endif /* HIGHSCORELIST_H_ */
