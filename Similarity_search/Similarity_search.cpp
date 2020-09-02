#include <iostream>
#include <fstream> 
#include <string>
#include <vector>
#include <set>
#define maxLen 1000

using namespace std;


class Token {
    //单词对象，目前一个token保存一个单词（字符串）
public:
    string content;
    int hashValue = 0;
    Token(string ct) {
        this->content = ct;
    }
};

void SplitString(const string& s, vector<string>& v, const string& c) {
    //将字符串s以字符c为分割符切割，结果保存于向量v
    string::size_type head, tail;
    tail = s.find(c);
    head = 0;
    while (string::npos != tail) {
        v.push_back(s.substr(head, tail - head));
        head = tail + c.size();
        tail = s.find(c, head);
    }
    if (head != s.length()) {  
        v.push_back(s.substr(head));
    }
}  
 

class Document {
    //文章对象，一个document保存一系列token
public:
    vector<Token> tokens;
    vector<string> words;
    void getWords(string text) {
        SplitString(text, this->words, " ");
    }

    void getTokens() {
        for (vector<string>::size_type i = 0; i != words.size(); i++) {
            //cout << words[i] << endl;
            if (words[i] == " ") { //filter
                continue;
            }
            Token *t = new Token(words[i]);
            tokens.push_back(*t);
        }
    }
    Document(string text) {
        getWords(text);
        getTokens();
    }
    void display() {
        for (vector<Token>::size_type i = 0; i != tokens.size(); i++) {
            cout << tokens[i].content << endl;
        }
    }
};

class Window {
    //窗口对象，包含起始位置indx1和结束位置indx2，保存决定该窗口哈希值的centerWord的所有位置向量，跨越centerWord数量k，和哈希值
public:
    unsigned indx1;
    unsigned indx2;
    vector <unsigned> centerWords;
    unsigned k;
    int hashValue;
    bool operator==(Window p)
    {
        return this->hashValue == p.hashValue && this->k == p.k;
    }
    Window(unsigned indx1, unsigned indx2, vector <unsigned> centerWords, unsigned k, int hashValue) {
        this->indx1 = indx1;
        this->indx2 = indx2;
        this->centerWords = centerWords;
        this->k = k;
        this->hashValue = hashValue;
    }
};

class WindowPair {
    //窗口对对象，仅包含两个窗口的下标对
public:
    unsigned a1;
    unsigned a2;
    unsigned b1;
    unsigned b2;
    WindowPair(unsigned a1,unsigned a2, unsigned b1, unsigned b2) {
        this->a1 = a1;
        this->a2 = a2;
        this->b1 = b1;
        this->b2 = b2;
    }
    bool operator==(WindowPair p)
    {
        return this->a1 == p.a1 && this->a2 == p.a2 && this->b1 == p.b1 && this->b2 == p.b2;
    }
};

class CrashMap {
    //冲突图，保存窗口对冲撞，以计算出冲撞次数符合要求的区域（对应窗口对）
public:
    int map[maxLen][maxLen];

    void init() {
        for (size_t i = 0; i < maxLen; i++)
        {
            for (size_t j = 0; j < maxLen; j++)
            {
                map[i][j] = 0;
            }
        }
    }

    void addArea(WindowPair wp) {
        for (size_t i = wp.a1; i <= wp.a2; i++)
        {
            for (size_t j = wp.b1; j <= wp.b2; j++)
            {
                map[i][j] = map[i][j] + 1;
            }
        }
    }

    int queryArea(WindowPair wp) {
        int ans = 0;
        for (size_t i = wp.a1; i <= wp.a2; i++)
        {
            for (size_t j = wp.b1; j <= wp.b2; j++)
            {
                ans += map[i][j];
            }
        }
        return ans;
    }

    void disp(int n) {
        for (size_t i = 0; i < n; i++)
        {
            for (size_t j = 0; j < n; j++)
            {
                if (map[i][j] >= 400) {
                    cout << i << " " << j << " " << map[i][j] << endl;
                }

            }
        }
    }
};

struct SegmentTree2DNode {
    //2D片段数，之前用于存储冲撞信息
    int a1;
    int a2;
    int b1;
    int b2;
    int min;
    SegmentTree2DNode* lu;
    SegmentTree2DNode* ld;
    SegmentTree2DNode* ru;
    SegmentTree2DNode* rd;
};

