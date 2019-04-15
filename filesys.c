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

static int filesys_inited = 0;

struct node
{
	char** merkelTree;
	int fd;
	struct node *next;
} ;

struct list_of_trees
{
	struct node *head;
	int num;
}*Trees_in_memory;

void Print(char A[], int n){
	for (int i = 0; i < n; ++i)
	{
		printf("%c", A[i]);
	}
	printf("\n");
}



// Insert node into list of trees in memory
void insert_into(char** merkel,int filed)
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
}

// Get node corresponding to filed from linked list
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

// Remove node from main memory
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

// Inserts and builds up the Merkel tree if not already present
void makeTree(char **hashes,int l,int r,int root, char **merkelTree){
	int lchild = 2 * root + 1;
	int rchild = 2 * root + 2;
	if (l == r)
	{
		char *shall = (char*)malloc(20*sizeof(char));
		get_sha1_hash(hashes[l],64,shall);
		strcpy(merkelTree[root],shall);
	}
	else
	{
		int mid = (l+r)/2;
		makeTree(hashes,l,mid,lchild,merkelTree);
		makeTree(hashes,mid+1,r,rchild,merkelTree);
		// printf("Before concatenating \n");
		char temp[41];
		strcpy(temp, merkelTree[lchild]);
		strcat(temp, merkelTree[rchild]);
		char *shall = (char*)malloc(20*sizeof(char));
		get_sha1_hash(temp,40,shall);
		strcpy(merkelTree[root],shall);
		// printf("%s\n", merkelTree[root]);
	}
}

//Create merkel tree of file denoted by fd by calling makeTree.
char ** createMerkelTree(int fd)
{
	int count = 0;

	// 1. Loop from 0 to end of file

	while(1)
	{
		char buf[64];
		int x = read(fd,buf,64);
		
		if (x <= 0)
		{
			break;
		}
		count++;
	}

	printf("Initial count %d\n", count);
	if (count>0)
	{
		// Initialising the array of hash
		lseek(fd, 0, SEEK_SET); 
		char **hashes = (char**)malloc(count*sizeof(char*));
		for (int i = 0; i < count; ++i)
		{
			hashes[i] = (char*)malloc(20*sizeof(char));
		}

		// 2. Keep storing values in multiples of 64 in an array 
		// 3. Pass it through the sha1 to get hash
		
		int i=0;
		char buf[count][64];
		while(i<count)
		{
			read(fd,buf[i],64);
			i++;
		}

		// Initialising the merkel tree
		char **merkelTree = (char**)malloc(count*4*sizeof(char*));
		for (int i = 0; i < 4*count; ++i)
		{
			merkelTree[i] = (char*)malloc(20*sizeof(char));
			// memset(merkelTree[i],'\0',20);
		}
		printf("Before aloocation mt%s\n", *merkelTree);
		// 4. Insert this hash into the Merkel Tree
		makeTree(hashes,0,count-1,0, merkelTree);
		printf("After aloc mt%s\n", *merkelTree);
		return merkelTree;
	}
	// printf("Done\n");
	return NULL;
}

//Get hash of a file from secure.txt
void get_entry(const char *filename, char* hash1)
{
	FILE *fp = fopen ("secure.txt", "a+");

	fseek(fp, 0, SEEK_END); // goto end of file
	if(ftell(fp) == 0){
		printf("Empty secure.txt\n");
		fclose(fp);
		// hash1 = NULL;
		hash1[0] = '\0';
		return;
	}
	fseek(fp, 0, SEEK_SET);// goto beginning of file


	while(1)
	{	

		


		char hash[23], file[11];
		
		char q;
		int i=0;

		printf("Bofore file loop\n");
		while((q = (char)fgetc(fp)) != ':'&& (int)q!= EOF)
		{
			file[i] = q;
			i++;
			printf("i = %d\n", i);
		}
		i =0 ;
		printf("%s\n","file" );
		while((q = (char)fgetc(fp)) != ';'&& (int)q!= EOF)
		{
			hash[i] = q;
			i++;
		}
		
		printf(" File%sHasH\n",file);Print(hash,23);
		printf("FILE%sFILENAME%s\n", file, filename);
		if (strcmp(filename, file) == 0)
		{
			fclose(fp);
			printf("%s\n", "Inside if cond of filename");
			hash1 = hash; 
			return;
		}

		if (feof(fp)) 
		    break ;
	}




	fclose(fp);
	// hash1 = NULL;
	hash1[0] = '\0';
	printf("%s\n", "Not Found");
}

// Add hash of file to secure.txt
void add_entry(const char *file, char *hash)
{
	if (hash == NULL)
	{
		printf("%s\n", "Returning");
		return;
	}
	printf("%s\n","Not NULL" );
	FILE *fp = fopen ("secure.txt", "a+");
	fputs(file,fp);
	fputs(":",fp);
	printf("HASH==%s\n", hash);
	fputs(hash,fp);
	fputs(";",fp);
	fclose(fp);
}


/* returns 20 bytes unique hash of the buffer (buf) of length (len)
 * in input array sha1.
 */
