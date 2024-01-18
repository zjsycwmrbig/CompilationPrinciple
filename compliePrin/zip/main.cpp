#include <stdio.h>
#include <iostream>
#include <string>
using namespace std;

#pragma region DEFINE

#define DEBUGLEVEL 0 // 0:不输出调试信息 1:输出调试信息
// 状态定义
#define OK 1
#define ERROR 0
#define OVER -1

// 可变常量定义
#define BUFFERSIZE 1024 // 缓存区大小

// 模式定义
#define FILEEOF -1 // 文件结束
#define ANOTATION 0 // 注释
#define KEYWORD 1 // 关键字
#define IDENTIFIER 2 // 标识符
#define NUMBER 3 // 常量
#define OPERATOR 4 // 运算符
#define DELIMITER 5 // 界符
#define STRING 6 // 字符串
#define CHARCON 7 // 字符
#define MACHERROR 8 // 自动机错误
#define DEFAULT 9 // 其他数据

// 混合模式定义
#define KEYWORD_IDENTIFIER 10 // 关键字标识符
#define NUMBER_OPERATOR 11 // 常量和运算符
#define ANOTATION_OPERATOR 12 // 注释和运算符
#define DEFINE 13 // 宏定义

#pragma endregion

#pragma region GLOBAL VARIABLE

char buffer[BUFFERSIZE];// 缓存区
int buffer_fp = 0;// 缓存区前置指针
int buffer_ap = 0;// 缓存区后置指针

FILE *fp;// 读取文件
FILE *fp_out;// 输出文件

int line = 1;// 行号
int is_change_line = 0;// 是否换行

int error = 0;// 错误数
int keyword_num = 0;// 关键字数
int identifier_num = 0;// 标识符数
int number_num = 0;// 常量数
int operator_num = 0;// 运算符数
int delimiter_num = 0;// 界符数
int string_num = 0;// 字符串数
int charcon_num = 0;// 字符数
int state_map[128]; // 状态转换表

#pragma endregion

#pragma region FUNCTION DECLARE
char getnext(string& str);
bool is_end();
int read_file(FILE *fp);
int annotation();
void add_line();
template<typename... Args>
void debug(const char* format, Args... args);
template<typename... Args>
void info(const char* format,Args... args);
template<typename... Args>
void print(int mode,string str);
void init_state_map();
int number();
void rollback(char c);
#pragma endregion

#pragma region TOOL

/**
 * @brief 读取缓存区中的一个字符，并且将其放入str中，移动缓存区指针
 * 
 * @return char 
 */
char getnext(string& str){
    if(buffer_fp == buffer_ap) {
        info("getc: buffer is empty"); //buffer is empty
        if(read_file(fp) == ERROR) return FILEEOF;
    }
    char c = buffer[buffer_fp];

    // 记录行数
    if(c == '\n') {add_line();is_change_line = 1;}else {is_change_line = 0;}
    if(c != FILEEOF) {
        buffer_fp = (buffer_fp + 1) % BUFFERSIZE;
        str += c;
    }
    return c;
}

/**
 * @brief 仅仅返回缓存区中下一个字符，不移动缓存区指针
 * 
 * @return char 
 */
char getnext(){
    if(buffer_fp == buffer_ap) {
        info("getc: buffer is empty"); //buffer is empty
        if(read_file(fp) == ERROR) return FILEEOF;
    }
    char c = buffer[buffer_fp];
    return c;
}
/**
 * @brief 返回第二个字符，经常用来判断选择类型
 * 
 * @return char 
 */
char getsecond(){
    if((buffer_fp + 1)%BUFFERSIZE == buffer_ap) {
        info("getc: buffer is empty"); //buffer is empty
        if(read_file(fp) == ERROR) return ERROR;
    }
    char c = buffer[(buffer_fp+1)%BUFFERSIZE];
    return c;
}
/**
 * @brief 
 * 
 * @return char 
 */
char getthird(){
    if((buffer_fp + 2)%BUFFERSIZE == buffer_ap) {
        info("getc: buffer is empty"); //buffer is empty
        if(read_file(fp) == ERROR) return ERROR;
    }
    char c = buffer[(buffer_fp+2)%BUFFERSIZE];
    return c;
}

/**
 * @brief 回滚一个字符，主要用于回滚分隔符等
 * 
 * @param c 
 */
