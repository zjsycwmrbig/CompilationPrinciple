#include<iostream> 
using namespace std;

int main(int argc, char* argv[]){
    // 根据输入的参数读取文件，打印出文件内容
    if(argc < 2){
        cout << "Usage: " << argv[0] << " filename" << endl;
        return 1;
    }
    cout << "filename: " << argv[1] << endl;
    // 读取文件内容
    FILE* fp = fopen(argv[1], "r");
    if(fp == NULL){
        cout << "open file " << argv[1] << " failed" << endl;
        return 1;
    }
    char buf[1024];
    while(fgets(buf, 1024, fp) != NULL){
        cout << buf;
    }
    fclose(fp);
    return 0;

}

// while(true){
//         switch (state)
//         {
//         case 0:
//             if(c == '0') state = 1;
//             else if(c >= '1' && c <= '9') state = 2;
//             else if(c == '-') state = 3;
//             else {
//                 // 不回滚防止循环 这个可能性也没有
//                 return ERROR;
//             }
//             break;
//         case 1:
//             if(c == '0') state = 1;
//             else if(c >= '1' && c <= '7') state = 2;
//             else if(c == 'x' || c == 'X') state = 4;
//             else if(c == 'l' || c == 'L') state = 6;
//             else if(c == '.') state = 7;
//             else if(c == 'e' || c == 'E') state = 9;
//             else {
//                 // 可能正确，需要判断
//                 return end_number(number,c);  
//             }
//             break;
//         case 2:
//             if(c >= '0' && c <= '9') state = 2;
//             else if(c == 'l' || c == 'L') state = 6;
//             else if(c == '.') state = 7;
//             else if(c == 'e' || c == 'E') state = 9;
//             else {
//                 // 可能正确，需要判断
//                 return end_number(number,c);
//             }
//             break;
//         case 3:
//             if(c >= '0' && c <= '9') state = 2;
//             else {
//                 // 必定错误 但是我们没有假定这种情况发生
//                 return ERROR;
//             }
//             break;
//         case 4:
//             if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) state = 5;
//             else {
//                 // 16进制必定错误
//                 return end_number(number,c);
//             }
//             break;
//         case 5:
//             if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) state = 5;
//             else if(c == 'l' || c == 'L') state = 6;
//             else {
//                 // 可能正确，需要判断
//                 return end_number(number,c);
//             }
//             break;

//         case 6:
//             if(c == 'l' || c == 'L') state = 6;
//             else {
//                 return end_number(number,c);
//             }
//             break;
//         case 7:
//             if(c >= '0' && c <= '9') state = 8;
//             else {
//                 // 必定错误 处理方法是不到
//                 return ERROR;
//             }
//             break;
//         case 8:
//             if(c >= '0' && c <= '9') state = 8;
//             else if(c == 'e' || c == 'E') state = 9;
//             else if(c == 'l' || c == 'L') state = 6;
//             else {
//                 // 可能正确，需要判断
//                 return end_number(number,c);
//             }
//             break;
//         case 9:
//             if(c >= '0' && c <= '9') state = 11;
//             else if(c == '+' || c == '-') state = 10;
//             else {
//                 // 必定错误 处理方法是不到
//                 if(c != FILEEOF){
//                     number.pop_back();
//                 }
//                 rollback(c);
//                 print(MACHERROR,number.c_str());
//                 return ERROR;
//             }
//             break;
//         case 10:
//             if(c >= '0' && c <= '9') state = 11;
//             else {
//                 // 不回滚防止循环 
//                 return ERROR;
//             }
//             break;
//         case 11:
//             if(c >= '0' && c <= '9') state = 11;
//             else if(c == 'l' || c == 'L') state = 6;
//             else {
//                 // 防止回退过多，主要是删除换行符等
//                 return end_number(number,c);
//             }
//             break;
//         default:
//             // 防止回退过多，主要是删除换行符等
//             if(c != FILEEOF){
//                 number.pop_back();
//             }
//             rollback(c);
//             print(NUMBER,number.c_str());
//             return OK;
//             break;
//         }
//         c = getnext(number);
//     }