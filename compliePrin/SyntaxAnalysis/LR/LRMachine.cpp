#include<bits/stdc++.h> 

using namespace std;
#pragma region // �����ķ� �ֶ�����
vector<string> Vn = { "E","T","F","S" };
vector<string> Vt = { "+","-","*","/","(",")","n","��" };
vector<string> V = { "E","T","F","+","-","*","/","(",")","n","��" };
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

// �������ʽ�Ҳ�
typedef struct ProductionRight {
    int type;
    string right;
}ProductionRight;

// �������ʽ
typedef struct Production {
    int id;
    string left;
    vector<ProductionRight> rights; 
}Production;

typedef struct LRProject {
    int dot; // ���λ�� ��һ�����ǵȴ���ȡ��λ��, size��ʶ��Լ��Ŀ
    int ProductionID; // ����ʽ
    int type; // ��Ŀ���� 0: �ƽ� 1: ��Լ 2: ����
    set<char> lookAhead; // ��ǰ������
}LRProject;

typedef struct LRProjectSet {
    int id; // ��Ŀ�����
    vector<LRProject> LRProjects; // ��Ŀ����
    map<char, int> gotoMap; // goto�� 
}LRProjectSet;

typedef struct LRAction {
    int type;
    int value;
}LRAction;

#pragma endregion

