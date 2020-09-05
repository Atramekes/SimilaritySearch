#include <iostream>
#include <fstream> 
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#define maxLen 1000

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
    cout << "doc:" << endl;
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
    cout << endl;
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
        for (int i = 0; i < cl.length(); i++)
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
    vector<int> c;
    int hval;// hash value
    CompactWindow(int indx1, int eos,int indx2, vector<int> c,int hv)
        : left(indx1), eos(eos), right(indx2), c(c), hval(hv) { }
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


int uni_hash(int val, int a, int b)
{
  return abs(a * val + b);
}


void conquer(vector<int> doc,int indx1, int eos, int indx2, unordered_map<int, vector<int>> whv, vector<CompactWindow>& results) {
    unordered_map<int, int> freq;
    int minval = 0x7fffffff; 
    int p = -1; //第p个min word取min-hash
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
    int q = pos.size();
    cout << indx1 << " " << eos << " " << indx2 << " " << endl;
    results.push_back(CompactWindow(indx1, eos, indx2, pos, minval));
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
            if (pos.at(k) > pos.at(k + p - 1) - 1) { //保证eos不大于indx2
                conquer(doc, left, pos.at(k + p - 1) - 1, pos.at(k + p - 1) - 1, whv, results);
            }
            else {
                conquer(doc, left, pos.at(k), pos.at(k + p - 1) - 1, whv, results);
            }
            
        }
        else
        {
            conquer(doc, left, eos, pos.at(k + p - 1) - 1, whv, results);
            return;
        }
    }
    conquer(doc, pos.at(q - p) + 1, indx2, indx2, whv, results);
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
        }
    }
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


}

void calSimilarity(Document d1, Document d2, int shuffleTimes, vector<WindowPair>* vp, int ssh) {
    //新方法比较相似度
    vector<CompactWindow> results;
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
        conquer(d1.doc, 0, d1.doc.size(), d1.doc.size() - 1, whv, results);
        conquer(d2.doc, 0, d2.doc.size(), d2.doc.size() - 1, whv, results);
    }

   
}

int main()
{   
    //主程序，命令式交互
    string command;
    vector<Document> dataDocuments;
    vector<WindowPair> resultPairs;
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
            vector<Window>* gw1 = new vector<Window>;
            vector<Window>* gw2 = new vector<Window>;
            vector<WindowPair>* crashPairs = new vector<WindowPair>;

            
            int crashTime = 10; //对两document一共进行crashTime次冲突
            for (size_t t = 0; t < crashTime; t++)
            {
                logifs << "※Crash round " << t << endl;
                calSimilarity(dataDocuments[0], dataDocuments[1], t + 1, crashPairs, 3);
            }
            CrashMap* cm = new CrashMap();
            cm->init();
            for (vector<WindowPair>::size_type i = 0; i != crashPairs->size(); i++) {
                cm->addArea(crashPairs->at(i));
            }
            SegmentTree2DNode* t = new SegmentTree2DNode();
            int l = dataDocuments[0].tokens.size();
            if (l < dataDocuments[1].tokens.size()) {
                l = dataDocuments[1].tokens.size();
            }
            build2DTree(t, 0, l, 0, l, cm->map);
            
            
            cout << query2DTree(t, 40, 55, 66, 74);


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
            build2DTree(t, 0, 800, 0, 800, a);
            cout << query2DTree(t, 3, 637, 101, 784);
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
            vector<CompactWindow> res;
            conquer(vi, 0, 14, 14, whv, res);
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

  