void rollback(char c){
    // 排除空格、换行、制表符等，不进行回退
    if(state_map[c] != DELIMITER && state_map[c] != OPERATOR && state_map[c] != NUMBER_OPERATOR) return;
    buffer_fp = (buffer_fp + BUFFERSIZE - 1) % BUFFERSIZE;
    buffer[buffer_fp] = c;
    // 回退一个字符
    if(buffer_fp == buffer_ap) buffer_ap = (buffer_ap + BUFFERSIZE - 1) % BUFFERSIZE;
}

/**
 * @brief 完全回复一段字符串，会恢复回车等，相当于时间回溯
 * 
 * @param c 
 */
void turnback(string str){
    for(int i = str.length() - 1; i >= 0; i--){
        if(str[i] == FILEEOF) continue;
        buffer_fp = (buffer_fp + BUFFERSIZE - 1) % BUFFERSIZE;
        buffer[buffer_fp] = str[i];
        // 回退
        if(buffer_fp == buffer_ap) buffer_ap = (buffer_ap + BUFFERSIZE - 1) % BUFFERSIZE;
        // 回退行号
        if(str[i] == '\n') line--;
    }
}

/**
 * @brief 判断是否是否还有必要继续
 * 
 * @return true 
 * @return false 
 */
bool is_end(){
    if(buffer[buffer_ap] == FILEEOF && buffer_fp == buffer_ap)return true;
    else return false;
}

/**
 * @brief 是否是关键字
 * 
 */
bool is_keyword(string str){
    // auto|double|int|struct|break|else|static|long|switch|case|enum|register|typedef|char|extern|return|union|const|float|short|unsigned|continue|for|signed|void|default|goto|sizeof|volatile|do|if|while
    if(str == "auto" || str == "double" || str == "int" || str == "struct" || str == "break" || str == "else" || str == "static" || str == "long" || str == "switch" || str == "case" || str == "enum" || str == "register" || str == "typedef" || str == "char" || str == "extern" || str == "return" || str == "union" || str == "const" || str == "float" || str == "short" || str == "unsigned" || str == "continue" || str == "for" || str == "signed" || str == "void" || str == "default" || str == "goto" || str == "sizeof" || str == "volatile" || str == "do" || str == "if" || str == "while"){
        return true;
    }else{
        return false;
    }
}

/**
 * @brief 初始化状态转换表
 * 
 */
void init_state_map(){
    // 初始化状态转换表
    fill(state_map, state_map + 128, MACHERROR);
    state_map[0] = FILEEOF;
    state_map['\n'] = DEFAULT;
    state_map[' '] = DEFAULT;
    state_map['\t'] = DEFAULT;
    state_map['\r'] = DEFAULT;
    state_map['!'] = OPERATOR;
    
    state_map['"'] = STRING;
    // state_map['#'] = DEFINE;
    for(char c = '$';c <= '&';c++){
        state_map[c] = OPERATOR;
    }
    state_map['\''] = CHARCON;
    for(char c = '(';c <= ')';c++){
        state_map[c] = DELIMITER;
    }
    // + - 看做运算符
    state_map['*'] = OPERATOR;
    state_map['+'] = OPERATOR;
    state_map[','] = DELIMITER;
    state_map['-'] = OPERATOR;
    state_map['.'] = OPERATOR;
    state_map['/'] = ANOTATION_OPERATOR;
    for(char c = '0';c <= '9';c++){
        state_map[c] = NUMBER;
    }
    for(char c = ':';c <= ';';c++){
        state_map[c] = DELIMITER;
    }
    for(char c = '<';c <= '>';c++){
        state_map[c] = OPERATOR;
    }
    for(char c = 'A'; c <= 'Z'; c++){
        state_map[c] = IDENTIFIER;
    }
    state_map['['] = DELIMITER;
    state_map[']'] = DELIMITER;
    state_map['^'] = OPERATOR;
    state_map['_'] = IDENTIFIER;
    for(char c = 'a';c <= 'z';c++){
        state_map[c] = KEYWORD_IDENTIFIER;
    }
    state_map['{'] = DELIMITER;
    state_map['|'] = OPERATOR;
    state_map['}'] = DELIMITER;
    state_map['?'] = DELIMITER;
    state_map['~'] = OPERATOR;
    state_map['.'] = NUMBER_OPERATOR;
}


/**
 * @brief 增加错误数
 * 
 */
void add_error(){
    error++;
}

/**
 * @brief 控制增加行号
 * 
 */
void add_line(){
    line++;
}

/**
 * @brief 控制增加模式数目
 * 
 */
void add(int mode){
    switch (mode){
    case KEYWORD:
        keyword_num++;
        break;
    case IDENTIFIER:
        identifier_num++;
        break;
    case NUMBER:
        number_num++;
        break;
    case OPERATOR:
        operator_num++;
        break;
    case DELIMITER:
        delimiter_num++;
        break;
    case STRING:
        string_num++;
        break;
    case CHARCON:
        charcon_num++;
        break;
    default:
        break;
    }
}

