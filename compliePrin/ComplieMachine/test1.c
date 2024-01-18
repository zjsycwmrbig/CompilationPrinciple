#include<stdio.h>
#include<string.h>
#define STA 0
#define DEF 1
#define NAM 2
#define EQU 3
#define EMP 4
#define NUM 5
#define FEN 6 
#define END 7
char str[100];
int mode = 0;
int cnt = -1;
char def[10][50] ={"int","double","float","short"}; 
int check(){
    int cur = cnt;
    for(int i = 0;i < 4;i++){
        int now = 0;int len = strlen(def[i]);
        while(str[cur++] == def[i][now++]);
        if(now >= len){mode = i;cnt = cur - 1;return 1;}
    }
    return 0;
}
int check_name(){
    int cur = cnt;
    while(str[cur]>='a'&&str[cur]<='z'||str[cur]>='A'&&str[cur]<= 'Z'||str[cur] == '_'||str[cur]<='9'&&str[cur]>='0'||str[cur] == '$')
    {
        cur++;
    }
    if(str[cur] == ','||str[cur] == '='||str[cur] == ' '){
        cnt = cur - 1;return 1;
    }
    else return 0;
}
int check_num(int mode){
    return 1;
}
void pre(){
    int book = 0;
    char STR[100];
    int cur = 0;
    for(int i = 0;i < 100&&str[i] != 0;i++){
        if(str[i] == ' '){if(!book){STR[cur++] = str[i];book = 1;}continue;}
        STR[cur++] = str[i];
    }
    for(int i = 0;i < cur;i++){
        str[i] = STR[i];
    }
    if(str[cur-1] == '\n')str[cur-1] = 0;
    else str[cur] = 0;
    return;
}
int main(){
    fgets(str, 100, stdin);
    int state = STA;
    pre();
    int flag = 0;
    while(state != END){
        cnt++;
        switch(state){
            case STA:{
                if(check()){
                    state = DEF;
                }
                else{
                    state = END;
                }
                break;
            }
            case DEF:
            {
                if(str[cnt] != ' ')state = END;
                else state = NAM;
                break;
            }
            case NAM:
            {
                if(check_name()){
                    if(str[cnt+1] == ','){
                        cnt++;
                        state = NAM;
                    }
                    else state = EQU;
                }
                else state = END;
            }
            case EQU:
            {
                if(str[cnt] == '=')state = NUM;
                else if(str[cnt] == ';')state = FEN;
                else state = END;
            }
            case NUM:
            {
                if(check_num(mode)){
                    if(str[cnt+1] == ',')cnt++,state = NAM;
                    else if(str[cnt+1] == ';')cnt++,state = FEN;
                }
                else state = END;
            }
            case FEN:
            {
                if(str[cnt] != 0)state = END;
                else flag = 1;
            }
        }
    }
    if(flag)printf("yes\n");
    else printf("no\n");
    return 0;
}