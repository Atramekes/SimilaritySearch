#include <iostream>
#include <fstream> 
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <windows.h>
#define maxLen 500

using namespace std;

unordered_map<int, vector<int>> whv;
unordered_map<string, int> dictionary; //将token映射为token id
unordered_map<int, int> wordcount;


void SplitString(const string& s, vector<string>& v, const string& delim) {
    //将字符串s以字符c为分割符切割，结果保存于向量v
    string::size_type head, tail;
    tail = s.find(delim);
    head = 0;
    while (string::npos != tail) {
        v.push_back(s.substr(head, tail - head));
        head = tail + delim.size();
        tail = s.find(delim, head);
    }
    if (head != s.length()) {
        v.push_back(s.substr(head));
    }
}

void strToTokens(const string& s, vector<string>& res, const string& delims) {
    string::size_type begIdx, endIdx;
    begIdx = s.find_first_not_of(delims);
    while (begIdx != string::npos) {
        endIdx = s.find_first_of(delims, begIdx);
        if (endIdx == string::npos)
            endIdx = s.length();
        res.push_back(s.substr(begIdx, endIdx - begIdx));
        begIdx = s.find_first_not_of(delims, endIdx);
    }
}


void wordmaxcount(vector<int>& doc, unordered_map<int, int>& wordcount)
{
    //cout << "doc:" << endl;
    unordered_map<int, int> doccount;
    for (auto& entry : doc)
    {
        //cout << entry << " ";
        doccount[entry] += 1;
    }
    for (auto& entry : doccount) {
        if (wordcount[entry.first] < entry.second)
            wordcount[entry.first] = entry.second;
    }
    //cout << endl;
}


class Token {
    //单词对象，目前一个token保存一个单词（字符串）
public:
    string content;
    int hashValue = 0;
    Token(string ct) {
        this->content = ct;
    }
};

void wordtoint(vector<Token> tokens, vector<int>& doc)
{
    string str;
    for (vector<Token>::size_type i = 0; i != tokens.size(); i++) {
        {
            str = tokens[i].content;
            if (dictionary.find(str) == dictionary.end())
            {
                int newid = dictionary.size();
                dictionary[str] = newid;
            }
            doc.push_back(dictionary[str]);
        }
    }
}

class Document {  
    //文章对象，一个document保存一系列token
public:
    vector<Token> tokens;
    vector<string> words;
    vector<int> doc;
    string cleaned(string str) {
        string cl;
        cl.assign(str);
        for (size_t i = 0; i < cl.length(); i++)
        {
            cl[i] = tolower(cl[i]);
            if (!isalnum(cl[i])) {
                cl[i] = ' ';
            }
        }
        return cl;
    }
    Document(string text) {
        SplitString(cleaned(text), this->words, " ");
        for (vector<string>::size_type i = 0; i != words.size(); i++) {
            if (words[i] == "") { //filter
                continue;
            }
            Token* t = new Token(words[i]);
            tokens.push_back(*t);
        }
        wordtoint(tokens,doc);
        wordmaxcount(doc, wordcount);
    }
    void display() {
        for (vector<Token>::size_type i = 0; i != tokens.size(); i++) {
            cout << tokens[i].content << endl;
        }
    }
};

class CompactWindow {
public:
    int left;
    int eos;
    int right;
    vector<int> loc;
    int p;
    int hval;// hash value
    int id=0;
    int hashid;
    CompactWindow(int indx1, int eos,int indx2, vector<int> loc, int p,int hv, int hashid)
        : left(indx1), eos(eos), right(indx2), loc(loc),p(p), hval(hv), hashid(hashid) { }
};