/**
 * @brief 显示目前的buffer_fp和buffer_ap以及buffer中的内容
 * 
 */
void check_buffer(){
    info("buffer_fp: %d, buffer_ap: %d", buffer_fp, buffer_ap);
    printf("[BUFFER] ");
    for(int i = buffer_fp; i != buffer_ap; i = (i+1)%BUFFERSIZE){
        std::printf("%c", buffer[i]);
    }
    printf("\n");
    getchar();
}

/**
 * @brief 清除bad标识,识别数字错误时使用消除后面的坏字符
 * 
 */
void clear_bad(){
    string str = "";
    // 清除bad标识
    while (true){
        char c = getnext();
        // ; 空格等
        if(state_map[c] == DELIMITER || state_map[c] == DEFAULT || state_map[c] == MACHERROR) return;
        c = getnext(str);
    }
}

/**
 * @brief 判断是否结束，包含分隔，空格，错误等
 * 
 */
bool is_number_end_char(char c){
    // 分隔符，错误，默认分隔，文件结束，运算符
    // if(state_map[c] == DELIMITER || state_map[c] == DEFAULT || state_map[c] == MACHERROR || state_map[c] == FILEEOF || state_map[c] == OPERATOR) return true;
    if(state_map[c] != IDENTIFIER && state_map[c] != KEYWORD_IDENTIFIER) return true;
    else return false;
}

/**
 * @brief 判断当下还是不是字符常量
 * 
 * @param str 
 * @return true 
 * @return false 
 */
bool is_charcon(string str){
    if(str.length() <= 2 || str[str.length()-1] != '\n' && str[str.length()-1] != FILEEOF) return true;
    else return false;
}

/**
 * @brief 识别数字结束时的处理方法，成功返回OK，失败返回ERROR，失败打印错误信息
 * 
 * @param number 接管
 * @param c 接管
 * @return int 
 */
int end_number(string number,char c){
    // 判断是否结束
    if(is_number_end_char(c)){
        
        // 防止回退过多，主要是删除换行符等
        if(c != FILEEOF){
            number.pop_back();
        }
        rollback(c);
        print(NUMBER,number.c_str());
        return OK;
    }else{
        // 把错误的字符全部读取出来，加到number上                
        while(true){
            char c = getnext(number);
            // 读取到其他的字符结束 123_ 按照错误处理
            if(is_number_end_char(c)){
                // 防止回退过多，主要是删除换行符等
                if(c != FILEEOF){
                    number.pop_back();
                }
                rollback(c);
                print(MACHERROR,number.c_str());
                return ERROR;
            }
        }
    }
}
#pragma endregion

#pragma region DEBUG
template<typename... Args>
void debug(const char* format, Args... args){
    if(DEBUGLEVEL == 0) return;
    std::printf("[DEBUG] ");
    std::printf(format, args...);
    std::printf("\n");
}

template<typename... Args>
void info(const char* format,Args... args){
    if(DEBUGLEVEL == 0) return;
    std::printf("[INFO ] ");
    std::printf(format, args...);
    std::printf("\n");
}

