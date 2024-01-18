#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<algorithm>
#include<map> 
#include<set>
using namespace std;

#pragma region DEFINE
#define NOT_TERMINAL 11110
#define TERMINAL 11111
#define EPSILON 11112


#define PRODUCTION_PARSE_ERROR 11113
#define HAVING_LOOP_DEFINE 11114
#define FOLLOW_NEDD_FOLLOW 11115
#define FOLLOW_NEDD_FIRST 11116
#pragma endregion

#pragma region // 定义文法
vector<string> Vn = { "E","A","T","B","F" };
vector<string> Vt = { "+","-","*","/","(",")","n","ε" };
vector<string> V = { "E","A","T","B","F","+","-","*","/","(",")","num","ε" };
vector<string> P = { "E->TA","A->+TA","A->-TA","A->ε","T->FB","B->*FB","B->/FB","B->ε","F->(E)","F->n" };
#pragma endregion

#pragma region 

// 定义产生式右部
typedef struct ProductionRight {
    int type;
    string right;
}ProductionRight;

// 定义产生式
typedef struct Production {
    string left;
    vector<ProductionRight> rights; 
}Production;

typedef struct FollowTask {
    int type; // 类型
    char nt; // 非终结符
}FollowTask;


#pragma endregion


#pragma region

Production getProduction(string production){
    Production p;
    int index = production.find("->");
    p.left = production.substr(0, index);
    string right = production.substr(index + 2);
    // 对右边处理
    int i = 0;
    string temp = "";
    while (i < right.length()) {
        ProductionRight pr;
        temp += right[i];
        // 判断是否为终结符
        if (find(Vt.begin(), Vt.end(), temp) != Vt.end()) {
            if(temp == "ε")
                pr.type = EPSILON;
            else{
                pr.type = TERMINAL;
            }
            pr.right = temp;
            p.rights.push_back(pr);
            i++;
            temp = "";
            continue;
        }else if (find(Vn.begin(), Vn.end(), temp) != Vn.end()) {
            pr.type = NOT_TERMINAL;
            pr.right = temp;
            p.rights.push_back(pr);
            i++;
            temp = "";
            continue;
        }else{
            i++;
            continue;
        }
    }
    if(temp != ""){
        // 出错
        exit(PRODUCTION_PARSE_ERROR);
    }
    return p;
}

// 针对文法得到产生式的FISRT集合
vector<string> getFIRSTChars(map<char, set<string>> FIRST,Production production){
    ProductionRight pr = production.rights[0];
    vector<string> firstChars;
    if(pr.type == TERMINAL){
        // 如果是终结符 直接加入
        firstChars.push_back(pr.right);
    }else if(pr.type == NOT_TERMINAL){
        // 如果是非终结符 递归求解
        for(auto it = FIRST[pr.right[0]].begin();it != FIRST[pr.right[0]].end();it++){
            firstChars.push_back(*it);
        }
    }else if(pr.type == EPSILON){
        // 如果是空
        firstChars.push_back("ε");
    }
    return firstChars;
}
// 针对每个非终结符得到FIRST集
int getFIRSTItem(char need,map<char, set<string>>& first,map<char,bool> vis,map<char,vector<Production>> ProductionMap){
    // 求解need对应的FIRST集
    // 首先判断是否正在求解
    if(vis[need]){
        exit(HAVING_LOOP_DEFINE);
    }
    vis[need] = true;
    // 遍历所有产生式
    vector<Production> productions = ProductionMap[need];
    for(int i = 0;i < productions.size();i++){
        // 对每一个产生式进行处理 得到第一个ProductionRight
        ProductionRight pr = productions[i].rights[0];
        if(pr.type == TERMINAL){
            // 如果是终结符 直接加入
            first[need].insert(pr.right);
        }else if(pr.type == NOT_TERMINAL){
            // 如果是非终结符 递归求解
            getFIRSTItem(pr.right[0],first,vis,ProductionMap);
            // 将结果加入现在求解的非终结符
            for(int j = 0;j < first[pr.right[0]].size();j++){
                first[need].insert(first[pr.right[0]].begin(),first[pr.right[0]].end());
            }
        }else if(pr.type == EPSILON){
            // 如果是空
            first[need].insert("ε");
        }
    }
    vis.erase(need);
    return 0;
}
// 构造FIRST集
map<char, set<string>> getFIRST(map<char,vector<Production>> ProductionMap){
    // 构建FIRST集
    map<char, set<string>> FIRST;
    map<char,bool> vis;
    // 遍历所有非终结符
    for(int i = 0;i < Vn.size();i++){
        // 遍历所有产生式
        char left = Vn[i][0];
        if(FIRST.find(left) != FIRST.end()){
            continue;
        }else{
            getFIRSTItem(left,FIRST,vis,ProductionMap);
        }
    }
    return FIRST;
}


