#include <stdio.h>
int main()
{
    int num;
    char name[30];
    printf("학번 : ");
    scanf("%d", &num);
    printf("\n");

    printf("이름 : ");
    scanf("%s", name);
    printf("\n");

    printf("%d %s \n",num, name);
    return 0;

}