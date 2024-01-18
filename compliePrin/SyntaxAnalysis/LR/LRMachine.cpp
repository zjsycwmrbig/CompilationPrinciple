#include<bits/stdc++.h> 

using namespace std;
#pragma region // 定义文法 手动修正
vector<string> Vn = { "E","T","F","S" };
vector<string> Vt = { "+","-","*","/","(",")","n","ε" };
vector<string> V = { "E","T","F","+","-","*","/","(",")","n","ε" };
vector<string> P = { "S->E","E->E+T","E->E-T","E->T","T->T*F","T->T/F","T->F","F->(E)","F->n" };
#pragma endregion

#pragma region DEFINE
#define NOT_TERMINAL 11110
#define TERMINAL 11111
#define EPSILON 11112

#define SHIFT 11113
#define REDUCE 11114
#define ACCEPT 11115

#define LR_GOTO 11116
#define LR_REDUCE 11117
#define LR_ACCEPT 11118

#pragma endregion


#pragma region STRUCT

// 定义产生式右部
typedef struct ProductionRight {
    int type;
    string right;
}ProductionRight;

// 定义产生式
typedef struct Production {
    int id;
    string left;
    vector<ProductionRight> rights; 
}Production;

typedef struct LRProject {
    int dot; // 点的位置 下一个就是等待读取的位置, size标识规约项目
    int ProductionID; // 产生式
    int type; // 项目类型 0: 移进 1: 规约 2: 接受
    set<char> lookAhead; // 向前看符号
}LRProject;

typedef struct LRProjectSet {
    int id; // 项目集编号
    vector<LRProject> LRProjects; // 项目集合
    map<char, int> gotoMap; // goto表 
}LRProjectSet;

typedef struct LRAction {
    int type;
    int value;
}LRAction;

#pragma endregion

#pragma region FUNCTION
// 生成产生式
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
        printf("ERROR: %s\n", production.c_str());
        exit(0);
    }
    return p;
}

// 针对每个非终结符得到FIRST集
int getFIRSTItem(char need,map<char, set<string>>& first,map<char,bool> solve_vis,map<char,vector<Production>> ProductionMap){
    // 求解need对应的FIRST集
    
    // 首先判断是否正在求解
    if(solve_vis.count(need)){
        return -1;
    }
    solve_vis[need] = true;
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
            int res = getFIRSTItem(pr.right[0],first,solve_vis,ProductionMap);
            if(res == -1){
                continue;
            }
            // 将结果加入现在求解的非终结符
            for(int j = 0;j < first[pr.right[0]].size();j++){
                first[need].insert(first[pr.right[0]].begin(),first[pr.right[0]].end());
            }
        }else if(pr.type == EPSILON){
            // 如果是空
            first[need].insert("ε");
        }
    }
    solve_vis.erase(need);
    return 0;
}

// 构造FIRST集
map<char, set<string>> getFIRST(map<char,vector<Production>> ProductionMap){
    // 构建FIRST集
    map<char, set<string>> FIRST;
    map<char,bool> is_solving;
    // 遍历所有非终结符
    for(int i = 0;i < Vn.size();i++){
        // 遍历所有产生式
        char left = Vn[i][0];
        if(FIRST.find(left) != FIRST.end()){
            continue;
        }else{
            getFIRSTItem(left,FIRST,is_solving,ProductionMap);
        }
    }
    return FIRST;
}

// 打印项目集
void printLRProjectset(LRProjectSet lrps,vector<Production> p){
    // 打印项目集
    printf("项目集 %d:\n",lrps.id);
    for(int i = 0;i < lrps.LRProjects.size();i++){
        printf("%s->",p[lrps.LRProjects[i].ProductionID].left.c_str());
        for(int k = 0;k < p[lrps.LRProjects[i].ProductionID].rights.size();k++){
            if(k == lrps.LRProjects[i].dot){
                printf(".");
            }
            printf("%s",p[lrps.LRProjects[i].ProductionID].rights[k].right.c_str());
        }
        if(lrps.LRProjects[i].dot == p[lrps.LRProjects[i].ProductionID].rights.size()){
            printf(".");
        }
        printf("  ");
        
        printf("type: ");
        if(lrps.LRProjects[i].type == SHIFT){
            printf("SHIFT  ");
        }else if(lrps.LRProjects[i].type == REDUCE){
            printf("REDUCE  ");
        }else if(lrps.LRProjects[i].type == ACCEPT){
            printf("ACCEPT  ");
        }
        
        printf("lookAhead: ");
        for(auto it = lrps.LRProjects[i].lookAhead.begin();it != lrps.LRProjects[i].lookAhead.end();it++){
            printf("%c ",*it);
        }
        printf("\n");
    }
    printf("goto表:\n");
    if(lrps.gotoMap.size() == 0){
        printf("空\n");
        return;
    }
    for(auto it = lrps.gotoMap.begin();it != lrps.gotoMap.end();it++){
        printf("%c->%d\n",it->first,it->second);
    }
}

