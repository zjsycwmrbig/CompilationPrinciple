%{
    //目前只支持测评单文件程序，所以需要将代码都写到这个文件中。

#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;
int line_num = 0; //记录行号
int key_num = 0; //记录关键字个数
int id_num = 0; //记录标识符个数
int operator_num = 0; //记录运算符个数
int num_num = 0; //记录整常量个数
int str_num = 0; //记录字符串常量个数
int char_num = 0; //记录字符常量个数
int error_num = 0; //记录错误个数

%}
 /* begin */
KEYWORD     auto|double|int|struct|break|else|static|long|switch|case|enum|register|typedef|char|extern|return|union|const|float|short|unsigned|continue|for|signed|void|default|goto|sizeof|volatile|do|if|while
IDENTIFIER       [a-zA-Z_][a-zA-Z0-9_]*
OPERATOR        +|-|*|/|%|++|--|==|!=|<|>|<=|>=|&&||||!|&||^|~|<<|>>|=|+=|-=|*=|/=|%=|&=|^=||=|.|->
DELIMITER      ;|,|{|}|(|)|[|]|#|:
CHARCON        '[^']'
STRCON         "[^"]*"
NUMBER        [0-9]+
ERROR          .
// 注释匹配
COMMENT        ////.*
COMMENT2       //*^(*/)**/

 /* end */

%%
 /* begin */
{KEYWORD}       {printf("<KEYWORD,%s>\n", yytext);key_num++;}
{IDENTIFIER}    {printf("<IDENTIFIER,%s>\n", yytext);id_num++;}

{OPERATOR}      {printf("<OPERATOR,%s>\n", yytext);operator_num++;}
{DELIMITER}     {printf("<DELIMITER,%s>\n", yytext);operator_num++;}
{CHARCON}       {printf("<CHARCON,%s>\n", yytext);char_num++;}
{STRCON}        {printf("<STRCON,%s>\n", yytext);str_num++;}
{NUMBER}        {printf("<NUMBER,%s>\n", yytext);num_num++;
{COMMENT}       {}
{COMMENT2}      {}


{ERROR}           {printf("ERROR: %s\n", yytext);error_num++;}

 /* end */

\n				{line_num++;}
.				{}
<<EOF>>			{printf("Total Lines: %d\n", line_num);
                printf("Total Keywords: %d\n", key_num);
                printf("Total Identifiers: %d\n", id_num);
                printf("Total Operators: %d\n", operator_num);
                printf("Total Numbers: %d\n", num_num);
                printf("Total Strings: %d\n", str_num);
                printf("Total Characters: %d\n", char_num);
                printf("Total Errors: %d\n", error_num);
    }
%%

int main(int argc, char* argv[])
{

    string file_name = argv[1]; //该字符串变量为识别的C语言程序文件名

    /*程序每次运行只需要考虑识别一个C语言程序;
      需要使用读取文件的操作,可以在自己的本地环境里创建样例文件进行测试；
      在自己的IDE里测试时可以将file_name赋其他的值，
      但注意，最终测评时由平台提供的main参数来传入文件名*/
    
    
    
	return 0;
}