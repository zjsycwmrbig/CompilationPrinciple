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

#pragma region // �����ķ�
vector<string> Vn = { "E","A","T","B","F" };
vector<string> Vt = { "+","-","*","/","(",")","n","��" };
vector<string> V = { "E","A","T","B","F","+","-","*","/","(",")","num","��" };
vector<string> P = { "E->TA","A->+TA","A->-TA","A->��","T->FB","B->*FB","B->/FB","B->��","F->(E)","F->n" };
#pragma endregion

#pragma region 

// �������ʽ�Ҳ�
typedef struct ProductionRight {
    int type;
    string right;
}ProductionRight;

// �������ʽ
typedef struct Production {
    string left;
    vector<ProductionRight> rights; 
}Production;

typedef struct FollowTask {
    int type; // ����
    char nt; // ���ս��
}FollowTask;


#pragma endregion


#pragma region

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
        exit(PRODUCTION_PARSE_ERROR);
    }
    return p;
}

// ����ķ��õ�����ʽ��FISRT����
vector<string> getFIRSTChars(map<char, set<string>> FIRST,Production production){
    ProductionRight pr = production.rights[0];
    vector<string> firstChars;
    if(pr.type == TERMINAL){
        // ������ս�� ֱ�Ӽ���
        firstChars.push_back(pr.right);
    }else if(pr.type == NOT_TERMINAL){
        // ����Ƿ��ս�� �ݹ����
        for(auto it = FIRST[pr.right[0]].begin();it != FIRST[pr.right[0]].end();it++){
            firstChars.push_back(*it);
        }
    }else if(pr.type == EPSILON){
        // ����ǿ�
        firstChars.push_back("��");
    }
    return firstChars;
}
// ���ÿ�����ս���õ�FIRST��
int getFIRSTItem(char need,map<char, set<string>>& first,map<char,bool> vis,map<char,vector<Production>> ProductionMap){
    // ���need��Ӧ��FIRST��
    // �����ж��Ƿ��������
    if(vis[need]){
        exit(HAVING_LOOP_DEFINE);
    }
    vis[need] = true;
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
            getFIRSTItem(pr.right[0],first,vis,ProductionMap);
            // ����������������ķ��ս��
            for(int j = 0;j < first[pr.right[0]].size();j++){
                first[need].insert(first[pr.right[0]].begin(),first[pr.right[0]].end());
            }
        }else if(pr.type == EPSILON){
            // ����ǿ�
            first[need].insert("��");
        }
    }
    vis.erase(need);
    return 0;
}
// ����FIRST��
map<char, set<string>> getFIRST(map<char,vector<Production>> ProductionMap){
    // ����FIRST��
    map<char, set<string>> FIRST;
    map<char,bool> vis;
    // �������з��ս��
    for(int i = 0;i < Vn.size();i++){
        // �������в���ʽ
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
    bool isChange = true; // ֻ��FOLLOW�������仯ʱ�ż���
    
    while (isChange){
        isChange = false;
        for(int i = 0;i < productions.size();i++){
            Production p = productions[i];
            // ��������ʽ�Ҳ�
            for(int j = 0;j < p.rights.size();j++){
                ProductionRight pr = p.rights[j];
                if(pr.type == NOT_TERMINAL){
                    // ������ս����ͷ����'$'
                    if(pr.right[0] == Vn[0][0]){
                        if(FOLLOW.find(pr.right[0]) != FOLLOW.end()){
                            // �����ж��Ƿ�����µ�Ԫ�ؼ���
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
                        // ��������һ�����ս�� ֱ�Ӽ���
                        FollowTask ft;
                        ft.type = FOLLOW_NEDD_FOLLOW;
                        ft.nt = p.left[0];
                        followTasks.push_back(ft);
                    }else{
                        ProductionRight pr_next = p.rights[j + 1];
                        if(pr_next.type == TERMINAL){
                            // ������ս�� ֱ�Ӽ���
                            if(FOLLOW.find(pr.right[0]) != FOLLOW.end()){
                                // �����ж��Ƿ�����µ�Ԫ�ؼ���
                                if(FOLLOW[pr.right[0]].find(pr_next.right) == FOLLOW[pr.right[0]].end()){
                                    FOLLOW[pr.right[0]].insert(pr_next.right);
                                    isChange = true;
                                }
                            }else{
                                FOLLOW[pr.right[0]].insert(pr_next.right);
                                isChange = true;
                            }
                        }else if(pr_next.type == NOT_TERMINAL){
                            // ����Ƿ��ս�� ����FIRST��
                            FollowTask ft;
                            ft.type = FOLLOW_NEDD_FIRST;
                            ft.nt = pr_next.right[0];
                            followTasks.push_back(ft);
                            ProductionRight pr_temp = pr_next;
                            int tmp_index = j+1; // ĿǰҪ����λ��
                            while(pr_temp.type == NOT_TERMINAL && FIRST[pr_temp.right[0]].find("��") != FIRST[pr_temp.right[0]].end()){
                                // ����Ƿ��ս�� ��FIRST�����п�
                                if(tmp_index == p.rights.size() - 1){
                                    // ��������һ��
                                    FollowTask ft;
                                    ft.type = FOLLOW_NEDD_FOLLOW;
                                    ft.nt = p.left[0];
                                    followTasks.push_back(ft);
                                    break;
                                }else{
                                    // ����������һ��
                                    ProductionRight pr_next = p.rights[tmp_index + 1];
                                    if(pr_next.type == TERMINAL){
                                        // ������ս�� ֱ�Ӽ���
                                        if(FOLLOW.find(pr_temp.right[0]) != FOLLOW.end()){
                                            // �����ж��Ƿ�����µ�Ԫ�ؼ���
                                            if(FOLLOW[pr_temp.right[0]].find(pr_next.right) == FOLLOW[pr_temp.right[0]].end()){
                                                FOLLOW[pr_temp.right[0]].insert(pr_next.right);
                                                isChange = true;
                                            }
                                        }else{
                                            FOLLOW[pr_temp.right[0]].insert(pr_next.right);
                                            isChange = true;
                                        }
                                    }else if(pr_next.type == NOT_TERMINAL){
                                        // ����Ƿ��ս�� ����FIRST��
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
                    // ��followTasks���д���
                    for(int i = 0;i < followTasks.size();i++){
                        if(followTasks[i].type == FOLLOW_NEDD_FIRST){
                            // ��FIRST������
                            for(auto it = FIRST[followTasks[i].nt].begin();it != FIRST[followTasks[i].nt].end();it++){
                                if(FOLLOW.find(pr.right[0]) != FOLLOW.end()){
                                    // �����ж��Ƿ�����µ�Ԫ�ؼ���
                                    if(*it == "��"){
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
                            // ��FOLLOW������
                            for(auto it = FOLLOW[followTasks[i].nt].begin();it != FOLLOW[followTasks[i].nt].end();it++){
                                if(FOLLOW.find(pr.right[0]) != FOLLOW.end()){
                                    // �����ж��Ƿ�����µ�Ԫ�ؼ���
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
    // �����ķ�˳�򹹽�Ԥ�������
    map<char,map<string,int>> LLtable; // ָ�����ʽ�����
    for(int i = 0;i < produstions.size();i++){
        // 1. �õ�fistChars
        vector<string> firstChars = getFIRSTChars(FIRST,produstions[i]);
        for(int j = 0;j < firstChars.size();j++){
            if(firstChars[j] == "��"){
                // ����ǿ�
                // 2. �õ�followChars
                vector<string> followChars;
                for(auto it = FOLLOW[produstions[i].left[0]].begin();it != FOLLOW[produstions[i].left[0]].end();it++){
                    followChars.push_back(*it);
                }
                // 3. ��followChars����LLtable
                for(int k = 0;k < followChars.size();k++){
                    LLtable[produstions[i].left[0]][followChars[k]] = i;
                }
            }else{
                // 2. ��firstChars����LLtable
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
        // �鿴����
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
    // ���������ķ����ɲ���ʽ ����Ҳ�����
    vector<Production> productions;
    for (int i = 0; i < P.size(); i++) {
        productions.push_back(getProduction(P[i]));
    }

    // �����ķ�ӳ��
    map<char,vector<Production>> productionsMap;
    for (int i = 0; i < productions.size(); i++) {
        productionsMap[productions[i].left[0]].push_back(productions[i]);
    }

    // ����FIRST��
    map<char, set<string>> FIRST;
    FIRST = getFIRST(productionsMap);
    // ���FIRST��
    // cout<<"FIRST:"<<endl;
    // for (int i = 0; i < Vn.size(); i++) {
    //     cout << Vn[i] << ":";
    //     for (auto it = FIRST[Vn[i][0]].begin(); it != FIRST[Vn[i][0]].end(); it++) {
    //         cout << *it << " ";
    //     }
    //     cout << endl;
    // }

    // ����FOLLOW��
    map<char, set<string>> FOLLOW;
    FOLLOW = getFOLLOW(productions,FIRST);
    // ���FOLLOW��
    // cout<<"FOLLOW:"<<endl;
    // for (int i = 0; i < Vn.size(); i++) {
    //     cout << Vn[i] << ":";
    //     for (auto it = FOLLOW[Vn[i][0]].begin(); it != FOLLOW[Vn[i][0]].end(); it++) {
    //         cout << *it << " ";
    //     }
    //     cout << endl;
    // }
    // ����Ԥ�������
    map<char,map<string,int>> table;
    table = getLLTable(FIRST,FOLLOW,productions);
    // ���Ԥ�������
    // cout<<"LLTable:"<<endl;
    // for (int i = 0; i < Vn.size(); i++) {
    //     cout << Vn[i] << ":";
    //     for (auto it = table[Vn[i][0]].begin(); it != table[Vn[i][0]].end(); it++) {
    //         cout << it->first << " " << it->second << " ";
    //     }
    //     cout << endl;
    // }
    // ��ȡ���봮 
    string input;
    cin >> input;
    input += "$";
    // ��ʼ����
    string stack = "$";
    stack += Vn[0][0];
    
    int index = 0;
    while (stack.size() > 0) {
        if(stack[stack.size() - 1] == input[index]){
            // ���ջ�������봮��ͬ
            if(stack[stack.size() - 1] == '$'){
                printf("%s\t%s\t%s\n",stack.c_str(),input.substr(index).c_str(),"accept");
            }else{
                printf("%s\t%s\t%s\n",stack.c_str(),input.substr(index).c_str(),"match");
            }
            stack.pop_back();
            index++;
        }else if(table[stack[stack.size() - 1]].find(input.substr(index,1)) != table[stack[stack.size() - 1]].end()){
            // ���ջ���Ƿ��ս��
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
            // ����
            printf("%s\t%s\t%s\n",stack.c_str(),input.substr(index).c_str(),"error");
            break;
        }
    }

    return 0;
}