map<char, set<string>> getFOLLOW(vector<Production> productions,map<char, set<string>> FIRST){
    map<char, set<string>> FOLLOW;
    bool isChange = true; // 只有FOLLOW集发生变化时才继续
    
    while (isChange){
        isChange = false;
        for(int i = 0;i < productions.size();i++){
            Production p = productions[i];
            // 遍历产生式右部
            for(int j = 0;j < p.rights.size();j++){
                ProductionRight pr = p.rights[j];
                if(pr.type == NOT_TERMINAL){
                    // 如果是终结符开头加入'$'
                    if(pr.right[0] == Vn[0][0]){
                        if(FOLLOW.find(pr.right[0]) != FOLLOW.end()){
                            // 依次判断是否存在新的元素加入
                            if(FOLLOW[pr.right[0]].find("$") == FOLLOW[pr.right[0]].end()){
                                FOLLOW[pr.right[0]].insert("$");
                                isChange = true;
                            }
                        }else{
                            FOLLOW[pr.right[0]].insert("$");
                            isChange = true;
                        }
                    }
                    vector<FollowTask> followTasks;
                    if(j == p.rights.size() - 1){
                        // 如果是最后一个非终结符 直接加入
                        FollowTask ft;
                        ft.type = FOLLOW_NEDD_FOLLOW;
                        ft.nt = p.left[0];
                        followTasks.push_back(ft);
                    }else{
                        ProductionRight pr_next = p.rights[j + 1];
                        if(pr_next.type == TERMINAL){
                            // 如果是终结符 直接加入
                            if(FOLLOW.find(pr.right[0]) != FOLLOW.end()){
                                // 依次判断是否存在新的元素加入
                                if(FOLLOW[pr.right[0]].find(pr_next.right) == FOLLOW[pr.right[0]].end()){
                                    FOLLOW[pr.right[0]].insert(pr_next.right);
                                    isChange = true;
                                }
                            }else{
                                FOLLOW[pr.right[0]].insert(pr_next.right);
                                isChange = true;
                            }
                        }else if(pr_next.type == NOT_TERMINAL){
                            // 如果是非终结符 加入FIRST集
                            FollowTask ft;
                            ft.type = FOLLOW_NEDD_FIRST;
                            ft.nt = pr_next.right[0];
                            followTasks.push_back(ft);
                            ProductionRight pr_temp = pr_next;
                            int tmp_index = j+1; // 目前要求解的位置
                            while(pr_temp.type == NOT_TERMINAL && FIRST[pr_temp.right[0]].find("ε") != FIRST[pr_temp.right[0]].end()){
                                // 如果是非终结符 且FIRST集中有空
                                if(tmp_index == p.rights.size() - 1){
                                    // 如果是最后一个
                                    FollowTask ft;
                                    ft.type = FOLLOW_NEDD_FOLLOW;
                                    ft.nt = p.left[0];
                                    followTasks.push_back(ft);
                                    break;
                                }else{
                                    // 如果不是最后一个
                                    ProductionRight pr_next = p.rights[tmp_index + 1];
                                    if(pr_next.type == TERMINAL){
                                        // 如果是终结符 直接加入
                                        if(FOLLOW.find(pr_temp.right[0]) != FOLLOW.end()){
                                            // 依次判断是否存在新的元素加入
                                            if(FOLLOW[pr_temp.right[0]].find(pr_next.right) == FOLLOW[pr_temp.right[0]].end()){
                                                FOLLOW[pr_temp.right[0]].insert(pr_next.right);
                                                isChange = true;
                                            }
                                        }else{
                                            FOLLOW[pr_temp.right[0]].insert(pr_next.right);
                                            isChange = true;
                                        }
                                    }else if(pr_next.type == NOT_TERMINAL){
                                        // 如果是非终结符 加入FIRST集
                                        FollowTask ft;
                                        ft.type = FOLLOW_NEDD_FIRST;
                                        ft.nt = pr_next.right[0];
                                        followTasks.push_back(ft);
                                        pr_temp = pr_next;
                                    }
                                }

                            }
                        }
                    }
                    // 对followTasks进行处理
                    for(int i = 0;i < followTasks.size();i++){
                        if(followTasks[i].type == FOLLOW_NEDD_FIRST){
                            // 把FIRST集加入
                            for(auto it = FIRST[followTasks[i].nt].begin();it != FIRST[followTasks[i].nt].end();it++){
                                if(FOLLOW.find(pr.right[0]) != FOLLOW.end()){
                                    // 依次判断是否存在新的元素加入
                                    if(*it == "ε"){
                                        continue;
                                    }
                                    if(FOLLOW[pr.right[0]].find(*it) == FOLLOW[pr.right[0]].end()){
                                        FOLLOW[pr.right[0]].insert(*it);
                                        isChange = true;
                                    }
                                }else{
                                    FOLLOW[pr.right[0]].insert(*it);
                                    isChange = true;
                                }
                            }
                        }else{
                            // 把FOLLOW集加入
                            for(auto it = FOLLOW[followTasks[i].nt].begin();it != FOLLOW[followTasks[i].nt].end();it++){
                                if(FOLLOW.find(pr.right[0]) != FOLLOW.end()){
                                    // 依次判断是否存在新的元素加入
                                    if(FOLLOW[pr.right[0]].find(*it) == FOLLOW[pr.right[0]].end()){
                                        FOLLOW[pr.right[0]].insert(*it);
                                        isChange = true;
                                    }
                                }else{
                                    FOLLOW[pr.right[0]].insert(*it);
                                    isChange = true;
                                }
                            }
                        }
                    } 
                }
            }
        }
    }
    
    return FOLLOW;
}