class Window {
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
/*
class WindowPair {
    //窗口对对象，仅包含两个窗口的下标对
public:
    unsigned a1;
    unsigned a2;
    unsigned b1;
    unsigned b2;
    WindowPair(unsigned a1, unsigned a2, unsigned b1, unsigned b2) {
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

};*/



int uni_hash(int val, int a, int b)
{
  srand(a * val + b);
  return abs(rand());
}


void conquer(vector<int> doc,int indx1, int eos, int indx2, unordered_map<int, vector<int>> whv,int hashid, vector<CompactWindow>& results) {
    unordered_map<int, int> freq;
    int minval = 0x7fffffff; 
    int p = -1; //第p个min word取min-hash，注意：访问这个元素应使用下标p-1
    int minword = -1; 

    if (indx1 > eos || indx2 - indx1 <= 0) {
        return;
    }
    for (int i = indx1; i <= indx2; i++)
    {
        int j = freq[doc[i]];
        freq[doc[i]] += 1;
        int hv = whv[doc[i]][j];

        if (hv < minval)
        {
            minval = hv;
            p = j;
            minword = doc[i];
        }
    }
    p = p + 1;
    vector<int> pos;
    for (int i = indx1; i <= indx2; i++)
    {
        if (doc[i] == minword)
        {
            pos.push_back(i);
        }
    }
    int q = pos.size(); //注意：访问最后一个元素应使用下标q-1
    //cout << indx1 << " " << eos << " " << indx2 << " " << endl;
    results.push_back(CompactWindow(indx1, eos, indx2, pos, p, minval, hashid));
    for (size_t k = 0; k <= q - p; k++)
    {
        int left;
        if (k == 0) {
            left = indx1;
        }
        else
        {
            left = pos.at(k - 1) + 1;
        }
        if (eos >= pos.at(k))
        {
            if (p==1) { //保证eos不大于indx2
                conquer(doc, left, pos.at(k) - 1, pos.at(k + p - 1) - 1, whv, hashid, results);
            }
            else {
                conquer(doc, left, pos.at(k), pos.at(k + p - 1) - 1, whv, hashid, results);
            }
            
        }
        else
        {
            conquer(doc, left, eos, pos.at(k + p - 1) - 1, whv, hashid, results);
            return;    
        }
    }
    conquer(doc, pos.at(q - p) + 1, eos, indx2, whv,hashid, results);
}



class IntervalPair {
public:
    int ll;
    int lr;
    int rl;
    int rr;
    int id;
    int hval;

