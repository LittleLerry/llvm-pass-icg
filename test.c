#include <stdio.h>
int main ()
{
    float score;
    char grade;
    scanf("%f", &score);
    int a= (int)score;
    switch(a/10)
    {
        case 10:
        case 9: grade='A';break;
        case 8:grade='B';break;
        case 7:grade='C';break;
        case 6:grade='D';break;
        default :grade='E';
    }
    printf("%.2f——>%c",score,grade);
}