template<typename... Args>
void print(int mode,string str){
    int out_line = is_change_line?line-1:line;
    if(DEBUGLEVEL != 0) {
        std::printf("[PRINT:%d] ",out_line);        // 输出行号
    }
    if(mode == ANOTATION){
        if(DEBUGLEVEL != 0) {
            cout << "ANOTATION: " << str << endl;
        }
    }else if(mode == KEYWORD){
        printf("%d <KEYWORD,%s>\n",out_line,str.c_str());
    }else if(mode == IDENTIFIER){
        printf("%d <IDENTIFIER,%s>\n",out_line,str.c_str());
    }else if(mode == NUMBER){
        printf("%d <NUMBER,%s>\n",out_line,str.c_str());
    }else if(mode == OPERATOR){
        printf("%d <OPERATOR,%s>\n",out_line,str.c_str());
    }else if(mode == DELIMITER){
        printf("%d <DELIMITER,%s>\n",out_line,str.c_str());
    }else if(mode == STRING){
        printf("%d <STRING,%s>\n",out_line,str.c_str());
    }else if(mode == CHARCON){
        printf("%d <CHARCON,%s>\n",out_line,str.c_str());
    }else if(mode == MACHERROR){
        printf("%d <ERROR,%s>\n",out_line,str.c_str());
    }else{
        if(DEBUGLEVEL) printf("%d <ERROR,%s>\n",out_line,str.c_str());
    }

    if(DEBUGLEVEL == 0) {
        return;
    }
    // 输出到文件
    fprintf(fp_out, "%d ", out_line);
    if(mode == ANOTATION){
        fprintf(fp_out, "ANOTATION: %s\n", str.c_str());
    }else if(mode == KEYWORD){
        fprintf(fp_out, "KEYWORD: ,%s\n",str.c_str());
    }else if(mode == IDENTIFIER){
        fprintf(fp_out, "IDENTIFIER: ,%s\n",str.c_str());
    }else if(mode == NUMBER){
        fprintf(fp_out, "NUMBER: ,%s\n",str.c_str());
    }else if(mode == OPERATOR){
        fprintf(fp_out, "OPERATOR: ,%s\n",str.c_str());
    }else if(mode == DELIMITER){
        fprintf(fp_out, "DELIMITER: ,%s\n",str.c_str());
    }else if(mode == STRING){
        fprintf(fp_out, "STRING: ,%s\n",str.c_str());
    }else if(mode == CHARCON){
        fprintf(fp_out, "CHARCON: ,%s\n",str.c_str());
    }else if(mode == MACHERROR){
        fprintf(fp_out, "MACHERROR: ,%s\n",str.c_str());
    }else{
        fprintf(fp_out, "ERROR: ,%s\n",str.c_str());
    }
}
#pragma endregion

#pragma region FUNCTION DEFINE

#pragma region READ_FILE
/**
 * @brief 
 * 
 * @param fp 文档的文件指针，填充到buffer中，保证buffer中的内容是有效的
 * @return int 读取失败，读取结束返回0，读取成功返回1
 */
int read_file(FILE *fp){
    if(fp == NULL) {
        debug("read_file: fp is NULL"); //fp is NULL
        return ERROR;
    }
    
    if(feof(fp)) {
        debug("read_file: fp is end"); //fp is end
        return OVER;
    }

    if(buffer_fp == (buffer_ap + 1) % BUFFERSIZE) {
        debug("read_file: buffer is full"); //buffer is empty
        return ERROR;
    }

    // 归档buffer_ap
    buffer_ap = (buffer_fp + BUFFERSIZE - 1) % BUFFERSIZE;

    // 按照fp读取满buffer
    int i = 0;
    for(i = buffer_fp; i != buffer_ap; i = (i+1)%BUFFERSIZE){
        char c = fgetc(fp);
        if(c == EOF) {
            info("read_file: read file end"); //read file end
            buffer[i] = FILEEOF;
            buffer_ap = i;
            return OVER;
        }
        buffer[i] = c;
    }
    return OK;
}
#pragma endregion

#pragma region STATE RUN
// 状态运行函数
/**
 * @brief 处理注释的自动机,注释是没有错误这一个说法的
 * 
 * @return int 
 */
int annotation(){
    string annotation = "";
    // 正则
    
    // 读取一个字符   
    char c = getnext(annotation);
    
    if(c == '/'){
        char c = getnext(annotation);
        if(c == '/'){
            // 单行注释
            while(c != '\n'){
                c = getnext(annotation);
                if(c == FILEEOF) break;
            }
            print(ANOTATION,annotation.c_str());
            return OK;
        }else if(c == '*'){
            // 多行注释
            while(true){
                c = getnext(annotation);
                if(c == '*'){
                    c = getnext(annotation);
                    if(c == '/'){
                        print(ANOTATION,annotation.c_str());
                        return OK;
                    }
                }
                if(c == FILEEOF) {
                    print(MACHERROR,annotation.c_str());
                    return OK;
                };
            }
        }
    }
    return ERROR;
}

/**
 * @brief 处理宏定义的自动机，遇到回车结束
 * 
 * @return int 
 */
int define(){
    string define = "";
    char c = getnext(define);
    while(c != '\n'){
        c = getnext(define);
    }
    print(DEFINE,define.c_str());
    return OK;
}

/**
 * @brief 读取number的自动机
 * 
 * @return char 返回分隔结束符号 
 */
