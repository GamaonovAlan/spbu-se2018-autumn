#include <stdio.h>
#include <limits.h>
#include "MD5.h"

struct list_t   //Custom list
{
    char* key;
    int count;  //Count of repetitions
    struct list_t *next;
};
typedef struct list_t List;

struct hash_t   //Custom hash table
{
    List **data;
    int *listLength;
    int size;
};
typedef struct hash_t HashTable;

void ExitWithMemoryError(size_t size)
{
    fprintf(stderr, "Unable to allocate enough memory! (%d bytes)", (int)size);
    exit(4);
}

//New hash table initialization
HashTable Init(unsigned int size)
{
    HashTable table;
    table.data = (List**) calloc(size, sizeof(List*));
    if (!table.data) ExitWithMemoryError(size * sizeof(List*));

    table.size = size;

    table.listLength = (int *) calloc(size, sizeof(int *));
    if (!table.data) ExitWithMemoryError(size * sizeof(int *));

    return table;
}

//Get hash
uint32_t Hash(char *key)
{
    uint32_t hash;

    uint32_t *buffer = (uint32_t*) calloc(4, sizeof(uint32_t*));
    if (!buffer) ExitWithMemoryError(4 * sizeof(uint32_t*));

    md5((uint8_t *)key, strlen(key), (uint8_t *)buffer);
    hash = buffer[0];
    free(buffer);

    return hash;
}

//Find word in hash table node
List* Find(char* word, uint32_t i, HashTable *table)
{
    List *elem;
    elem = table->data[i];

    while (elem)
    {
        if (!strcmp(elem->key, word)) return elem;
        elem = elem->next;
    }
    return NULL;    //If we didn't find
}

void Add(char* word, HashTable *table)
{
    uint32_t i = Hash(word) % table->size;  //Index

    //If such word is int the table
    //we will increase count of repetitions
    List *elem = Find(word, i, table);
    if (elem)
    {
        elem->count++;
        return;
    }

    //If there isn't such word in table
    //we will add new node for him
    if (!table->data[i])
    {
        table->data[i] = (List*) malloc(sizeof(List));
        if (!table->data[i]) ExitWithMemoryError(sizeof(List));

        elem = table->data[i];
        table->listLength[i] = 1;
    }
    //If collision
    else
    {
        List *prev = table->data[i];
        while (prev->next) prev = prev->next;   //Go to end of list

        elem = (List *) malloc(sizeof(List));
        if (!elem) ExitWithMemoryError(sizeof(List));

        prev->next = elem;  //Add new element to the end of list
        table->listLength[i]++;
    }

    //Initialize element in hash table
    elem->key = (char *) calloc(strlen(word) + 1, sizeof(char));
    if (!elem->key) ExitWithMemoryError(strlen(word) + 1 * sizeof(char));
    strcpy(elem->key, word);
    elem->next = NULL;
    elem->count = 1;    //Set number of repetitions for element
}

int GetRepetitionsNumber(char* word, HashTable* table)
{
    uint32_t i = Hash(word) % table->size;  //Index
    List *elem = Find(word, i, table);

    //If the hash table doesn't contain such word
    if (!elem) return 0;

    //If the hash table contains such word
    return elem->count;
}

void GetStatistics(HashTable* table)
{
    int repetitionsMaxNum = 0;

    List *node;
    //Find max and print all words
    for (int i = 0; i < table->size; i++)
    {

        node = table->data[i];
        while (node)
        {
            if (node->count > repetitionsMaxNum) repetitionsMaxNum = node->count;
            //Print all words and repeat count
            fprintf(stdout, "%s %d\n", node->key, node->count);

            node = node->next;
        }
    }

    //Print all max
    for (int i = 0; i < table->size; i++)
    {
        node = table->data[i];
        while (node)
        {
            if (node->count == repetitionsMaxNum)
                fprintf(stderr, "%s %d\n", node->key, node->count);

            node = node->next;
        }
    }
}

//Deleting hash table
void Delete(HashTable* table)
{
    for (int i = 0; i < table->size; i++)
    {
        List *node = table->data[i];

        while (node)
        {
            List *temp = node;
            node = node->next;

            free(temp->key);
            free(temp);
        }
    }

    free(table->listLength);
    free(table->data);
}

const int MAX_WORD_SIZE = 2 * CHAR_MAX;
const int MAX_TABLE_SIZE = INT16_MAX;

int main()
{
    HashTable table = Init(MAX_TABLE_SIZE);

    char* buffer = (char *) calloc(MAX_WORD_SIZE, sizeof(char));
    if (!buffer) ExitWithMemoryError(MAX_WORD_SIZE * sizeof(char));

    char c;
    char* word;

    int pos = 0;
    while ((c = (char)getchar()) != EOF)
    {
        if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
        {
            buffer[pos] = c;
            pos++;
        }
        else
        {
            buffer[pos] = '\0';

            word = (char *) calloc(strlen(buffer) + 1, sizeof(char));
            if (!word) ExitWithMemoryError((strlen(buffer) + 1) * sizeof(char));

            strcpy(word, buffer);
            Add(word, &table);
            free(word);
            pos = 0;
        }
    }

    GetStatistics(&table);
    Delete(&table);

    return 0;
}