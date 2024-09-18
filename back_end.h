#ifndef LINKED_LIST_H

#define LINKED_LIST_H

typedef struct ListNode_s
{
    char name[20];
    int id;
    int deposit;
    struct ListNode_s *suc;
} ListNode;

typedef struct LinkedList_s
{
    ListNode *head;
    ListNode *tail;
} LinkedList;

typedef struct DataRecord
{
    char name[20];
    int id;
    int deposit;
} DataRecord;

typedef struct Command
{
    int command;
    struct DataRecord data;
} Command;

// methods of the linked_list:
LinkedList *
new_LinkedList();
void delete_LinkedList(LinkedList *this);
int AppendKey_LinkedList(LinkedList *this, const char name[20], int id, int deposit);
void DeleteKey_LinkedList(LinkedList *this, int id, int from_back_end_fd);
void DumpKey_LinkedList(LinkedList *this, int to_back_end_fd);
ListNode *Search(LinkedList *this, int id);
void sig10(int arg);
void Exit_Handler(LinkedList *this);
#endif