int number(){
    // 读取数字的自动机
    string number = "";
    char c = getnext(number);
    int state = 0; // 初始状态
    const int BEGIN = 0;

    // 识别八进制,十六进制,长整型,无符号,无符号长整型，浮点类型的自动机
    // 0 1 2 3 4 5 6 7 8 9 . e E + - l L u U f F
    while(true){
        switch (state)
        {
        case 0:
            if(c == '0') state = 1;
            else if(c >= '1' && c <= '9') state = 2;
            else if(c == '-') state = 3;
            else if(c == '.') state = 7; // 处理浮点数
            else {
                // 不回滚防止循环 这个可能性也没有
                return ERROR;
            }
            break;
        case 1:
            if(c == '0') state = 1;
            else if(c >= '1' && c <= '7') state = 2;
            else if(c == 'x' || c == 'X') state = 4;
            else if(c == 'l' || c == 'L') state = 6;
            else if(c == 'u' || c == 'U') state = 12; // 处理无符号
            else if(c == '.') state = 7;
            else if(c == 'e' || c == 'E') state = 9;
            else {
                // 可能正确，需要判断
                return end_number(number,c);  
            }
            break;
        case 2:
            if(c >= '0' && c <= '9') state = 2;
            else if(c == 'l' || c == 'L') state = 6;
            else if(c == 'u' || c == 'U') state = 12; // 处理无符号
            else if(c == '.') state = 7;
            else if(c == 'e' || c == 'E') state = 9;
            else {
                // 可能正确，需要判断
                return end_number(number,c);
            }
            break;
        case 3:
            if(c >= '0' && c <= '9') state = 2;
            else {
                // 必定错误 但是我们没有假定这种情况发生
                return ERROR;
            }
            break;
        case 4:
            if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) state = 5;
            else {
                // 16进制必定错误
                return end_number(number,c);
            }
            break;
        case 5:
            if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) state = 5;
            else if(c == 'l' || c == 'L') state = 6;
            else {
                // 可能正确，需要判断
                return end_number(number,c);
            }
            break;

        case 6:
            if(c == 'l' || c == 'L') state = 6;
            else if(c == 'u' || c == 'U') state = 12; // 处理无符号
            else {
                return end_number(number,c);
            }
            break;
        case 7:
            if(c >= '0' && c <= '9') state = 8;
            else if(c == 'l' || c == 'L') state = 6;
            else if(c == 'f' || c == 'F') state = 13;
            else if(c == 'e' || c == 'E') state = 9;
            else {
                // 必定错误 处理方法是不到
                return ERROR;
            }
            break;
        case 8:
            if(c >= '0' && c <= '9') state = 8;
            else if(c == 'e' || c == 'E') state = 9;
            else if(c == 'l' || c == 'L') state = 6;
            else if(c == 'f' || c == 'F') state = 13;
            else {
                // 可能正确，需要判断
                return end_number(number,c);
            }
            break;
        case 9:
            if(c >= '0' && c <= '9') state = 11;
            else if(c == '+' || c == '-') state = 10;
            else if(c == 'f' || c == 'F') state = 13;
            else if(c == 'l' || c == 'L') state = 6;
            else {
                // 必定错误 处理方法是不到WWW
                if(c != FILEEOF){
                    number.pop_back();
                }
                rollback(c);
                print(MACHERROR,number.c_str());
                return ERROR;
            }
            break;
        case 10:
            if(c >= '0' && c <= '9') state = 11;
            else {
                // 不回滚防止循环 
                return ERROR;
            }
            break;
        case 11:
            if(c >= '0' && c <= '9') state = 11;
            else if(c == 'l' || c == 'L') state = 6;
            else if(c == 'f' || c == 'F') state = 13;
            else {
                // 防止回退过多，主要是删除换行符等
                return end_number(number,c);
            }
            break;
        case 12:
            if(c == 'l' || c == 'L') state = 6;
            else if(c == 'f' || c == 'F') state = 13;
            else {
                // 防止回退过多，主要是删除换行符等
                return end_number(number,c);
            }
            break;
        case 13:
            if(c == 'f' || c == 'F') state = 13;
            else {
                // 防止回退过多，主要是删除换行符等
                return end_number(number,c);
            }
            break;
        default:
            // 防止回退过多，主要是删除换行符等
            if(c != FILEEOF){
                number.pop_back();
            }
            rollback(c);
            print(NUMBER,number.c_str());
            return OK;
            break;
        }
        c = getnext(number);
    }

    

    
}

/**
 * @brief 运算符自动机
 * 
 * @return int 
 */