void genWindow(unsigned indx1, unsigned indx2, Document d, vector<Window>* w, unsigned ssh) {
    //新的生成窗口的方法。暴力枚举了所有子窗口
    if (indx2 < ssh + indx1) {
        return;
    }
    int minVal, value;
    vector<unsigned> loc;
    minVal = RAND_MAX;
    for (vector<Token>::size_type i = indx1; i <= indx2; i++) {
        value = d.tokens[i].hashValue;
        if (value < minVal) {
            loc.clear();
            loc.push_back(i);
            minVal = value;
        }
        else if (value == minVal) {
            loc.push_back(i);
        }
    }

    for (size_t i = 0; i < loc.size(); i++)
    {
        unsigned x, y;
        for (x = 0; x < loc.at(1); x++)
        {
            if (i + 1 >= loc.size()) {
                break;
            }
            for (y = loc.at(i); y < loc.at(i + 1); y++)
            {
                if (y < ssh + x) {
                    continue;
                }
                Window* win = new Window(x, y, loc, i + 1, minVal);
                w->push_back(*win);
            }
        }
    }

    if (loc.at(0) > 0) {   
        genWindow(indx1, loc.at(0) - 1, d, w, ssh);
    }
    if (loc.at(0) < indx2) {
        genWindow(loc.at(0) + 1, indx2, d, w, ssh);
    }


}

void build2DTree(SegmentTree2DNode* t,int a1,int a2,int b1,int b2,int vals[maxLen][maxLen]) {
    //旧的创造2D片段树的方法
    if (a1 == a2 && b1 == b2) {
        t->a1 = a1;
        t->a2 = a2;
        t->b1 = b1;
        t->b2 = b2;
        t->min = vals[a1][b1];
        return;
    }
    int midA, midB;
    midA = (a1 + a2) / 2;
    midB = (b1 + b2) / 2;
    SegmentTree2DNode* lu, * ld, * ru, * rd;
    int min, tmp;
    lu = new SegmentTree2DNode();
    ld = new SegmentTree2DNode();
    ru = new SegmentTree2DNode();
    rd = new SegmentTree2DNode();
    //cout << "to build " << a1 << " " << a2 << " " << b1 << " " << b2 << " \n";
    if (a1 == a2) {
        build2DTree(lu, a1, a2, midB + 1, b2, vals);
        build2DTree(ld, a1, a2, b1, midB, vals);
        min = lu->min < ld->min ? lu->min : ld->min;
        t->lu = lu;
        t->ld = ld;
    }
    else if(b1 == b2)
    {
        build2DTree(ld, a1, midA, b1, b2, vals);
        build2DTree(rd, midA + 1, a2, b1, b2, vals);
        min = ld->min < rd->min ? ld->min : rd->min;
        t->ld = ld;
        t->rd = rd;
    }
    else
    {
        build2DTree(lu, a1, midA, midB + 1, b2, vals);
        build2DTree(ld, a1, midA, b1, midB, vals);
        build2DTree(ru, midA + 1, a2, midB + 1, b2, vals);
        build2DTree(rd, midA + 1, a2, b1, midB, vals);
        tmp = lu->min < ld->min ? lu->min : ld->min;
        min = ru->min < rd->min ? ru->min : rd->min;
        min = tmp < min ? tmp : min;
        t->lu = lu;
        t->ld = ld;
        t->ru = ru;
        t->rd = rd;
    }
   
    //struct SegmentTree2DNode ans = { a1, a2, b1, b2, min, lu, ld, ru, rd };
    t->a1 = a1;
    t->a2 = a2;
    t->b1 = b1;
    t->b2 = b2;
    t->min = min;
}

