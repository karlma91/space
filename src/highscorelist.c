#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "highscorelist.h"

static FILE * file;

static int load_file = 0;

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
	element->next = NULL;
	int position = 1;
	while(*cur != NULL){
		position++;
		if((*cur)->score <= element->score - load_file){
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
int highscorelist_getscore(scorelist *list, int position, char *name, int *score)
{
	int counter = 1;
	scoreelement *cur = list->head;
	while(counter < list->elements+1){
		if(counter == position){
			strcpy(name,cur->name);
			*score = cur->score;
			return 0;
		}
		cur = cur->next;
		counter++;
	}
	//fprintf(stderr,"position not inside list %d",position);
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

	load_file = 1;
	char name[5];
	int score = 0;
	int ret = 0;
	while(1==1){
		ret = fscanf(file,"%s %d",name,&score);
		if(ret != EOF && ret == 2){
			highscorelist_addscore(list,name,score);
		}else{
			break;
		}
	}
	fclose(file);
	load_file = 0;
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
	char name[5];
	int score = 0;

	int i;
	for(i=0; i<list->elements; i++){
		highscorelist_getscore(list,i+1,name,&score);
		fprintf(file,"%s %d\n",name,score);
	}
	fclose(file);
	return 0;
}