int operation(){
    // 读取运算符的自动机
    string op = "";
    char c = getnext(op);
    int state = 0; // 初始状态
    // + - * / % ++ -- == != > < >= <= && || ! & | ^ ~ << >> = += -= *= /= %= &= |= ^= <<= >>= . ->
    // *= /= %= &= |= ^= == !=
    // ++ -- && || ->
    // << >> <= >= <<= >>=
    // 识别自动机

    while (true)
    {
        info("state: %d",state);
        info("c: %c",c);
        info("op: %s",op.c_str());
        // getchar();
        switch(state){
            case 0:
                if(state_map[c] == OPERATOR || state_map[c] == ANOTATION_OPERATOR || state_map[c] == NUMBER_OPERATOR){
                    state = 1;
                }else{
                    rollback(c);
                    return ERROR;
                }
                break;
            case 1:
                if(state_map[c] != OPERATOR && state_map[c] != ANOTATION_OPERATOR && state_map[c] != NUMBER_OPERATOR){
                    
                    debug("c : %c",c);
                    debug("op : %s",op.c_str());
                    // 特殊，应该turnback,强制回退
                    if(c != '\n' && c != FILEEOF){
                        string str = "";
                        str += c;
                        turnback(str);
                    }

                    if(c != FILEEOF){
                        op.pop_back();
                    }
                    
                    if(op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "++" || op == "--" || op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=" || op == "&&" || op == "||" || op == "!" || op == "&" || op == "|" || op == "^" || op == "~" || op == "<<" || op == ">>" || op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "%=" || op == "&=" || op == "|=" || op == "^=" || op == "<<=" || op == ">>=" || op == "." || op == "->"){
                        print(OPERATOR,op.c_str());
                        return OK;
                    }else{
                        return ERROR;
                    }
                }
                break;
            default :
                rollback(c);
                return ERROR;
                break;
            
        }
        c = getnext(op);
    }
    
}

/**
 * @brief 字符串自动机，带前置的
 * 
 */
int string_mach(string pre){
    // 读取字符串的自动机
    string str = pre;
    char c = getnext(str);
    int state = 0; // 初始状态
    // ".*"
    while(true){
        debug("state: %d , %c : %d",state,c,c);
        switch (state)
        {
        case 0:
            if(c == '"') state = 1;
            else {
                // 不回滚防止循环 
                return ERROR;
            }
            break;
        case 1:
            if(c == '"') state = 2;
            // 判断‘\n’的情况
            else if(c == FILEEOF || c == '\n'){
                if(str[str.length() - 1] == '\n') str.pop_back();
                print(MACHERROR,str.c_str());
                return ERROR;
            }else if(c == '\\'){
                state = 3;
            }
            else state = 1;
            break;
        case 2:
            if(c != FILEEOF)str.pop_back();
            print(STRING,str.c_str());
            rollback(c);
            return OK;
            break;
        case 3:
            if(c == FILEEOF || c == '\n'){
                if(str[str.length() - 1] == '\n') str.pop_back();
                print(MACHERROR,str.c_str());
                return ERROR;
            }else {
                state = 1;
            }
            break;
        default:
            rollback(c);
            return ERROR;
            break;
        }
        c = getnext(str);
    }
}

/**
 * @brief 字符自动机
 * 
 */
int charcon_mach(string pre){
    // 读取字符的自动机,这里不考虑存在转移字符，或者说直接是未知长度的字符
    
    // 接受前置的字符串，比如 u'123'd等等
    string str = pre;

    char c = getnext(str);
    debug("CHARCON IN PRE !");
    debug("c: %c",c);
    debug("%s",str.c_str());
    // 字符的长度
    int charcon = 0;

    int state = 0; // 初始状态
    // '.*'
    while(true){
        debug("state: %d , %c : %d",state,c,c);
        switch (state)
        {
        case 0:
            if(c == '\'') state = 1;
            else {
                // 不回滚防止循环 
                return ERROR;
            }
            break;
        case 1:
            if(c == '\'') state = 2;
            else if(c == '\\') state = 3;
            else if(c == FILEEOF || !is_charcon(str)){
                if(str[str.length() - 1] == '\n') str.pop_back();
                print(MACHERROR,str.c_str());
                return ERROR;
            }
            else state = 1;
            break;
        case 2:
            if(c != FILEEOF) str.pop_back();
            print(CHARCON,str.c_str());
            rollback(c);
            return OK;
            break;
        case 3:
            if(c == FILEEOF || c == '\n'){
                if(str[str.length() - 1] == '\n') str.pop_back();
                print(MACHERROR,str.c_str());
                return ERROR;
            }else {
                state = 1;
            }
            break;
        default:
            rollback(c);
            return ERROR;
            break;
        }
        c = getnext(str);
        charcon++;
    }
}

/**
 * @brief 标识符自动机
 * 
 * return int 指明是否是关键字，成功和失败
 */
