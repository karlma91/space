#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "highscorelist.h"

#define DEBUG fprintf(stderr,"Line: %d\n",__LINE__);

static FILE * file;

static scoreelement score_def = {"----",0,1352918808,NULL};
static int read_addscore(scorelist *list, char *name, int score,long time,int from_file);
/**
 * inits a list struct
 */
void highscorelist_create(scorelist *list)
{
	list->elements = 0;
	list->head = NULL;
}

/**
 * adds a score to the list
 * returns position, or -1 if error
 */
int highscorelist_addscore(scorelist *list, char *name, int score)
{
	long t = 0;
	time(&t);
	return read_addscore(list, name,score,t,0);
}

static int read_addscore(scorelist *list, char *name, int score,long time,int from_file)
{
	//fprintf(stderr, "%s %d %d %d\n",name,score,time,from_file);
	if(strlen(name)>4 || strlen(name)<3){
			//fprintf(stderr,"%s to long or short name",name);
			return -1;
		}
		if(score < 0){
			//fprintf(stderr,"negative score %d\n",score);
			return -1;
		}
		scoreelement **cur = &(list->head);

		scoreelement *element = malloc(sizeof(scoreelement));
		strcpy(element->name, name);
		element->score = score;
		element->time = time;
		element->next = NULL;
		int position = 1;
		while(*cur != NULL){
			position++;
			if((*cur)->score <= element->score - from_file){
				element->next = *cur;
				*cur = element;
				list->elements++;
				return position;
			}
			cur = &((*cur)->next);
		}

		(*cur) = element;
		list->elements++;
		return position;
}

/**
 * gets score at a position
 */
int highscorelist_getscore(scorelist *list, int position, scoreelement *score)
{
	int counter = 1;
	scoreelement *cur = list->head;
	while(counter < list->elements+1){
		if(counter == position){
			strcpy(score->name, cur->name);
			score->score = cur->score;
			score->time = cur->time;
			return 0;
		}
		cur = cur->next;
		counter++;
	}
	strcpy(score->name, score_def.name);
	score->score = score_def.score;
	score->time = score_def.time;
	return -1;
}

/**
 * frees all element in list but not the scorelist struct
 */
void highscorelist_destroy(scorelist *list)
{
	int counter = 1;
	scoreelement *temp;
	scoreelement *cur = list->head;
	while(cur != NULL){
		temp = cur;
		cur = cur->next;
		free(temp);
		counter++;
	}
	list->elements = 0;
	list->head = NULL;
}

/**
 * reads a from a file to the list
 */
int highscorelist_readfile(scorelist *list, char *filename)
{
	file = fopen(filename,"r");
	if (file == NULL) {
		fprintf(stderr, "Could not load %s\n",filename);
		return 1;
	}

	char name[5];
	int score = 0;
	int time = 0;
	int ret = 0;
	while(1==1){
		ret = fscanf(file,"%s %d %d\n",name,&score,&time);
		if (ret != EOF) {
			if (ret == 3) {
				read_addscore(list,name,score,time,1);
			} else {
				fprintf(stderr,"error reading highscore file\n");
				break;
			}
		} else {
			break;
		}
	}
	fclose(file);
	return 0;
}

/**
 * write list to file
 */
int highscorelist_writefile(scorelist *list, char *filename)
{
	file = fopen(filename,"w");
	if (file == NULL) {
		fprintf(stderr, "Could not load %s\n",filename);
		return 1;
	}

	scoreelement temp = {"    ",0,0,0};

	int i;
	for(i=0; i<list->elements; i++){
		highscorelist_getscore(list,i+1,&temp);
		fprintf(file,"%s %d %d\n",temp.name,temp.score,temp.time);
	}
	fclose(file);
	return 0;
}