void get_sha1_hash (const void *buf, int len, const void *sha1)
{
	SHA1 ((unsigned char*)buf, len, (unsigned char*)sha1);
}

/* Build an in-memory Merkle tree for the file.
 * Compare the integrity of file with respect to
 * root hash stored in secure.txt. If the file
 * doesn't exist, create an entry in secure.txt.
 * If an existing file is going to be truncated
 * update the hash in secure.txt.
 * returns -1 on failing the integrity check.
 */
int s_open (const char *pathname, int flags, mode_t mode)
{
	assert (filesys_inited);

	// Make a merkel tree :-
	// 1. Loop from 0 to end of file
	// 2. Keep storing values in multiples of 64 in an array 
	// 3. Pass it through the sha1 to get hash
	// 4. Insert this hash into the Merkel Tree
	// 5. Then get both the roots and compare
	// 6. Decide course of action after comparision
	// 7. Check for truncate and take appropriate rebalancing action
	// 8. Update the Merkel tree and secure.txt file
	//10. Free all mallocs

	printf("%s\n","Inside open" );
	int fd = open (pathname, flags, mode);
	char **tree = createMerkelTree(fd);
	char hash[20];
	printf("%s\n","Get entry" );
	get_entry(pathname,hash);

	if (strlen(hash)>1)
	{
		printf("%s\n","---------------Get entry------" );
	}

	if (tree != NULL)
	{
		if (strlen(hash)>1)
		{
			
			if (strcmp(*tree,hash) != 0)
			{
				// File integrity compromised.
				printf("compromised %s \n", hash );
				return -1;
			}
			else
			{
				if (flags & O_TRUNC)
				{
					// Change value of hash in secure.txt
				}
			}
		}
		// printf("%s\n","Before insert" );
		insert_into(tree,fd);
		// printf("%s\n","After insert" );
	}

	if (strlen(hash)<=1 && tree !=NULL)
	{
		printf("%s\n", "No hash but tree found");
		add_entry(pathname,*tree);
	}
	if (tree == NULL)
	{
		printf("%s\n"," Tree NULL" );
	}

	lseek(fd, 0, SEEK_SET);
	printf("%s\n", "s_opened");
	return fd;
}

/* SEEK_END should always return the file size 
 * updated through the secure file system APIs.
 */
int s_lseek (int fd, long offset, int whence)
{
	assert (filesys_inited);

	
	
	return lseek (fd, offset, SEEK_SET);
}

/* read the blocks that needs to be updated
 * check the integrity of the blocks
 * modify the blocks
 * update the in-memory Merkle tree and root in secure.txt
 * returns -1 on failing the integrity check.
 */

ssize_t s_write (int fd, const void *buf, size_t count)
{
	assert (filesys_inited);

	// 1. read the blocks that needs to be updated
	// 2. check the integrity of the blocks
	// 			Roam inside merkel tree and compare hashes
	// 3. modify the blocks
	//			Beech me karo write and store return valoo
	// 4. update the in-memory Merkle tree and root in secure.txt
	// 5. set appropriate return values

	return write (fd, buf, count);
}

/* check the integrity of blocks containing the 
 * requested data.
 * returns -1 on failing the integrity check.
 */
ssize_t s_read (int fd, void *buf, size_t count)
{
	assert (filesys_inited);

	// Loop through these blocks and check if there hash
	// are same as the hashes in the merkel tree
	// return -1 on failure

	return read (fd, buf, count);
}

/* destroy the in-memory Merkle tree */
int s_close (int fd)
{
	assert (filesys_inited);

	// Destroy the merkel tree somehow
	// delete_node(fd);

	return close (fd);
}

/* Check the integrity of all files in secure.txt
 * remove the non-existent files from secure.txt
 * returns 1, if an existing file is tampered
 * return 0 on successful initialization
 */
int filesys_init (void)
{	

	// Check the Integrity of all the files 
	// by opening using s_open repeatedly in a loop 
	// till the total entries in secure.txt are finished

	// Simply update the variable and return from within the loop
	// else execute the below lines

	printf("%s\n","INITED started" );
	Trees_in_memory = (struct list_of_trees*)malloc(sizeof(struct list_of_trees));
	// FILE *fp = fopen ("secure.txt", "r");
	// while(1)
	// {
	// 	char hash[20], filename[20];
	// 	fgets (hash,20,fp);
	// 	fgets (filename,20,fp);
	// 	if (strlen(hash)<=1  || strlen(filename)<=1 )
	// 	{
	// 		break;
	// 	}
	// 	int fd1 = open((char*)filename,0);
	// 	char **tree = createMerkelTree(fd1);
	// 	if (strcmp(tree[0], hash) != 0)
	// 	{
	// 		close(fd1);
	// 		fclose(fp);
	// 		return 1;
	// 	}
	// 	close(fd1);
	// }
	// // Initialisation successfull
	// fclose(fp);

	printf("%s\n","INITED" );
	filesys_inited = 1;
	return 0;
}