int keyword_identifier_mach(){
    // 读取标识符的自动机
    string str = "";
    char c = getnext(str);
    int state = 0; // 初始状态
    // [a-zA-Z_][a-zA-Z0-9_]*
    while(true){
        switch (state)
        {
        case 0:
            if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') state = 1;
            else {
                // 不回滚防止循环 ，其实这个可能性不存在，为了健壮性
                return ERROR;
            }
            break;
        case 1:
            if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9')) state = 1;
            else {

                // 防止回退过多，主要是删除换行符等
                if(c != FILEEOF){
                    str.pop_back();
                }
                
                rollback(c);
                // 判断是否是关键字
                if(is_keyword(str)){
                    print(KEYWORD,str.c_str());
                    return KEYWORD;
                }else{
                    print(IDENTIFIER,str.c_str());
                    return IDENTIFIER;
                }
                return OK;
            }
            break;
        default:
            return ERROR;
            break;
        }
        c = getnext(str);
    }
}

#pragma endregion

#pragma endregion

#pragma region TEST FUNCTION

#pragma region TEST DEBUG AND INFO
/**
 * @brief 
 * 
 * @return int 
 */
int Test_0(){
    info("TEST DEBUG and INFO");
    info("Test_0");
    debug("Test_0");
    return OK;
}
#pragma endregion

#pragma region TEST READ_FILE
/**
 * @brief 测试read_file函数
 * 
 * @return int 
 */
int Test_1(){
    info("TEST read_file");

    while (read_file(fp) == OK){
        info("buffer_fp: %d, buffer_ap: %d", buffer_fp, buffer_ap);
        for(int i = buffer_fp; i != buffer_ap; i = (i+1)%BUFFERSIZE){
            std::printf("%c", buffer[i]);
            
            if((buffer_fp+1)%BUFFERSIZE == buffer_ap) break;
            
            buffer_fp = (buffer_fp+1)%BUFFERSIZE;
        }
        std::printf("\n");
    }
    
    info("Last buffer_fp: %d, buffer_ap: %d", buffer_fp, buffer_ap);
    
    for(int i = buffer_fp; i != buffer_ap; i = (i+1)%BUFFERSIZE){
        std::printf("%c", buffer[i]);
        buffer_fp = (buffer_fp+1)%BUFFERSIZE;
    }
    std::printf("\n");

    return OK;
}
#pragma endregion

#pragma region TEST ANNOTATION
void Test_2(){
    info("TEST annotation");
    
    // 多个缓存区充满
    while(!is_end()){
        annotation();
    }
}
#pragma endregion

#pragma region TEST STATE_MAP
int Test_3(){
    info("TEST state_map");
    init_state_map();
    for(int i = 0; i < 128; i++){
        if(i%10 == 0) std::printf("\n");
        printf("%c: ",i);
        switch (state_map[i])
        {
        case FILEEOF:
            printf("FILEEOF");
            break;
        case ANOTATION:
            printf("ANOTATION");
            break;
        case KEYWORD:
            printf("KEYWORD");
            break;
        case IDENTIFIER:
            printf("IDENTIFIER");
            break;
        case NUMBER:
            printf("NUMBER");
            break;
        case OPERATOR:
            printf("OPERATOR");
            break;
        case DELIMITER:
            printf("DELIMITER");
            break;
        case STRING:
            printf("STRING");
            break;
        case CHARCON:
            printf("CHARCON");
            break;
        case MACHERROR:
            printf("MACHERROR");
            break;
        case DEFAULT:
            printf("DEFAULT");
            break;
        case KEYWORD_IDENTIFIER:
            printf("KEYWORD_IDENTIFIER");
            break;
        case NUMBER_OPERATOR:
            printf("NUMBER_OPERATOR");
            break;
        case ANOTATION_OPERATOR:
            printf("ANOTATION_OPERATOR");
            break;
        case DEFINE:
            printf("DEFINE");
            break;
        default:
            break;
        }
        printf(" ");
    }
    std::printf("\n");
    return OK;
}
#pragma endregion

#pragma endregion


