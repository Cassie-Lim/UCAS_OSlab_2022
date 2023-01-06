#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#define COW_NUM 9
#define MAX_MODIFY_NUM 4
uint64_t va_orig;
uint64_t va[COW_NUM];
typedef struct{
    int cur;
    int data;
}Write_Hist;
Write_Hist hist[COW_NUM][MAX_MODIFY_NUM];
int hcur[COW_NUM];

int main(){
    int i, j, k;
    int* arr;
    int arr_len = 4096/sizeof(int);
    int wri_times, data, cur;
    uint64_t pa;
    srand(clock());
    va_orig = 0x6000000lu;
    arr = (int*)va_orig;
    // 数据初始化
    for(int i=0; i<arr_len; i+=128)
        arr[i]=i;
    // 历史记录指针初始化
    for(int i=0; i<COW_NUM; i++)
        hcur[i]=0;
    printf("----------------Format:[_] pa1-pa2-------------------\n");
    for(i=0; i<COW_NUM; i++){
        va[i] = sys_copy_on_write(va_orig);
        // 决定本轮是否修改页面
        if(rand()%3){
            wri_times = rand()%MAX_MODIFY_NUM+1;
            for(j=0; j<wri_times; j++){
                data = rand()+1;
                cur = rand()%arr_len;
                arr[cur] = data;
                for(k=0; k<hcur[i]; k++)
                    if(hist[i][k].cur==cur)
                        break;
                // 该位置从未被修改过
                if(k==hcur[i]){
                    hist[i][hcur[i]].cur = cur;
                    hist[i][hcur[i]].data = data;
                    hcur[i]++;
                }
                else{
                    hist[i][k].data = data;
                }
            }
            printf("[M]");  // 表modified
        }
        else
            printf("[N]");  // 表not modifyed
        printf("0x%lx-0x%lx\t", sys_uva2pa(va_orig), sys_uva2pa(va[i]));
        if((i+1)%3==0)
            printf("\n");
    }
    printf("----------------Format: cur-data-------------------\n");
    for(i=0; i<COW_NUM; i++){
        int* arr1 = (int*)va[i];
        int* arr2 = (int*)i==COW_NUM-1? va_orig: va[i+1];
        printf("Diff: ");
        for(int j=0; j<arr_len; j++)
            if(arr1[j]!=arr2[j])
                printf("%d-%d\t", j, arr2[j]);
        printf("\n");
    }
}