#pragma region FUNCTION
// ���ɲ���ʽ
Production getProduction(string production){
    Production p;
    int index = production.find("->");
    p.left = production.substr(0, index);
    string right = production.substr(index + 2);
    // ���ұߴ���
    int i = 0;
    string temp = "";
    while (i < right.length()) {
        ProductionRight pr;
        temp += right[i];
        // �ж��Ƿ�Ϊ�ս��
        if (find(Vt.begin(), Vt.end(), temp) != Vt.end()) {
            if(temp == "��")
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
        // ����
        printf("ERROR: %s\n", production.c_str());
        exit(0);
    }
    return p;
}

// ���ÿ�����ս���õ�FIRST��
int getFIRSTItem(char need,map<char, set<string>>& first,map<char,bool> solve_vis,map<char,vector<Production>> ProductionMap){
    // ���need��Ӧ��FIRST��
    
    // �����ж��Ƿ��������
    if(solve_vis.count(need)){
        return -1;
    }
    solve_vis[need] = true;
    // �������в���ʽ
    vector<Production> productions = ProductionMap[need];
    for(int i = 0;i < productions.size();i++){
        // ��ÿһ������ʽ���д��� �õ���һ��ProductionRight
        ProductionRight pr = productions[i].rights[0];
        if(pr.type == TERMINAL){
            // ������ս�� ֱ�Ӽ���
            first[need].insert(pr.right);
        }else if(pr.type == NOT_TERMINAL){
            // ����Ƿ��ս�� �ݹ����
            int res = getFIRSTItem(pr.right[0],first,solve_vis,ProductionMap);
            if(res == -1){
                continue;
            }
            // ����������������ķ��ս��
            for(int j = 0;j < first[pr.right[0]].size();j++){
                first[need].insert(first[pr.right[0]].begin(),first[pr.right[0]].end());
            }
        }else if(pr.type == EPSILON){
            // ����ǿ�
            first[need].insert("��");
        }
    }
    solve_vis.erase(need);
    return 0;
}

// ����FIRST��
map<char, set<string>> getFIRST(map<char,vector<Production>> ProductionMap){
    // ����FIRST��
    map<char, set<string>> FIRST;
    map<char,bool> is_solving;
    // �������з��ս��
    for(int i = 0;i < Vn.size();i++){
        // �������в���ʽ
        char left = Vn[i][0];
        if(FIRST.find(left) != FIRST.end()){
            continue;
        }else{
            getFIRSTItem(left,FIRST,is_solving,ProductionMap);
        }
    }
    return FIRST;
}

// ��ӡ��Ŀ��
void printLRProjectset(LRProjectSet lrps,vector<Production> p){
    // ��ӡ��Ŀ��
    printf("��Ŀ�� %d:\n",lrps.id);
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
    printf("goto��:\n");
    if(lrps.gotoMap.size() == 0){
        printf("��\n");
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
    // ������ͷ
    printf("LRTable:\n");
    vector<vector<string>> table_body;
    vector<string> table_head;
    table_head.push_back("״̬");
    for(int i = 0;i < Vt.size();i++){
        if(Vt[i] == "��"){
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
    
    // ��ӡ ���߷ָ�
    for(int i = 0;i < table_body.size();i++){
        for(int j = 0;j < table_body[i].size();j++){
            printf("%s\t",table_body[i][j].c_str());
        }
        printf("\n");
    }
}
// ������Ŀ������Ψһ��ʶ�ַ���,��ֹ�ظ�
string generateSetId(LRProjectSet lrps){
    string res = "";
    vector<string> temp;

    // ����ʽ��� dot �Լ� lookAhead��ͬ����ͬһ����Ŀ
    for(int i = 0;i < lrps.LRProjects.size();i++){
        string temp_str = "";
        temp_str += to_string(lrps.LRProjects[i].ProductionID);
        temp_str += "-";
        temp_str += to_string(lrps.LRProjects[i].dot);
        temp_str += "-";
        // ��lookAhead��������
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

// ������Ŀ����Ψһ��ʶ�ַ���,��ֹ�ظ�
string generateProjectionId(LRProject lrp){
    string res = "";
    res += to_string(lrp.ProductionID);
    res += "-";
    res += to_string(lrp.dot);
    return res;
}

// ������Ŀ��
vector<LRProjectSet> getLRProject(vector<Production> p,map<char, set<string>> FIRST,map<char,vector<Production>> ProductionMap){
    vector<LRProjectSet> LRPS;
    // ʹ�ö�������¼��Ҫ���д������Ŀ����� ʹ�ö��������й����������
    queue<LRProjectSet> q;

    int production_set_id = 0;
    // ��ʼ��Ŀ �ع��ֻ��һ����Ŀ
    LRProject lrp;
    lrp.dot = 0;
    lrp.ProductionID = 0;
    lrp.lookAhead.insert('$');
    lrp.type = SHIFT;

    map<string,int> set_vis; // ��Ŀ��vis
    // ��ʼ����һ����Ŀ��
    LRProjectSet begin;
    begin.id = production_set_id++;
    begin.LRProjects.push_back(lrp);
    set_vis[generateSetId(begin)] = 0;

    // ���LRPS�еĵ�һ��Ԫ��
    q.push(begin);
    // ����GOTO��
    while(!q.empty()){
        // ���� ȡ������Ԫ��
        LRProjectSet lrps = q.front();
        q.pop();
        // printf("���ڴ��� %d\n",lrps.id);
        // printLRProjectset(lrps,p);
        // 1. �ع�lrps��Ŀ�� ���õ�ԭ������Ŀ��
        vector<LRProject> temp = lrps.LRProjects; //temp�����޸�
        map<string,bool> projection_vis; // ��Ŀvis ��ʶĳ����Ŀ�Ƿ��Ѿ�����
        map<char,bool> waiting_vis; // ��ʱvis ��ʶĳ�����ս���Ƿ��Ѿ���temp�г��ֹ�
        queue<char> waiting_queue; // ��ʱ���� ��ʶ��Ҫ�����ع�ķ��ս��
        // ��ʼ��vis
        for(int i = 0;i < temp.size();i++){
            projection_vis[generateProjectionId(temp[i])] = true; // �Ѿ����ڵ���Ŀ
            // ��������ع�����
            if(temp[i].type == SHIFT && p[temp[i].ProductionID].rights[temp[i].dot].type == NOT_TERMINAL){
                waiting_vis[p[temp[i].ProductionID].rights[temp[i].dot].right[0]] = true; // ��Ҫ�����ع�ķ��ս�� ��Ӽӱ�ʶ
                waiting_queue.push(p[temp[i].ProductionID].rights[temp[i].dot].right[0]);
            }
        }
        // ��ʼ��temp
        while(!waiting_queue.empty()){
            // �õ�����Ԫ��
            char not_terminal = waiting_queue.front();
            waiting_queue.pop();
            // �ҵ����еĲ���ʽ
            vector<Production> productions = ProductionMap[not_terminal];
            for(int j = 0;j < productions.size();j++){
                // �����µ���Ŀ
                LRProject lrp;
                lrp.dot = 0;
                lrp.ProductionID = productions[j].id;
                // �������ʽ����ֻ��һ���� 
                if(productions[j].rights.size() == 1 && productions[j].rights[0].type == EPSILON){
                    lrp.type = REDUCE;
                }else{
                    lrp.type = SHIFT;
                }
                // �ж��Ƿ��Ѿ�����,�������
                if(projection_vis.count(generateProjectionId(lrp))){
                    continue;
                }
                // ��ʶ�Ѿ�����
                projection_vis[generateProjectionId(lrp)] = true;
                // ���뵽��Ŀ����
                lrps.LRProjects.push_back(lrp);

                // �۲��Ƿ���Ҫ����temp�� Ҳ����dot�����Ƿ��ս�� ����û����temp�г��ֹ�
                if(productions[j].rights[0].type == NOT_TERMINAL && !waiting_vis.count(productions[j].rights[0].right[0])){
                    waiting_queue.push(productions[j].rights[0].right[0]);
                    waiting_vis[productions[j].rights[0].right[0]] = true;
                }
            }
        }
        // printf("�ع����\n");
        // 2.���ǰ���������� ��lrps��Ŀ���зḻlookAhead
        bool isChange = true;
        while(isChange){
            isChange = false;
            char solving_char;
            set<char> newLookAhead;

            for(int i = 0;i < lrps.LRProjects.size();i++){
                // ȡ����Ŀ
                LRProject lrp = lrps.LRProjects[i];
                // �ж��Ƿ�Ϊ��Լ��Ŀ ��Լ��Ŀ����Ҫ����ǰ���������ŵļ���
                if(lrp.type == REDUCE || lrp.type == ACCEPT){
                    continue;
                }
                
                // �ж�dot���Ƿ�Ϊ���ս�� ֻ����������Ŀ����Ҫ����ǰ���������ŵļ���
                if(p[lrp.ProductionID].rights[lrp.dot].type == NOT_TERMINAL){
                    // ���newLookAhead
                    newLookAhead.clear();
                    // ����ʽID ��Ӧ��һ���������Ŀ ����dot����ķ��ս����Ӧ�ķ��ս��
                    solving_char = p[lrp.ProductionID].rights[lrp.dot].right[0];

                    // 1. �������������ս�� û�з�����
                    if(lrp.dot == p[lrp.ProductionID].rights.size() - 1){
                        newLookAhead.insert(lrp.lookAhead.begin(), lrp.lookAhead.end());
                    }else{
                        // 2. ���滹�з���
                        // 2.1 ����ķ������ս��
                        if(p[lrp.ProductionID].rights[lrp.dot + 1].type == TERMINAL){
                            newLookAhead.insert(p[lrp.ProductionID].rights[lrp.dot + 1].right[0]);
                        // 2.2 ����ķ����Ƿ��ս��
                        }else if(p[lrp.ProductionID].rights[lrp.dot + 1].type == NOT_TERMINAL){
                            char next_not_terminal = p[lrp.ProductionID].rights[lrp.dot + 1].right[0];
                            // 2.2.1 ����ķ����Ƿ��ս�� �Һ���ķ��ŵ�first���в�������
                            if(FIRST[next_not_terminal].count("��") == 0){
                                for(auto it = FIRST[next_not_terminal].begin();it != FIRST[next_not_terminal].end();it++){
                                    if(*it != "��"){
                                        newLookAhead.insert((*it)[0]);
                                    }
                                }
                            // 2.2.2 ����ķ����Ƿ��ս�� �Һ���ķ��ŵ�first���а����� ��Ҫ�ݹ����
                            }else{
                                int next_not_terminal_dot = lrp.dot + 1;
                                // �ݹ����
                                while(FIRST[next_not_terminal].count("��") == 0){
                                    // ����������з��ŵ�first�����뵽newLookAhead��
                                    for(auto it = FIRST[next_not_terminal].begin();it != FIRST[next_not_terminal].end();it++){
                                        if(*it != "��"){
                                            newLookAhead.insert((*it)[0]);
                                        }
                                    }
                                    // �ж��Ƿ�Ϊ���һ������
                                    if(next_not_terminal_dot == p[lrp.ProductionID].rights.size() - 1){
                                        // �����һ������
                                        newLookAhead.insert(lrp.lookAhead.begin(), lrp.lookAhead.end());
                                        break;
                                    }
                                    // ����next_not_terminal
                                    next_not_terminal_dot++;
                                    next_not_terminal = p[lrp.ProductionID].rights[next_not_terminal_dot].right[0];
                                    // �ж��Ƿ�Ϊ�ս��
                                    if(p[lrp.ProductionID].rights[next_not_terminal_dot].type == TERMINAL){
                                        newLookAhead.insert(p[lrp.ProductionID].rights[next_not_terminal_dot].right[0]);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                // �ҵ���ProductionID��Ӧ�ĵ���Ŀ
                for(int j = 0;j < lrps.LRProjects.size();j++){
                    // ���θ���lookAhead
                    if(p[lrps.LRProjects[j].ProductionID].left[0] == solving_char ){
                        int old_size = lrps.LRProjects[j].lookAhead.size();
                        // ����lookAhead
                        lrps.LRProjects[j].lookAhead.insert(newLookAhead.begin(),newLookAhead.end());
                        // �ж��Ƿ����˸ı�
                        if(lrps.LRProjects[j].lookAhead.size() != old_size){
                            isChange = true;
                        }
                    }
                }
            }
        }
        
        // printf("ǰ���������ż������\n");

        // 3.���goto�� �ҵ������������Ŀ��
        map<char,bool> goto_vis;
        vector<LRProject> projections = lrps.LRProjects;
        for(int goto_index = 0;goto_index < projections.size();goto_index++){
            // �ƽ���Ŀ �Ż���д���
            if(projections[goto_index].type == SHIFT){ 
                char goto_char = p[projections[goto_index].ProductionID].rights[projections[goto_index].dot].right[0];
                if(goto_vis.count(goto_char)){
                    continue; // �Ѿ��������
                }
                // �����µ���Ŀ��
                LRProjectSet newLRPS;
                for(int all_goto_index = 0;all_goto_index < projections.size();all_goto_index++){
                    if(projections[all_goto_index].type != SHIFT){
                        continue;
                    }
                    // ����Ŀ���ַ�
                    if(p[projections[all_goto_index].ProductionID].rights[projections[all_goto_index].dot].right[0] != goto_char){
                        continue;
                    }

                    // �ҵ�һ����ͬ���ƽ���Ŀ
                    LRProject lrp;
                    lrp.dot = projections[all_goto_index].dot + 1;
                    lrp.ProductionID = projections[all_goto_index].ProductionID;
                    // �̳�lookAhead
                    lrp.lookAhead.insert(projections[all_goto_index].lookAhead.begin(),projections[all_goto_index].lookAhead.end());
                    // �ж��Ƿ�Ϊ��Լ��Ŀ
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
                // TODO ������Ҫ����Ŀ������ ��ֹ��ν���
                if(set_vis.count(generateSetId(newLRPS))){
                    lrps.gotoMap[goto_char] = set_vis[generateSetId(newLRPS)]; // ����goto��
                    continue;
                }
                newLRPS.id = production_set_id++;
                set_vis[generateSetId(newLRPS)] = newLRPS.id;
                goto_vis[goto_char] = true;
                lrps.gotoMap[goto_char] = newLRPS.id; // ����goto��
                q.push(newLRPS);
            }
        }
        // printf("goto��������\n");
        // printf("--------------------\n");
        LRPS.push_back(lrps); // ���뵽LRPS��
        // printLRProjectset(lrps,p);
    }

    
    return LRPS;
}

map<char,vector<Production>> getProductionMap(vector<Production> productions){
// �����ķ�ӳ��
    map<char,vector<Production>> productionsMap;
    map<char,bool> is_repeat;
    for(int i = productions.size() - 1;i >= 0 ;i--){
        // ���ڶ���ַ�
        if(productions[i].left.size() > 1 && is_repeat.count(productions[i].left[0])){
            // �Ѿ������� �ҵ��������ظ���
            char temp = 'S'; // ��S��ʼ
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
            // �޸Ĳ���ʽ �� Vn
            Vn.erase(find(Vn.begin(),Vn.end(),productions[i].left));
            Vn.push_back(temp + "");
            productions[i].left = temp + "";
            productionsMap[productions[i].left[0]].push_back(productions[i]);
        }else{ // û���ظ�ֱ�ӹ���
            is_repeat[productions[i].left[0]] = true;
            productionsMap[productions[i].left[0]].push_back(productions[i]);
        }
    }
    return productionsMap;
}

map<int,map<char,LRAction>> getLRtable(vector<LRProjectSet> LRProjectSets,vector<Production> productions){
    map<int,map<char,LRAction>> LRTable;
    // ����������Ŀ��
    for(int i = 0;i < LRProjectSets.size();i++){
        // ����������Ŀ �ҵ����еĹ�Լ��Ŀ
        for(int j = 0;j < LRProjectSets[i].LRProjects.size();j++){
            if(LRProjectSets[i].LRProjects[j].type == REDUCE){
                // ��Լ
                LRAction action;
                action.type = LR_REDUCE;
                action.value = LRProjectSets[i].LRProjects[j].ProductionID;
                // �������е�lookAhead
                for(auto it = LRProjectSets[i].LRProjects[j].lookAhead.begin();it != LRProjectSets[i].LRProjects[j].lookAhead.end();it++){
                    LRTable[i][*it] = action;
                }
            }else if(LRProjectSets[i].LRProjects[j].type == ACCEPT){
                // ����
                LRAction action;
                action.type = LR_ACCEPT;
                action.value = 0;
                LRTable[i]['$'] = action;
            }
        }
        // �������е�goto��
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
    // ���������
    string input;
    cin >> input;
    input += "$";
    // ��ʼ������ջ
    stack<int> analysis_stack;
    analysis_stack.push(0);
    // ��ʼ������ջ
    stack<char> symbol_stack;
    symbol_stack.push('$');
    // ��ʼ�����봮
    int input_index = 0;
    // ��ʼ����
    
    // ������������Լʹ�õĲ���ʽ��� �� shift �� error �� accept(��ʾ��ǰ����Ӧִ�еĶ���)

    while(true){
        // ��ӡ
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

        // ����ջ��״̬
        int top_state = analysis_stack.top();
        // ���뵱ǰ�������
        char current_symbol = input[input_index];
        // ���뵱ǰջ������
        char top_symbol = symbol_stack.top();
        // �ж��Ƿ�Ϊ����״̬
        if(LRTable[top_state][current_symbol].type == LR_ACCEPT){
            printf("accept\n");
            break;
        }
        // �ж��Ƿ�Ϊ�ƽ�״̬
        if(LRTable[top_state][current_symbol].type == LR_GOTO){
            // �ƽ�
            analysis_stack.push(LRTable[top_state][current_symbol].value);
            symbol_stack.push(current_symbol);
            input_index++;
            printf("shift\n");
            continue;
        }
        // �ж��Ƿ�Ϊ��Լ״̬
        if(LRTable[top_state][current_symbol].type == LR_REDUCE){
            // ��Լ
            int production_id = LRTable[top_state][current_symbol].value;
            // �ҵ���Ӧ�Ĳ���ʽ
            Production production = p[production_id];
            // ����ջ���ķ���
            for(int i = 0;i < production.rights.size();i++){
                analysis_stack.pop();
                symbol_stack.pop();
            }
            // ������ʽ��ߵķ��ս����ջ
            analysis_stack.push(LRTable[analysis_stack.top()][production.left[0]].value);
            symbol_stack.push(production.left[0]);
            printf("%d\n",production_id);
            continue;
        }
        // ����
        printf("error\n");
        break;
    }
}

#pragma endregion


int main(){
    // ��ʼ���ķ�
    vector<Production> productions;
    for (int i = 0; i < P.size(); i++) {
        Production p = getProduction(P[i]);
        p.id = i;
        productions.push_back(p);
    }
    // ����ķ�ӳ�� ���������ķ�
    map<char,vector<Production>> productionsMap = getProductionMap(productions);
    
    // ��ʼ��FIRST��
    map<char, set<string>> FIRST;
    
    FIRST = getFIRST(productionsMap);

    // ��ʼ����Ŀ��
    vector<LRProjectSet> LRPS = getLRProject(productions,FIRST,productionsMap);
    
    // ��ӡ��Ŀ��
    // for(int i = 0;i < LRPS.size();i++){
    //     printLRProjectset(LRPS[i],productions);
    // }

    // ����LR������
    map<int,map<char,LRAction>> LRTable;
    LRTable = getLRtable(LRPS,productions);
    // ��ӡLR������
    // printLRtable(LRTable);
    // ����
    // printf("=============================================\n");
    analyse(LRTable,productions);
    return 0;
}