int main(int argc, char* argv[]){

    string file_name = argv[1]; //该字符串变量为识别的C语言程序文件名
    
    // 打开输出文件
    if(DEBUGLEVEL){
        if((fp_out = fopen("out.txt", "w")) == NULL){
            info("OPEN OUTFILE FAIL!");
            return 0;
        }
        printf("OPEN OUTFILE SUCCESS!\n");
    }
    
    if((fp = fopen(file_name.c_str(), "r")) == NULL){
        info("OPEN FILE FAIL!");
        printf("OPEN FILE FAIL!\n");
        return 0;
    }

    // 初始化状态转换表
    init_state_map();
    
    // 读取缓存区
    read_file(fp);
    int status;
    // 定义临时字符容器
    string tmp;
    while(!is_end()){
        char c = getnext();
        
        info("working on %c",c);

        switch (state_map[c]){
        case FILEEOF:
            // 结束
            break;
        case DELIMITER:
            // 界符比较简单，直接输出
            getnext(tmp);
            print(DELIMITER,tmp.c_str());
            add(DELIMITER);
            break;
        case IDENTIFIER:     
        case KEYWORD_IDENTIFIER:
            if((c == 'L' || c == 'u' || c == 'U') ){
                if(getsecond() == '"' || getsecond() == '8' && getthird() == '"'){
                    // 字符串 初始化str
                    while(c = getnext(tmp) != '"');
                    tmp.pop_back();
                    // 把 " 回滚回去
                    turnback("\"");

                    if(string_mach(tmp) == ERROR){
                        // 错误
                        add_error();
                    }else{
                        add(STRING);
                    }
                    tmp.clear();
                    continue;
                }else if(getsecond() == '\''){
                    // 宽字符或者unicode字符,初始化char_str
                    while(c = getnext(tmp) != '\'');
                    
                    tmp.pop_back();
                    
                    // 把 ‘ 回滚回去
                    turnback("\'");

                    if(charcon_mach(tmp) == ERROR){
                        // 错误
                        add_error();
                    }else{
                        add(CHARCON);
                    }
                    tmp.clear();
                    continue;
                }
            }
            
            
            // 关键字和标识符
            status = keyword_identifier_mach();
            if(status == ERROR){
                // 错误
                add_error();
            }else if(status == KEYWORD){
                add(KEYWORD);
            }else{
                add(IDENTIFIER);
            }
            break;
        case STRING:
            // 字符串
            if(string_mach(tmp) == ERROR){
                // 错误
                add_error();
            }else{
                add(STRING);
            }
            break;
        case CHARCON:
            // 字符
            if(charcon_mach(tmp) == ERROR){
                // 错误
                add_error();
            }else{
                add(CHARCON);
            }
            break;
        case NUMBER:
            // 常量
            if(number() == ERROR){
                // 错误
                add_error();
            }else{
                add(NUMBER);
            }
            break;
        case NUMBER_OPERATOR:
            // 常量和运算符 主要针对.23这种情况，如果识别失败则回退并且识别运算符
            if(state_map[getsecond()] == NUMBER){
                // 数字常量
                if(number() == ERROR){
                    // 错误,尝试运算符
                    if(operation() == ERROR){
                        // 错误
                        add_error();
                    }
                }else{
                    add(NUMBER);
                }
            }else{
                // 运算符
                if(operation() == ERROR){
                    // 错误
                    add_error();
                }else{
                    add(OPERATOR);
                }
            }
            break;
        case ANOTATION_OPERATOR:
            // 注释和运算符
            if(getsecond() == '/' || getsecond() == '*'){
                // 单行注释
                annotation();
            }else{
                // 除号匹配,OPEARTOR
                if(operation() == ERROR){
                    // 错误
                    add_error();
                }else{
                    add(OPERATOR);
                }
            }
            break;
        case OPERATOR:
            // 运算符
            if(operation() == ERROR){
                // 错误
                add_error();
            }else{
                add(OPERATOR);
            }
            break;
        case DEFINE:
            // 宏定义，不做处理
            define();
            break;
        case MACHERROR:
            getnext(tmp);
            // 自动机错误，自动计入错误
            print(MACHERROR,tmp.c_str());
            add_error();
            break;
        case DEFAULT:
            // 空格、换行、制表符
            getnext(tmp); // go on
            break;
        default:
            debug("UNKNOW STATE: %c",c);
            break;
        }
        tmp.clear();
    }

    // 统计输出
    printf("%d\n",line);
    printf("%d ",keyword_num);
    printf("%d ",identifier_num);
    printf("%d ",operator_num);
    printf("%d ",delimiter_num);
    printf("%d ",charcon_num);
    printf("%d ",string_num);
    printf("%d\n",number_num);

    printf("%d\n",error);


    /*  
      程序每次运行只需要考虑识别一个C语言程序;
      需要使用读取文件的操作,可以在自己的本地环境里创建样例文件进行测试；
      在自己的IDE里测试时可以将file_name赋其他的值，
      但注意，最终测评时由平台提供的main参数来传入文件名
    */
    fclose(fp);
    if(DEBUGLEVEL) fclose(fp_out);
	return 0;
}


/*
    1。 +1
    2. ’\n'

*/