int query2DTree(SegmentTree2DNode* root ,int a1, int a2, int b1, int b2) {
    //旧的查询2D片段树的方法
    if (root->a1 == a1 && root->a2 == a2 && root->b1 == b1 && root->b2 == b2) {
        return root->min;
    }
    int midA, midB;
    midA = (root->a1 + root->a2) / 2;
    midB = (root->b1 + root->b2) / 2;
    int lu, ld, ru, rd;
    if (a1 <= midA && midA < a2) {
        if (b1 <= midB && midB < b2) {
            lu = query2DTree(root->lu, a1, midA, midB+1, b2);
            ld = query2DTree(root->ld, a1, midA, b1, midB);
            ru = query2DTree(root->ru, midA+1, a2, midB+1, b2);
            rd = query2DTree(root->rd, midA+1, a2, b1, midB);
            int min, tmp;
            tmp = lu < ld ? lu : ld;
            min = ru < rd ? ru : rd;
            min = tmp < min ? tmp : min;
            return min;
        }
        else {
            if (midB < b1) {
                lu = query2DTree(root->lu, a1, midA, b1, b2);
                ru = query2DTree(root->ru, midA+1, a2, b1, b2);
                int min = ru < lu ? ru : lu;
                return min;
            }
            else {
                ld = query2DTree(root->ld, a1, midA, b1, b2);
                rd = query2DTree(root->rd, midA+1, a2, b1, b2);
                int min = rd < ld ? rd : ld;
                return min;
            }
        }
    }
    else {
        if (midA < a1) {
            if (b1 <= midB && midB < b2) {
                ru = query2DTree(root->ru, a1, a2, midB+1, b2);
                rd = query2DTree(root->rd, a1, a2, b1, midB);
                int min;
                min = ru < rd ? ru : rd;
                return min;
            }
            else {
                if (midB < b1) {
                    return query2DTree(root->ru, a1, a2, b1, b2);
                }
                else {
                    return query2DTree(root->rd, a1, a2, b1, b2);
                }
            }
        }
        else {
            if (b1 <= midB && midB < b2) {
                lu = query2DTree(root->lu, a1, a2, midB+1, b2);
                ld = query2DTree(root->ld, a1, a2, b1, midB);
                int min;
                min = lu < ld ? lu : ld;
                return min;
            }
            else {
                if (midB < b1) {
                    return query2DTree(root->lu, a1, a2, b1, b2);
                }
                else {
                    return query2DTree(root->ld, a1, a2, b1, b2);
                }
            }
        }
    }
    
   
}

int RanHash(string text, int seed) {
    //根据输入生成随机哈希值
    std::hash<std::string> hash_fn;
    int hash = hash_fn(text);
    srand((seed+1)*hash);
    return rand();
}

void compareSimilarity(Document d1, Document d2, int seed, vector<WindowPair> &vp, int ssh) {
    //旧的比较两个document相似度的方法，目前由于存在错误已经不用
    vector<Token>::size_type l1, l2;
    vector<Window> w1, w2;
    
    l1 = d1.tokens.size();
    l2 = d2.tokens.size();   
    w1.clear();
    w2.clear();
    //get hash value for all tokens
    for (vector<Token>::size_type i = 0; i != l1; i++) {  
        d1.tokens[i].hashValue = RanHash(d1.tokens[i].content, seed);
    }
    for (vector<Token>::size_type i = 0; i != l2; i++) {
        d2.tokens[i].hashValue = RanHash(d2.tokens[i].content, seed);
    }

    int lastI1, lastI2;
    lastI1 = lastI2 = -1;
    for (vector<Token>::size_type i = 0; i != l1; i++) {
        int i1, i2, value, j;
        i1 = i2 = i;
        value = d1.tokens[i].hashValue;
        j = i - 1;
        while (j >= 0)
        {
            if (d1.tokens[j].hashValue >= value)
            {
                i1 = j;
                j--;
            }
            else
            {
               
                break;
            }
        }
        j = i + 1;
        while (j < l1)
        {
            if (d1.tokens[j].hashValue >= value)
            {
                i2 = j;
                j++;
            }
            else
            {
                break;
            }
        }
        if (i2 - i1 < ssh) {
            continue;
        }
        /*Window *w = new Window();
        if (i1 != lastI1 || i2 != lastI2) {
            w->indx1 = i1;
            w->indx2 = i2;
            w->center = i;
            w->hashValue = value;
            w1.push_back(*w);
        }
        lastI1 = i1;
        lastI2 = i2;*/
        
    }
    lastI1 = lastI2 = -1;
    for (vector<Token>::size_type i = 0; i != l2; i++) {
        int i1, i2, value, j;
        i1 = i2 = i;
        value = d2.tokens[i].hashValue;
        j = i - 1;
        while (j >= 0)
        {
            if (d2.tokens[j].hashValue >= value)
            {
                i1 = j;
                j--;
            }
            else
            {
                break;
            }
        }
        j = i + 1;
        while (j < l2)
        {
            if (d2.tokens[j].hashValue >= value)
            {
                i2 = j;
                j++;
            }
            else
            {
                break;
            }
        }/*
        if (i1 != lastI1 || i2 != lastI2) {
            Window* w = new Window();
            w->indx1 = i1;
            w->indx2 = i2;
            w->center = i;
            w->hashValue = value;
            w2.push_back(*w);
        } 
        lastI1 = i1;
        lastI2 = i2;*/
    }
    /*
    for (vector<Window>::size_type i = 0; i != w1.size(); i++) {
        cout << " [ "<< w1[i].indx1 << " , " << w1[i].indx2 << " ] " << w1[i].hashValue << endl;
    }
    cout << "##############################" << endl;
    for (vector<Window>::size_type i = 0; i != w2.size(); i++) {
        cout << " [ " << w2[i].indx1 << " , " << w2[i].indx2 << " ] " << w2[i].hashValue << endl;
    }*/
    int ct = 0;
    for (vector<Window>::size_type i = 0; i != w1.size(); i++) {
        for (vector<Window>::size_type j = 0; j != w2.size(); j++) {
            if (w1[i].hashValue == w2[j].hashValue) {
                WindowPair* wp = new WindowPair(w1[i].indx1, w1[i].indx2, w2[j].indx1, w2[j].indx2);
                vp.push_back(*wp);
                ct++;
                //cout << " [ " << w1[i].indx1 << " , " << w1[i].indx2 << " ] " <<" , " << " [ " << w2[j].indx1 << " , " << w2[j].indx2 << " ] " << endl;
            }
        }
    }


    //cout << "total cash:" << ct << endl;
}