void printProjection(LRProject lrp,vector<Production> p){
    printf("%s->",p[lrp.ProductionID].left.c_str());
    for(int k = 0;k < p[lrp.ProductionID].rights.size();k++){
        if(k == lrp.dot){
            printf(".");
        }
        printf("%s",p[lrp.ProductionID].rights[k].right.c_str());
    }
    if(lrp.dot == p[lrp.ProductionID].rights.size()){
        printf(".");
    }
    printf("  ");
    
    printf("type: ");
    if(lrp.type == SHIFT){
        printf("SHIFT  ");
    }else if(lrp.type == REDUCE){
        printf("REDUCE  ");
    }else if(lrp.type == ACCEPT){
        printf("ACCEPT  ");
    }
    
    printf("lookAhead: ");
    for(auto it = lrp.lookAhead.begin();it != lrp.lookAhead.end();it++){
        printf("%c ",*it);
    }
    printf("\n");
}

void printLRtable(map<int,map<char,LRAction>> LRTable){
    // 构建表头
    printf("LRTable:\n");
    vector<vector<string>> table_body;
    vector<string> table_head;
    table_head.push_back("状态");
    for(int i = 0;i < Vt.size();i++){
        if(Vt[i] == "ε"){
            continue;
        }
        table_head.push_back(Vt[i]);
    }
    table_head.push_back("$");

    for(int i = 0;i < Vn.size();i++){
        if(Vn[i] == "S"){
            continue;
        }
        table_head.push_back(Vn[i]);
    }
    table_body.push_back(table_head);
    for(int i = 0;i < LRTable.size();i++){
        vector<string> temp;
        temp.push_back(to_string(i));
        for(int j = 1;j < table_head.size();j++){
            if(LRTable[i].count(table_head[j][0])){
                if(LRTable[i][table_head[j][0]].type == LR_GOTO){
                    temp.push_back("G" + to_string(LRTable[i][table_head[j][0]].value));
                }else if(LRTable[i][table_head[j][0]].type == LR_REDUCE){
                    temp.push_back("R" + to_string(LRTable[i][table_head[j][0]].value));
                }else if(LRTable[i][table_head[j][0]].type == LR_ACCEPT){
                    temp.push_back("ACC");
                }
            }else{
                temp.push_back("");
            }
        }
        table_body.push_back(temp);
    }
    
    // 打印 用线分隔
    for(int i = 0;i < table_body.size();i++){
        for(int j = 0;j < table_body[i].size();j++){
            printf("%s\t",table_body[i][j].c_str());
        }
        printf("\n");
    }
}
// 根据项目集产生唯一标识字符串,防止重复
string generateSetId(LRProjectSet lrps){
    string res = "";
    vector<string> temp;

    // 产生式编号 dot 以及 lookAhead相同就是同一个项目
    for(int i = 0;i < lrps.LRProjects.size();i++){
        string temp_str = "";
        temp_str += to_string(lrps.LRProjects[i].ProductionID);
        temp_str += "-";
        temp_str += to_string(lrps.LRProjects[i].dot);
        temp_str += "-";
        // 对lookAhead进行排序
        vector<char> temp_lookAhead;
        for(auto it = lrps.LRProjects[i].lookAhead.begin();it != lrps.LRProjects[i].lookAhead.end();it++){
            temp_lookAhead.push_back(*it);
        }
        sort(temp_lookAhead.begin(),temp_lookAhead.end());
        for(int j = 0;j < temp_lookAhead.size();j++){
            temp_str += temp_lookAhead[j];
        }
        temp.push_back(temp_str);
    }
    
    sort(temp.begin(),temp.end());
    
    for(int i = 0;i < temp.size();i++){
        res += temp[i];
        res += "=";
    }

    return res;
}