map<char,map<string,int>> getLLTable(map<char, set<string>> FIRST,map<char, set<string>> FOLLOW,vector<Production> produstions){
    // 按照文法顺序构建预测分析表
    map<char,map<string,int>> LLtable; // 指向产生式的序号
    for(int i = 0;i < produstions.size();i++){
        // 1. 得到fistChars
        vector<string> firstChars = getFIRSTChars(FIRST,produstions[i]);
        for(int j = 0;j < firstChars.size();j++){
            if(firstChars[j] == "ε"){
                // 如果是空
                // 2. 得到followChars
                vector<string> followChars;
                for(auto it = FOLLOW[produstions[i].left[0]].begin();it != FOLLOW[produstions[i].left[0]].end();it++){
                    followChars.push_back(*it);
                }
                // 3. 将followChars加入LLtable
                for(int k = 0;k < followChars.size();k++){
                    LLtable[produstions[i].left[0]][followChars[k]] = i;
                }
            }else{
                // 2. 将firstChars加入LLtable
                LLtable[produstions[i].left[0]][firstChars[j]] = i;
            }
        }
    }
    return LLtable;
}

#pragma endregion

#pragma region TEST
void testGetProduction(vector<Production> productions){
    for (int i = 0; i < productions.size(); i++) {
        cout << productions[i].left << "->";
        for (int j = 0; j < productions[i].rights.size(); j++) {
            cout << productions[i].rights[j].right;
        }
        cout << endl;
        // 查看类型
        for (int j = 0; j < productions[i].rights.size(); j++) {
            if(productions[i].rights[j].type == NOT_TERMINAL){
                cout <<"N ";
            }else if(productions[i].rights[j].type == TERMINAL){
                cout <<  "T ";
            }else if(productions[i].rights[j].type == EPSILON){
                cout <<  "E ";
            }
        }
        cout << endl;
    }
}
#pragma endregion