    IntervalPair(int ll, int lr, int rl, int rr, int id, int hv)
        : ll(ll), lr(lr), rl(rl), rr(rr),id(id), hval(hv) { }
};




struct SegmentTree2DNode {
    //2D片段数，之前用于存储冲撞信息
    int a1;
    int a2;
    int b1;
    int b2;     
    //vector<int> inf;
    int maxcount;
    SegmentTree2DNode* lu;
    SegmentTree2DNode* ld;
    SegmentTree2DNode* ru;
    SegmentTree2DNode* rd;
};




void build2DTree(SegmentTree2DNode* t,int a1,int a2,int b1,int b2,vector<int> vals[maxLen][maxLen]) {
    //新创造2D片段树的方法
    if (a1 == a2 && b1 == b2) {
        t->a1 = a1;
        t->a2 = a2;
        t->b1 = b1;
        t->b2 = b2;
        //t->inf = vals[a1][b1];
        t->maxcount = vals[a1][b1].size();
        //cout << t->maxcount << endl;
        return;
    }
    int midA, midB;
    midA = (a1 + a2) / 2;
    midB = (b1 + b2) / 2;
    SegmentTree2DNode* lu, * ld, * ru, * rd;
    int maxc;
    lu = new SegmentTree2DNode();
    ld = new SegmentTree2DNode();
    ru = new SegmentTree2DNode();
    rd = new SegmentTree2DNode();
    //cout << "to build " << a1 << " " << a2 << " " << b1 << " " << b2 << " \n";
    if (a1 == a2) {
        build2DTree(lu, a1, a2, midB + 1, b2, vals);
        build2DTree(ld, a1, a2, b1, midB, vals);
        maxc = max(lu->maxcount, ld->maxcount);
        t->lu = lu;
        t->ld = ld;
        delete(ru);
        delete(rd);
    }
    else if(b1 == b2)
    {
        build2DTree(ld, a1, midA, b1, b2, vals);
        build2DTree(rd, midA + 1, a2, b1, b2, vals);
        maxc = max(ld->maxcount, rd->maxcount);
        t->ld = ld;
        t->rd = rd;
        delete(ru);
        delete(lu);
    }
    else
    {
        build2DTree(lu, a1, midA, midB + 1, b2, vals);
        build2DTree(ld, a1, midA, b1, midB, vals);
        build2DTree(ru, midA + 1, a2, midB + 1, b2, vals);
        build2DTree(rd, midA + 1, a2, b1, midB, vals);
        maxc = max(max(lu->maxcount, ld->maxcount), max(ru->maxcount, rd->maxcount));
        t->lu = lu;
        t->ld = ld;
        t->ru = ru;
        t->rd = rd;
    }
   
    t->a1 = a1;
    t->a2 = a2;
    t->b1 = b1;
    t->b2 = b2;
    t->maxcount = maxc;
}

int query2DTree(SegmentTree2DNode* root ,int a1, int a2, int b1, int b2) {
    //新的查询2D片段树的方法
    if (root->a1 == a1 && root->a2 == a2 && root->b1 == b1 && root->b2 == b2) {
        return root->maxcount;
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
            int maxc = max(max(lu, ld), max(ru, rd));
            return maxc;
        }
        else {
            if (midB < b1) {
                lu = query2DTree(root->lu, a1, midA, b1, b2);
                ru = query2DTree(root->ru, midA+1, a2, b1, b2);
                int maxc = max(ru, lu);
                return maxc;
            }
            else {
                ld = query2DTree(root->ld, a1, midA, b1, b2);
                rd = query2DTree(root->rd, midA+1, a2, b1, b2);
                int maxc = max(rd, ld);
                return maxc;
            }
        }
    }
    else {
        if (midA < a1) {
            if (b1 <= midB && midB < b2) {
                ru = query2DTree(root->ru, a1, a2, midB+1, b2);
                rd = query2DTree(root->rd, a1, a2, b1, midB);
                int maxc = max(ru, rd);
                return maxc;
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
                int maxc = max(ld, lu);
                return maxc;
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

void clean2DTree(SegmentTree2DNode* t) {
    if (t->lu != NULL) {
        clean2DTree(t->lu);
    }
    if (t->ld != NULL) {
        clean2DTree(t->ld);
    }
    if (t->ru != NULL) {
        clean2DTree(t->ru);
    }
    if (t->rd != NULL) {
        clean2DTree(t->rd);
    }
    delete(t->lu);
    delete(t->ld);
    delete(t->ru);
    delete(t->rd);
    t->lu = NULL;
    t->ld = NULL;
    t->ru = NULL;
    t->rd = NULL;
}

vector<int> ipmap[maxLen][maxLen];
vector<int> ipmap2[maxLen][maxLen];

void calSimilarity(Document d1, Document d2, int shuffleTimes, float theta) {
    //新方法比较相似度
    vector<CompactWindow> cw1;
    vector<CompactWindow> cw2;
    for (size_t k = 0; k < shuffleTimes; k++)
    {
        srand(k);
        whv.clear();
        int a = 0;
        while (a == 0)
            a = rand();
        int b = rand();
        int totalwords = dictionary.size();
        for (auto& entry : wordcount)   
        {
            for (auto i = 0; i < entry.second; i++)
            {
                int wid = i * totalwords + entry.first;
                int hv = uni_hash(wid, a, b);
                whv[entry.first].push_back(hv);
            }
        }
        conquer(d1.doc, 0, d1.doc.size() - 1, d1.doc.size() - 1, whv, k, cw1);
        conquer(d2.doc, 0, d2.doc.size() - 1, d2.doc.size() - 1, whv, k, cw2);
    }
    int ssh = (int)shuffleTimes * theta;
    int ipid = 0;
    vector<IntervalPair> iptable;
    vector<IntervalPair> iptable2;
    SegmentTree2DNode* tree = new SegmentTree2DNode();
    
    for (vector<CompactWindow>::size_type w = 0; w != cw1.size(); w++) {
        //cout << "[" << cw1[w].left << "," << cw1[w].right << "] eos:" << cw1[w].eos << " hv:"<< cw1[w].hval << " p:" << cw1[w].p << endl;
        int p = cw1[w].p;
        int q = cw1[w].loc.size();
        int ll, lr, rl;
        int rr = cw1[w].right;
        for (size_t i = 0; i <= q - p; i++)
        {
            if (i == 0) {
                ll = cw1[w].left;
            }
            else {
                ll = cw1[w].loc[i - 1] + 1;
            }
            lr = cw1[w].loc[i];
            rl = cw1[w].loc[i + p - 1];
            IntervalPair* ip = new IntervalPair(ll, lr, rl, rr, ipid, cw1[w].hval);
            iptable.push_back(*ip);
            for (size_t m = ll; m <= lr; m++)
            {
                for (size_t n = rl; n <= rr; n++)
                {
                    ipmap[m][n].push_back(ipid);
                }
            }
            ipid = ipid + 1;
        }
    }
    build2DTree(tree, 0, d1.doc.size(), 0, d1.doc.size(), ipmap);

    
    cout << "Start searching" << endl;
    for (vector<CompactWindow>::size_type w1 = 0; w1 != cw1.size(); w1++)
    {
        //cout << w1 <<":" << endl;
        iptable2.clear();
        int ipid2 = 0;
        for (size_t i = 0; i < maxLen; i++)
        {
            for (size_t j = 0; j < maxLen; j++)
            {
                ipmap2[i][j].clear();
            }
        }

        for (vector<CompactWindow>::size_type w2 = 0; w2 != cw2.size(); w2++)
        {
            if (cw1[w1].hval == cw2[w2].hval && cw1[w1].hashid == cw2[w2].hashid) {
                int p = cw2[w2].p;
                int q = cw2[w2].loc.size();
                int ll, lr, rl;
                int rr = cw2[w2].right;
                for (size_t i = 0; i <= q - p; i++)
                {
                    if (i == 0) {
                        ll = cw2[w2].left;
                    }
                    else {
                        ll = cw2[w2].loc[i - 1] + 1;
                    }
                    lr = cw2[w2].loc[i];
                    rl = cw2[w2].loc[i + p - 1];
                    IntervalPair* ip = new IntervalPair(ll, lr, rl, rr, ipid2, cw2[w2].hval);
                    iptable2.push_back(*ip);
                    for (size_t m = ll; m <= lr; m++)
                    {
                        for (size_t n = rl; n <= rr; n++)
                        {
                            ipmap2[m][n].push_back(ipid2);
                        }
                    }
                    ipid2 = ipid2 + 1;
                }
            }
        }
        
        SegmentTree2DNode* tree2 = new SegmentTree2DNode();
        build2DTree(tree2, 0, d2.doc.size(), 0, d2.doc.size(), ipmap2);
        for (vector<IntervalPair>::size_type w = 0; w != iptable2.size(); w++) {
            int queryResult = query2DTree(tree2, iptable2[w].ll, iptable2[w].lr, iptable2[w].rl, iptable2[w].rr);
            if (queryResult > ssh) {
                cout << "[" << iptable[w1].ll << "," << iptable[w1].lr << "][" << iptable[w1].rl << "," << iptable[w1].rr << "] and ";
                cout << "[" << iptable2[w].ll << "," << iptable2[w].lr << "][" << iptable2[w].rl << "," << iptable2[w].rr << "]->";
                cout << queryResult << endl;
            }
        }
        clean2DTree(tree2);
    }

}


int main()
{   
    //主程序，命令式交互
    string command;
    vector<Document> dataDocuments;
    ofstream logifs;
    logifs.open("log.txt", ios::out);
    cout << "Welcome\n";
    while (true)    
    {
        cin >> command;
        if (command == "load")
        {   //从数据集读入documents
            ifstream ifs;
            
            ifs.open("test.txt", ios::in);//reut2-000.sgm //test是两篇文章；reut2-000.sgm是下载的数据集
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
                logifs << "※Reading file complete." << endl;
            }
        }
        else if (command == "test")
        {   //测试，比较document 1和2的相似度
            calSimilarity(dataDocuments[0], dataDocuments[1], 100, 0.3);

        }
        else if (command == "tree")
        {
            //仅对2D片段树进行测试
            static int a[maxLen][maxLen];
            for (size_t i = 0; i < maxLen; i++)
            {
                for (size_t j = 0; j < maxLen; j++)
                {
                    a[i][j] = i+j;
                }
            }
            SegmentTree2DNode* t = new SegmentTree2DNode();
            //build2DTree(t, 0, 800, 0, 800, a);
            //cout << query2DTree(t, 3, 637, 101, 784);
        }
        else if (command == "cc") {
            vector<int> vi = { 1,2,3,4,3,5,3,1,2,2,3,5,5,3,1 };
            whv[1].push_back(89);
            whv[1].push_back(83);
            whv[1].push_back(98);
            whv[2].push_back(69);
            whv[2].push_back(68);
            whv[2].push_back(66);
            whv[3].push_back(43);
            whv[3].push_back(93);
            whv[3].push_back(3);
            whv[3].push_back(57);
            whv[3].push_back(33);
            whv[4].push_back(23);
            whv[5].push_back(71);
            whv[5].push_back(72);
            whv[5].push_back(73);
            vector<CompactWindow> cw1;
            conquer(vi, 0, 14, 14, whv, 1, cw1);
            for (vector<CompactWindow>::size_type w = 0; w != cw1.size(); w++) {
                cout << "[" << cw1[w].left << "," << cw1[w].right << "] eos:" << cw1[w].eos << " hv:" << cw1[w].hval << " p:" << cw1[w].p << endl;
            }
        }
        else if (command == "rand")
        {
            srand(0);
            int a = 0;
            while (a == 0)
                a = rand();
            int b = rand();
            cout << a << endl;
            cout << b << endl;
        }
        else if (command == "quit")
        {
        logifs.close();
        break;
        }
        else {
            cout << "wrong command" << endl;
        }
    }

}

  