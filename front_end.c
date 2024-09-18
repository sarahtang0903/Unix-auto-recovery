#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "back_end.h"
#include <stdbool.h>

int Options()
{
    int option;
    printf("--------Options-------\n");
    printf("(1) insert data record\n");
    printf("(2) search data record\n");
    printf("(3) delete data record\n");
    printf("(4) list all data record\n");
    printf("(5) close the front-end\n");
    printf("-----------------------\n");
    printf("Please choose one option:\n");
    scanf("%d", &option);
    return option;
}

int main()
{
    char *to_back_end_fifo = "./to_back_end_fifo";
    char *from_back_end_fifo = "./from_back_end_fifo";
    mkfifo(to_back_end_fifo, 0666);
    mkfifo(from_back_end_fifo, 0666);

    int to_back_end_fd = open(to_back_end_fifo, O_WRONLY);
    int from_back_end_fd = open(from_back_end_fifo, O_RDONLY);
    if (to_back_end_fd == -1 || from_back_end_fd == -1)
    {
        perror("open FIFOs");
        return 1;
    }

    int num;
    do
    {
        num = Options();
        Command commandata;
        switch (num)
        {
        case 1:
        {
            commandata.command = 1;
            write(to_back_end_fd, &commandata, sizeof(commandata));
            struct DataRecord record;
            char add_name[20];
            int id;
            int deposit;
            int comment;
            printf("Please enter the name: \n");
            scanf("%s", add_name);
            printf("Please enter the ID: \n");
            scanf("%d", &id);
            printf("Please enter the deposit: \n");
            scanf("%d", &deposit);

            strncpy(record.name, add_name, sizeof(record.name) - 1);
            record.name[sizeof(record.name) - 1] = '\0';
            record.id = id;
            record.deposit = deposit;

            write(to_back_end_fd, &record, sizeof(record));

            struct DataRecord add_result;
            read(from_back_end_fd, &add_result, sizeof(add_result));
            if (add_result.id == -1)
            {
                printf("Inserted!!\n");
            }
            else
            {
                printf("Insert Error!!\n");
            }
        }
        break;

        case 2:
        {
            commandata.command = 2;
            write(to_back_end_fd, &commandata, sizeof(commandata));
            int search_id;
            printf("Please enter the ID to search:\n");
            scanf("%d", &search_id);
            write(to_back_end_fd, &search_id, sizeof(search_id));

            struct DataRecord search_result;
            read(from_back_end_fd, &search_result, sizeof(search_result));

            if (search_result.id != -1)
            {
                printf("-----------------------\n");
                printf("Name: %s\n", search_result.name);
                printf("ID: %d\n", search_result.id);
                printf("Deposit: %d\n", search_result.deposit);
                printf("-----------------------\n");
            }
            else
            {
                printf("-----------------------\n");
                printf("Not founded\n");
                printf("-----------------------\n");
            }
        }
        break;

        case 3:
        {
            commandata.command = 3;
            write(to_back_end_fd, &commandata, sizeof(commandata));

            int delete_id;
            printf("Please enter the ID to delete: \n");
            scanf("%d", &delete_id);
            write(to_back_end_fd, &delete_id, sizeof(delete_id));

            struct DataRecord delete_result;
            read(from_back_end_fd, &delete_result, sizeof(delete_result));

            if (delete_result.id == -1)
            {
                printf("List is empty!!\n");
            }
            else if (delete_result.id == -2)
            {
                printf("No user!!\n");
            }
            else
            {
                printf("Deleted!!\n");
            }
        }
        break;

        case 4:
        {
            commandata.command = 4;
            write(to_back_end_fd, &commandata, sizeof(commandata));

            struct DataRecord list_result;

            while (read(from_back_end_fd, &list_result, sizeof(list_result)) > 0)
            {
                if (list_result.id == -1)
                {
                    // 收到 "No user" 標誌
                    printf("No user\n");
                    break;
                }
                else if (list_result.id == -2)
                {
                    // 收到结束标志
                    printf("Listed all.\n");
                    break;
                }

                printf("-----------------------\n");
                printf("Name: %s\n", list_result.name);
                printf("ID: %d\n", list_result.id);
                printf("Deposit: %d\n", list_result.deposit);
                printf("-----------------------\n");
            }
        }
        break;

        case 5:
        {
            commandata.command = 5;
            write(to_back_end_fd, &commandata, sizeof(commandata));
            return 0;
        }
        break;

        default:
            break;
        }
    } while (num != 5);
    close(to_back_end_fd);
    close(from_back_end_fd);

    return 0;
}