int main()
{   
    //主程序，命令式交互
    string command;
    vector<Document> dataDocuments;
    vector<WindowPair> resultPairs;
    cout << "Welcome\n";
    while (true)    
    {
        cin >> command;
        if (command == "test")
        {
            resultPairs.clear();
            for (size_t i = 0; i < 100; i++)
            {
                compareSimilarity(dataDocuments[0], dataDocuments[1], i, resultPairs, 5);
            }
            CrashMap *cm = new CrashMap();
            cm->init();
            for (vector<WindowPair>::size_type i = 0; i != resultPairs.size(); i++) {
                cm->addArea(resultPairs[i]);
                //cout << " [ " << resultPairs[i].a1<< " , " << resultPairs[i].a2 << " ] " << " [ " << resultPairs[i].b1 << " , " << resultPairs[i].b2 << " ] "<< endl;
            }
            SegmentTree2DNode* t = new SegmentTree2DNode();
            build2DTree(t, 0, 800, 0, 800, cm->map);
            
            cm->disp(200);
            cout << query2DTree(t, 40, 55, 66, 74);
        }

        else if (command == "load")
        {
            ifstream ifs;
            ifs.open("test.txt", ios::in);//reut2-000.sgm
            if (!ifs.is_open()) {
                cout << "file open error" << endl;
            }
            else {
                string buf, text = "";
                string::size_type pos;
                bool inBody = false;
                while (getline(ifs, buf))
                {
                    pos = buf.find("<BODY>");
                    if (buf.find("</BODY>") != buf.npos) {
                        inBody = false;
                        Document *dd = new Document(text);
                        dataDocuments.push_back(*dd);
                        text = "";
                    }
                    else  if (pos != buf.npos) {
                        inBody = true;
                        text.append(buf.substr(pos + 6));
                        text.append(" ");
                    }
                    else {
                        if (inBody) {
                            text.append(buf);
                            text.append(" ");
                        }
                    }
                }
                ifs.close();
            }
        }
        else if (command == "neww")
        {
            vector<Window>* gw1 = new vector<Window>;
            vector<Window>* gw2 = new vector<Window>;
            genWindow(0, dataDocuments[0].words.size() - 1, dataDocuments[0], gw1, 3);
            genWindow(0, dataDocuments[1].words.size() - 1, dataDocuments[1], gw2, 3);
            CrashMap* cm = new CrashMap();
            cm->init();
            for (vector<Window>::size_type i = 0; i < gw1->size(); i++)
            {
                for (vector<Window>::size_type j = 0; j < gw2->size(); j++)
                {
                    if (gw1->at(i).hashValue == gw2->at(j).hashValue)
                    {
                        WindowPair* wp = new WindowPair(gw1->at(i).indx1, gw1->at(i).indx2, gw2->at(j).indx1, gw2->at(j).indx2);
                        cm->addArea(*wp);  
                    }
                    
                }
            }

        }
        else if (command == "tree")
        {
            static int a[maxLen][maxLen];
            for (size_t i = 0; i < maxLen; i++)
            {
                for (size_t j = 0; j < maxLen; j++)
                {
                    a[i][j] = i+j;
                }
            }
            SegmentTree2DNode* t = new SegmentTree2DNode();
            build2DTree(t, 0, 800, 0, 800, a);
            cout << query2DTree(t, 3, 637, 101, 784);
        }
        else if (command == "quit")
        {
            break;
        }
        else {
            cout << "wrong command" << endl;
        }
    }

}

  