int main(){
    // 根据已有文法生成产生式 这里也有序号
    vector<Production> productions;
    for (int i = 0; i < P.size(); i++) {
        productions.push_back(getProduction(P[i]));
    }

    // 构建文法映射
    map<char,vector<Production>> productionsMap;
    for (int i = 0; i < productions.size(); i++) {
        productionsMap[productions[i].left[0]].push_back(productions[i]);
    }

    // 构建FIRST集
    map<char, set<string>> FIRST;
    FIRST = getFIRST(productionsMap);
    // 输出FIRST集
    // cout<<"FIRST:"<<endl;
    // for (int i = 0; i < Vn.size(); i++) {
    //     cout << Vn[i] << ":";
    //     for (auto it = FIRST[Vn[i][0]].begin(); it != FIRST[Vn[i][0]].end(); it++) {
    //         cout << *it << " ";
    //     }
    //     cout << endl;
    // }

    // 构建FOLLOW集
    map<char, set<string>> FOLLOW;
    FOLLOW = getFOLLOW(productions,FIRST);
    // 输出FOLLOW集
    // cout<<"FOLLOW:"<<endl;
    // for (int i = 0; i < Vn.size(); i++) {
    //     cout << Vn[i] << ":";
    //     for (auto it = FOLLOW[Vn[i][0]].begin(); it != FOLLOW[Vn[i][0]].end(); it++) {
    //         cout << *it << " ";
    //     }
    //     cout << endl;
    // }
    // 构建预测分析表
    map<char,map<string,int>> table;
    table = getLLTable(FIRST,FOLLOW,productions);
    // 输出预测分析表
    // cout<<"LLTable:"<<endl;
    // for (int i = 0; i < Vn.size(); i++) {
    //     cout << Vn[i] << ":";
    //     for (auto it = table[Vn[i][0]].begin(); it != table[Vn[i][0]].end(); it++) {
    //         cout << it->first << " " << it->second << " ";
    //     }
    //     cout << endl;
    // }
    // 读取输入串 
    string input;
    cin >> input;
    input += "$";
    // 开始分析
    string stack = "$";
    stack += Vn[0][0];
    
    int index = 0;
    while (stack.size() > 0) {
        if(stack[stack.size() - 1] == input[index]){
            // 如果栈顶和输入串相同
            if(stack[stack.size() - 1] == '$'){
                printf("%s\t%s\t%s\n",stack.c_str(),input.substr(index).c_str(),"accept");
            }else{
                printf("%s\t%s\t%s\n",stack.c_str(),input.substr(index).c_str(),"match");
            }
            stack.pop_back();
            index++;
        }else if(table[stack[stack.size() - 1]].find(input.substr(index,1)) != table[stack[stack.size() - 1]].end()){
            // 如果栈顶是非终结符
            int productionIndex = table[stack[stack.size() - 1]][input.substr(index,1)];
            printf("%s\t%s\t%d\n",stack.c_str(),input.substr(index).c_str(),productionIndex+1);
            stack.pop_back();
            for(int i = productions[productionIndex].rights.size() - 1;i >= 0;i--){
                if(productions[productionIndex].rights[i].type == EPSILON){
                    continue;
                }
                stack += productions[productionIndex].rights[i].right;
            }
        }else{
            // 出错
            printf("%s\t%s\t%s\n",stack.c_str(),input.substr(index).c_str(),"error");
            break;
        }
    }

    return 0;
}