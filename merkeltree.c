#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include "filesys.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

struct node
{
	char merkelTree;
	int fd;
	struct node *next;
} ;

struct list_of_trees
{
	struct node *head;
	int num;
}*Trees_in_memory;


void insert_into(char merkel,int filed)
{
	struct node *cur_node = (struct node *)malloc(sizeof(struct node));
	cur_node -> merkelTree = merkel;
	cur_node -> fd = filed;
	cur_node -> next = NULL;
	if (Trees_in_memory->head == NULL)
	{
		Trees_in_memory->head = cur_node;
	}
	else
	{
		cur_node -> next = Trees_in_memory -> head;
		Trees_in_memory->head = cur_node;
	}
	Trees_in_memory->num++;
	// traverses();
}

struct node* get_node(int filed)
{
	struct node *iter = Trees_in_memory ->head;
	while(iter!=NULL)
	{
		// printf("%d\n",iter->fd );
		if (iter->fd == filed)
		{
			return iter;
		}
		iter = iter ->next;
	}
	return NULL;
}

int delete_node(int filed)
{
	struct node *prev = NULL;
	struct node *iter = Trees_in_memory ->head;
	while(iter!=NULL)
	{
		if (iter->fd == filed)
		{
			if (prev != NULL)
			{
				prev -> next = iter->next;
			}
			else
			{
				Trees_in_memory -> head = Trees_in_memory -> head ->next;
			}
			// printf("%c\n",iter->merkelTree );
			free(iter);
			Trees_in_memory->num -=1;
			return 0;
		}
		prev = iter;
		iter = iter ->next;
	}
	return 1;
}

void get_sha1_hash (const void *buf, int len, const void *sha1)
{
	SHA1 ((unsigned char*)buf, len, (unsigned char*)sha1);
}

// Inserts and builds up the Merkel tree if not already present
// void makeTree(char **hashes,int n,int l,int r,int root){
// 	int lchild = 2 * root + 1;
// 	int rchild = 2 * root + 2;
// 	if (l == r)
// 	{
// 		char *shall = (char*)malloc(20*sizeof(char));
// 		get_sha1_hash(hashes[l],3,shall);
// 		strcpy(merkelTree[root],shall);
// 	}
// 	else
// 	{
// 		int mid = (l+r)/2;
// 		makeTree(hashes,n,l,mid,lchild);
// 		makeTree(hashes,n,mid+1,r,rchild);
// 		// printf("Before concatenating \n");
// 		char temp[41];
// 		strcpy(temp, merkelTree[lchild]);
// 		strcat(temp, merkelTree[rchild]);
// 		char *shall = (char*)malloc(20*sizeof(char));
// 		get_sha1_hash(temp,40,shall);
// 		strcpy(merkelTree[root],shall);
// 		printf("%s\n", merkelTree[root]);
// 	}
// }

int main(int argc, char const *argv[])
{	
	Trees_in_memory = (struct list_of_trees*)malloc(sizeof(struct list_of_trees));
	printf("%s\n","Inserts" );
	for (int i = 0; i < 10; ++i)
	{
		insert_into((char)(i+97),i);
	}
	printf("%s\n","Traverses" );
	for (int i = 0; i < 10; ++i)
	{
		struct node *c = get_node(i);
		if (c != NULL)
		{
			printf("%c\n",c ->merkelTree);
		}
		
	}
	printf("%s\n", "Deletes");
	for (int i = 0; i < 10; ++i)
	{
		delete_node(i);
	}
	printf("%s\n","Done" );
	return 0;
}