// 根据项目产生唯一标识字符串,防止重复
string generateProjectionId(LRProject lrp){
    string res = "";
    res += to_string(lrp.ProductionID);
    res += "-";
    res += to_string(lrp.dot);
    return res;
}

// 生成项目集
vector<LRProjectSet> getLRProject(vector<Production> p,map<char, set<string>> FIRST,map<char,vector<Production>> ProductionMap){
    vector<LRProjectSet> LRPS;
    // 使用队列来记录需要进行处理的项目集序号 使用队列来进行广度优先搜索
    queue<LRProjectSet> q;

    int production_set_id = 0;
    // 初始项目 拓广后只有一个项目
    LRProject lrp;
    lrp.dot = 0;
    lrp.ProductionID = 0;
    lrp.lookAhead.insert('$');
    lrp.type = SHIFT;

    map<string,int> set_vis; // 项目集vis
    // 初始化第一个项目集
    LRProjectSet begin;
    begin.id = production_set_id++;
    begin.LRProjects.push_back(lrp);
    set_vis[generateSetId(begin)] = 0;

    // 入队LRPS中的第一个元素
    q.push(begin);
    // 创建GOTO表
    while(!q.empty()){
        // 引用 取出队首元素
        LRProjectSet lrps = q.front();
        q.pop();
        // printf("正在处理 %d\n",lrps.id);
        // printLRProjectset(lrps,p);
        // 1. 拓广lrps项目集 先拿到原本的项目集
        vector<LRProject> temp = lrps.LRProjects; //temp可以修改
        map<string,bool> projection_vis; // 项目vis 标识某个项目是否已经存在
        map<char,bool> waiting_vis; // 临时vis 标识某个非终结符是否已经在temp中出现过
        queue<char> waiting_queue; // 临时队列 标识需要进行拓广的非终结符
        // 初始化vis
        for(int i = 0;i < temp.size();i++){
            projection_vis[generateProjectionId(temp[i])] = true; // 已经存在的项目
            // 如果满足拓广条件
            if(temp[i].type == SHIFT && p[temp[i].ProductionID].rights[temp[i].dot].type == NOT_TERMINAL){
                waiting_vis[p[temp[i].ProductionID].rights[temp[i].dot].right[0]] = true; // 需要进行拓广的非终结符 入队加标识
                waiting_queue.push(p[temp[i].ProductionID].rights[temp[i].dot].right[0]);
            }
        }
        // 初始化temp
        while(!waiting_queue.empty()){
            // 拿到队首元素
            char not_terminal = waiting_queue.front();
            waiting_queue.pop();
            // 找到所有的产生式
            vector<Production> productions = ProductionMap[not_terminal];
            for(int j = 0;j < productions.size();j++){
                // 产生新的项目
                LRProject lrp;
                lrp.dot = 0;
                lrp.ProductionID = productions[j].id;
                // 如果产生式不是只有一个空 
                if(productions[j].rights.size() == 1 && productions[j].rights[0].type == EPSILON){
                    lrp.type = REDUCE;
                }else{
                    lrp.type = SHIFT;
                }
                // 判断是否已经存在,如果存在
                if(projection_vis.count(generateProjectionId(lrp))){
                    continue;
                }
                // 标识已经存在
                projection_vis[generateProjectionId(lrp)] = true;
                // 加入到项目集中
                lrps.LRProjects.push_back(lrp);

                // 观察是否需要放入temp中 也就是dot后面是非终结符 但是没有在temp中出现过
                if(productions[j].rights[0].type == NOT_TERMINAL && !waiting_vis.count(productions[j].rights[0].right[0])){
                    waiting_queue.push(productions[j].rights[0].right[0]);
                    waiting_vis[productions[j].rights[0].right[0]] = true;
                }
            }
        }
        // printf("拓广完毕\n");
        // 2.求解前进搜索符号 从lrps项目集中丰富lookAhead
        bool isChange = true;
        while(isChange){
            isChange = false;
            char solving_char;
            set<char> newLookAhead;

            for(int i = 0;i < lrps.LRProjects.size();i++){
                // 取出项目
                LRProject lrp = lrps.LRProjects[i];
                // 判断是否为规约项目 规约项目不需要进行前进搜索符号的计算
                if(lrp.type == REDUCE || lrp.type == ACCEPT){
                    continue;
                }
                
                // 判断dot后是否为非终结符 只有这样的项目才需要进行前进搜索符号的计算
                if(p[lrp.ProductionID].rights[lrp.dot].type == NOT_TERMINAL){
                    // 清除newLookAhead
                    newLookAhead.clear();
                    // 产生式ID 对应着一个特殊的项目 就是dot后面的非终结符对应的非终结符
                    solving_char = p[lrp.ProductionID].rights[lrp.dot].right[0];

                    // 1. 后面除了这个非终结符 没有符号了
                    if(lrp.dot == p[lrp.ProductionID].rights.size() - 1){
                        newLookAhead.insert(lrp.lookAhead.begin(), lrp.lookAhead.end());
                    }else{
                        // 2. 后面还有符号
                        // 2.1 后面的符号是终结符
                        if(p[lrp.ProductionID].rights[lrp.dot + 1].type == TERMINAL){
                            newLookAhead.insert(p[lrp.ProductionID].rights[lrp.dot + 1].right[0]);
                        // 2.2 后面的符号是非终结符
                        }else if(p[lrp.ProductionID].rights[lrp.dot + 1].type == NOT_TERMINAL){
                            char next_not_terminal = p[lrp.ProductionID].rights[lrp.dot + 1].right[0];
                            // 2.2.1 后面的符号是非终结符 且后面的符号的first集中不包含ε
                            if(FIRST[next_not_terminal].count("ε") == 0){
                                for(auto it = FIRST[next_not_terminal].begin();it != FIRST[next_not_terminal].end();it++){
                                    if(*it != "ε"){
                                        newLookAhead.insert((*it)[0]);
                                    }
                                }
                            // 2.2.2 后面的符号是非终结符 且后面的符号的first集中包含ε 需要递归求解
                            }else{
                                int next_not_terminal_dot = lrp.dot + 1;
                                // 递归求解
                                while(FIRST[next_not_terminal].count("ε") == 0){
                                    // 将后面的所有符号的first集加入到newLookAhead中
                                    for(auto it = FIRST[next_not_terminal].begin();it != FIRST[next_not_terminal].end();it++){
                                        if(*it != "ε"){
                                            newLookAhead.insert((*it)[0]);
                                        }
                                    }
                                    // 判断是否为最后一个符号
                                    if(next_not_terminal_dot == p[lrp.ProductionID].rights.size() - 1){
                                        // 是最后一个符号
                                        newLookAhead.insert(lrp.lookAhead.begin(), lrp.lookAhead.end());
                                        break;
                                    }
                                    // 更新next_not_terminal
                                    next_not_terminal_dot++;
                                    next_not_terminal = p[lrp.ProductionID].rights[next_not_terminal_dot].right[0];
                                    // 判定是否为终结符
                                    if(p[lrp.ProductionID].rights[next_not_terminal_dot].type == TERMINAL){
                                        newLookAhead.insert(p[lrp.ProductionID].rights[next_not_terminal_dot].right[0]);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                // 找到和ProductionID对应的的项目
                for(int j = 0;j < lrps.LRProjects.size();j++){
                    // 依次更新lookAhead
                    if(p[lrps.LRProjects[j].ProductionID].left[0] == solving_char ){
                        int old_size = lrps.LRProjects[j].lookAhead.size();
                        // 更新lookAhead
                        lrps.LRProjects[j].lookAhead.insert(newLookAhead.begin(),newLookAhead.end());
                        // 判断是否发生了改变
                        if(lrps.LRProjects[j].lookAhead.size() != old_size){
                            isChange = true;
                        }
                    }
                }
            }
        }
        
        // printf("前进搜索符号计算完毕\n");

        // 3.求解goto表 找到下面可求解的项目集
        map<char,bool> goto_vis;
        vector<LRProject> projections = lrps.LRProjects;
        for(int goto_index = 0;goto_index < projections.size();goto_index++){
            // 移进项目 才会进行处理
            if(projections[goto_index].type == SHIFT){ 
                char goto_char = p[projections[goto_index].ProductionID].rights[projections[goto_index].dot].right[0];
                if(goto_vis.count(goto_char)){
                    continue; // 已经处理过了
                }
                // 构建新的项目集
                LRProjectSet newLRPS;
                for(int all_goto_index = 0;all_goto_index < projections.size();all_goto_index++){
                    if(projections[all_goto_index].type != SHIFT){
                        continue;
                    }
                    // 不是目标字符
                    if(p[projections[all_goto_index].ProductionID].rights[projections[all_goto_index].dot].right[0] != goto_char){
                        continue;
                    }

                    // 找到一个相同的移进项目
                    LRProject lrp;
                    lrp.dot = projections[all_goto_index].dot + 1;
                    lrp.ProductionID = projections[all_goto_index].ProductionID;
                    // 继承lookAhead
                    lrp.lookAhead.insert(projections[all_goto_index].lookAhead.begin(),projections[all_goto_index].lookAhead.end());
                    // 判断是否为规约项目
                    if(lrp.dot == p[lrp.ProductionID].rights.size()){
                        lrp.type = REDUCE;
                        if(lrp.ProductionID == 0){
                            lrp.type = ACCEPT;
                        }
                    }else{
                        lrp.type = SHIFT;
                    }
                    newLRPS.LRProjects.push_back(lrp);
                }
                // TODO 这里需要对项目集判重 防止多次进入
                if(set_vis.count(generateSetId(newLRPS))){
                    lrps.gotoMap[goto_char] = set_vis[generateSetId(newLRPS)]; // 更新goto表
                    continue;
                }
                newLRPS.id = production_set_id++;
                set_vis[generateSetId(newLRPS)] = newLRPS.id;
                goto_vis[goto_char] = true;
                lrps.gotoMap[goto_char] = newLRPS.id; // 更新goto表
                q.push(newLRPS);
            }
        }
        // printf("goto表计算完毕\n");
        // printf("--------------------\n");
        LRPS.push_back(lrps); // 加入到LRPS中
        // printLRProjectset(lrps,p);
    }

    
    return LRPS;
}

map<char,vector<Production>> getProductionMap(vector<Production> productions){
// 构建文法映射
    map<char,vector<Production>> productionsMap;
    map<char,bool> is_repeat;
    for(int i = productions.size() - 1;i >= 0 ;i--){
        // 存在多个字符
        if(productions[i].left.size() > 1 && is_repeat.count(productions[i].left[0])){
            // 已经存在了 找到不存在重复的
            char temp = 'S'; // 从S开始
            for(int j = 0;j < 26;j++){
                if(!is_repeat.count(temp)){
                    break;
                }
                temp = temp + 1;
                if(temp > 'Z'){
                    temp = 'A';
                }
                if(j == 25){
                    printf("ERROR: %s\n",P[i].c_str());
                    exit(0);
                }
            }
            // 修改产生式 和 Vn
            Vn.erase(find(Vn.begin(),Vn.end(),productions[i].left));
            Vn.push_back(temp + "");
            productions[i].left = temp + "";
            productionsMap[productions[i].left[0]].push_back(productions[i]);
        }else{ // 没有重复直接构建
            is_repeat[productions[i].left[0]] = true;
            productionsMap[productions[i].left[0]].push_back(productions[i]);
        }
    }
    return productionsMap;
}

map<int,map<char,LRAction>> getLRtable(vector<LRProjectSet> LRProjectSets,vector<Production> productions){
    map<int,map<char,LRAction>> LRTable;
    // 遍历所有项目集
    for(int i = 0;i < LRProjectSets.size();i++){
        // 遍历所有项目 找到所有的规约项目
        for(int j = 0;j < LRProjectSets[i].LRProjects.size();j++){
            if(LRProjectSets[i].LRProjects[j].type == REDUCE){
                // 规约
                LRAction action;
                action.type = LR_REDUCE;
                action.value = LRProjectSets[i].LRProjects[j].ProductionID;
                // 遍历所有的lookAhead
                for(auto it = LRProjectSets[i].LRProjects[j].lookAhead.begin();it != LRProjectSets[i].LRProjects[j].lookAhead.end();it++){
                    LRTable[i][*it] = action;
                }
            }else if(LRProjectSets[i].LRProjects[j].type == ACCEPT){
                // 接受
                LRAction action;
                action.type = LR_ACCEPT;
                action.value = 0;
                LRTable[i]['$'] = action;
            }
        }
        // 遍历所有的goto表
        if(LRProjectSets[i].gotoMap.size() == 0){
            continue;
        }
        for(auto it = LRProjectSets[i].gotoMap.begin();it != LRProjectSets[i].gotoMap.end();it++){
            LRTable[i][it->first].type = LR_GOTO;
            LRTable[i][it->first].value = it->second;
        }
    }
    return LRTable;
}

void analyse(map<int,map<char,LRAction>> LRTable,vector<Production> p){
    // 读入分析串
    string input;
    cin >> input;
    input += "$";
    // 初始化分析栈
    stack<int> analysis_stack;
    analysis_stack.push(0);
    // 初始化符号栈
    stack<char> symbol_stack;
    symbol_stack.push('$');
    // 初始化输入串
    int input_index = 0;
    // 开始分析
    
    // 分析动作：归约使用的产生式编号 或 shift 或 error 或 accept(表示当前步骤应执行的动作)

    while(true){
        // 打印
        // printf("analysis_stack: ");
        // stack<int> temp_analysis_stack = analysis_stack;
        // while(!temp_analysis_stack.empty()){
        //     printf("%d ",temp_analysis_stack.top());
        //     temp_analysis_stack.pop();
        // }
        // printf("\n");
        // printf("symbol_stack: ");
        // stack<char> temp_symbol_stack = symbol_stack;
        // while(!temp_symbol_stack.empty()){
        //     printf("%c ",temp_symbol_stack.top());
        //     temp_symbol_stack.pop();
        // }
        // printf("\n");
        // printf("input: ");
        // for(int i = input_index;i < input.length();i++){
        //     printf("%c",input[i]);
        // }
        // printf("\n");
        // printf("=============================================\n");

        // 读入栈顶状态
        int top_state = analysis_stack.top();
        // 读入当前输入符号
        char current_symbol = input[input_index];
        // 读入当前栈顶符号
        char top_symbol = symbol_stack.top();
        // 判断是否为接受状态
        if(LRTable[top_state][current_symbol].type == LR_ACCEPT){
            printf("accept\n");
            break;
        }
        // 判断是否为移进状态
        if(LRTable[top_state][current_symbol].type == LR_GOTO){
            // 移进
            analysis_stack.push(LRTable[top_state][current_symbol].value);
            symbol_stack.push(current_symbol);
            input_index++;
            printf("shift\n");
            continue;
        }
        // 判断是否为规约状态
        if(LRTable[top_state][current_symbol].type == LR_REDUCE){
            // 规约
            int production_id = LRTable[top_state][current_symbol].value;
            // 找到对应的产生式
            Production production = p[production_id];
            // 弹出栈顶的符号
            for(int i = 0;i < production.rights.size();i++){
                analysis_stack.pop();
                symbol_stack.pop();
            }
            // 将产生式左边的非终结符入栈
            analysis_stack.push(LRTable[analysis_stack.top()][production.left[0]].value);
            symbol_stack.push(production.left[0]);
            printf("%d\n",production_id);
            continue;
        }
        // 出错
        printf("error\n");
        break;
    }
}

#pragma endregion


int main(){
    // 初始化文法
    vector<Production> productions;
    for (int i = 0; i < P.size(); i++) {
        Production p = getProduction(P[i]);
        p.id = i;
        productions.push_back(p);
    }
    // 获得文法映射 并且修正文法
    map<char,vector<Production>> productionsMap = getProductionMap(productions);
    
    // 初始化FIRST集
    map<char, set<string>> FIRST;
    
    FIRST = getFIRST(productionsMap);

    // 初始化项目集
    vector<LRProjectSet> LRPS = getLRProject(productions,FIRST,productionsMap);
    
    // 打印项目集
    // for(int i = 0;i < LRPS.size();i++){
    //     printLRProjectset(LRPS[i],productions);
    // }

    // 构建LR分析表
    map<int,map<char,LRAction>> LRTable;
    LRTable = getLRtable(LRPS,productions);
    // 打印LR分析表
    // printLRtable(LRTable);
    // 分析
    // printf("=============================================\n");
    analyse(LRTable,productions);
    return 0;
}