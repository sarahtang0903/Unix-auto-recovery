#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include "back_end.h"

LinkedList *list = NULL;

LinkedList *
new_LinkedList()
{
    LinkedList *this;

    this = (LinkedList *)malloc(sizeof(LinkedList));

    this->head = NULL;
    this->tail = NULL;

    return this;
} // new_LinkedList ()

void delete_LinkedList(LinkedList *this)
{
    ListNode *p, *q;

    p = this->head;
    while (p != NULL)
    {
        q = p->suc;
        free(p);
        p = q;
    }

    free(this);
} // delete_LinkedList ()

int AppendKey_LinkedList(LinkedList *this, const char name[20], int id, int deposit)
{
    ListNode *new_node;

    new_node = (ListNode *)malloc(sizeof(ListNode));
    if (new_node == NULL)
    {
        // 内存分配失败
        return 0;
    }
    strncpy(new_node->name, name, sizeof(new_node->name) - 1);
    new_node->name[sizeof(new_node->name) - 1] = '\0';
    new_node->id = id;
    new_node->deposit = deposit;
    new_node->suc = NULL;

    if (this->head == NULL)
    {
        this->head = new_node;
        this->tail = new_node;
    }
    else
    {
        this->tail->suc = new_node;
        this->tail = new_node;
    }
    // Exit_Handler(this);
    return 1;
} // InsertKey_LinkedList ()

void DeleteKey_LinkedList(LinkedList *this, int id, int from_back_end_fd)
{
    if (this->head == NULL)
    {
        struct DataRecord list_empty = {.id = -1};
        write(from_back_end_fd, &list_empty, sizeof(struct DataRecord));
        return;
    }
    ListNode *p, *q;

    p = this->head;
    q = NULL;
    while ((p != NULL) && (p->id != id))
    {
        q = p;
        p = p->suc;
    }

    if (p == NULL)
    {
        struct DataRecord no_delete_user = {.id = -2};
        write(from_back_end_fd, &no_delete_user, sizeof(struct DataRecord));

        return;
    }

    if (q == NULL)
    { // match the first
        this->head = p->suc;
    }
    else
    {
        q->suc = p->suc;
    }

    if (p == this->tail)
        this->tail = q;

    free(p);
    struct DataRecord deleted_user = {.id = -3};
    write(from_back_end_fd, &deleted_user, sizeof(struct DataRecord));
    // Exit_Handler(list);

} // DeleteKey_LinkedList ()

void DumpKey_LinkedList(LinkedList *this, int to_back_end_fd)
{
    ListNode *p;

    p = this->head;
    if (p == NULL)
    {
        struct DataRecord no_user = {.id = -1};
        write(to_back_end_fd, &no_user, sizeof(struct DataRecord));
    }

    else
    {
        while (p != NULL)
        {
            write(to_back_end_fd, p, sizeof(struct DataRecord));
            p = p->suc;
        }
        struct DataRecord end_marker = {.id = -2};
        write(to_back_end_fd, &end_marker, sizeof(struct DataRecord));
    }
} // DumpKey_LinkedList ()

ListNode *Search(LinkedList *this, int id)
{
    ListNode *p = this->head;

    while (p != NULL)
    {
        if (p->id == id)
        {
            return p;
        }
        p = p->suc;
    }

    return NULL;
}

void sig10(int arg)
{
    Exit_Handler(list);
    exit(0);
}

void Exit_Handler(LinkedList *list)
{

    FILE *file;
    file = fopen("data.txt", "w");

    if (file == NULL)
    {
        perror("Error opening file for writing");
        return;
    }

    ListNode *p;
    p = list->head;
    while (p != NULL)
    {
        fprintf(file, " %s %d %d\n", p->name, p->id, p->deposit);
        p = p->suc;
    }
    fprintf(file, "end\n");
    fclose(file);
}

void readfile(LinkedList *this)
{
    delete_LinkedList(this);
    struct ListNode_s read_data;
    FILE *file;
    file = fopen("data.txt", "r");
    if (file != NULL)
    {
        while (fscanf(file, "%s", read_data.name) != EOF)
        {
            if (!strcmp(read_data.name, "end"))
                break;
            fscanf(file, "%d", &read_data.id);
            fscanf(file, "%d", &read_data.deposit);
            AppendKey_LinkedList(list, read_data.name, read_data.id, read_data.deposit);
        }
        fclose(file);
    }
}

LinkedList *list;
int main()
{
    signal(2, sig10);

    // 省略部分代碼
    list = new_LinkedList();
    readfile(list);

    char *to_back_end_fifo = "./to_back_end_fifo";
    char *from_back_end_fifo = "./from_back_end_fifo";
    mkfifo(to_back_end_fifo, 0666);
    mkfifo(from_back_end_fifo, 0666);

    int to_back_end_fd = open(to_back_end_fifo, O_RDONLY);
    int from_back_end_fd = open(from_back_end_fifo, O_WRONLY);
    if (to_back_end_fd == -1 || from_back_end_fd == -1)
    {
        perror("open FIFOs");
        return 1;
    }

    int command;
    int search_id;
    int delete_id;

    while (1)
    {
        Command commandata;
        read(to_back_end_fd, &commandata, sizeof(commandata));

        switch (commandata.command)
        {
        case 1:
        {

            struct DataRecord record;
            read(to_back_end_fd, &record, sizeof(record));
            int insertion_result = AppendKey_LinkedList(list, record.name, record.id, record.deposit);

            if (insertion_result)
            {
                struct DataRecord add_complete = {.id = -1};
                write(from_back_end_fd, &add_complete, sizeof(struct DataRecord));
            }
            else
            {
                struct DataRecord add_error = {.id = -2};
                write(from_back_end_fd, &add_error, sizeof(struct DataRecord));
            }
        }
        break;

        case 2:
        {

            read(to_back_end_fd, &search_id, sizeof(search_id));
            struct DataRecord search_result;
            ListNode *search_answer = Search(list, search_id);

            if (search_answer != NULL)
            {
                strncpy(search_result.name, search_answer->name, sizeof(search_result.name) - 1);
                search_result.name[sizeof(search_result.name) - 1] = '\0';
                search_result.id = search_answer->id;
                search_result.deposit = search_answer->deposit;
            }
            else
            {
                search_result.id = -1;
            }
            write(from_back_end_fd, &search_result, sizeof(search_result));
        }
        break;

        case 3:
        {

            int delete_id;
            read(to_back_end_fd, &delete_id, sizeof(delete_id));
            DeleteKey_LinkedList(list, delete_id, from_back_end_fd);
        }
        break;

        case 4:
        {

            DumpKey_LinkedList(list, from_back_end_fd);
        }
        break;

        case 5:
        {
            Exit_Handler(list);
        }
        break;

        default:
            break;
        }
    }
}
