#include<fstream>
#include<cctype>
#include<bits/stdc++.h>
#define BUF_SIZE 1 << 17
using namespace std;

char buf[BUF_SIZE];
int pos = BUF_SIZE;
FILE* fin = fopen("abce.in", "r");
FILE* fout = fopen("abce.out", "w");

inline char getChar(FILE* fin) {
    if(pos == BUF_SIZE) {
        fread(buf, 1, BUF_SIZE, fin);
        pos = 0;
    }
    return buf[pos++];
}

inline int read(FILE* fin) {
    int res = 0;
    int s = 1;
    char ch;
    do {
        ch = getChar(fin);
        if(ch == '-')
            s = -s;
    }while(!isdigit(ch));
    do {
        res = 10*res + ch - '0';
        ch = getChar(fin);
    }while(isdigit(ch));
    return res * s;
}

class treeNode {
private:
    int nrOfKeys; // numarul curent de chei memorate in nod
    int *keys; // cheile memorate in ordine crescatoare
    bool isLeaf;
    int t; // fiecare nod, cu exceptia radacinii va avea minim t - 1 chei si maxim 2t - 1
    // t va fi gradul minim al B-arborelui
    treeNode **c; // daca x este nod intern, el va avea nrOfKeys(x) + 1 fii
public:
    treeNode(int, bool);
    void traverse(int, int);
    treeNode* Search(int); // cauta valoarea k in B-arbore
    int findKey(int); // returneaza indicele primei chei mai mari sau egale cu k
    void insertNonFull(int);
    void splitChild(int, treeNode* y);
    void Remove(int); // functia de stergere a unei chei
    void removeFromLeaf(int);
    void removeFromNonLeaf(int);
    int getPredecessor(int); // functia determina predecesorul unei chei, cheia fiind pe pozitia pos in nod
    int getSuccessor(int); // functia determina succesorul unei chei, cheia fiind pe pozitia pos in nod
    void Fill(int);
    void borrowFromPrev(int);
    void borrowFromNext(int);
    void Merge(int);
    int findPredecessor(int);
    int findSuccessor(int);
    bool findValue(int);
    friend class BTree;
};

class BTree {
    // Stocam radacina arborelui in aceasta clasa
    // Pentru fiecare B-arbore trebuie sa retinem si gradul minim (t)
private:
    treeNode* root;
    int t;
public:
    BTree(int _t);
    void traverse(int, int);
    treeNode* Search(int);
    void Insert(int); // functia de inserare a cheii cu valoarea k
    void Remove(int); // functia de stergere a cheii cu valoarea k
    bool Find(int);
    int BTreeGetPredecessor(int);
    int BTreeGetSuccessor(int);
};

treeNode::treeNode(int _t, bool _isLeaf) {
    t = _t;
    isLeaf = _isLeaf;
    nrOfKeys = 0; // numarul initial de chei dintr-un nod va fi 0
    keys = new int[2 * t - 1]; // alocam dinamic memorie pentru vectorul de chei
    c = new treeNode* [2 * t]; // alocam dinamic memorie pentru vectorul de fii
    for(int i = 0; i < 2 * t; ++i)
        c[i] = nullptr;
}

int treeNode::findKey(int val) {
    // Functia returneaza indicele cheii dintr-un nod care este mai mare sau egala decat val
    int i = 0;
    while(i < nrOfKeys && keys[i] < val)
        ++i;
    return i;
}

bool treeNode::findValue(int val) {
    int i = 0;
    while(i < nrOfKeys && val > keys[i])
        i++;
    if(i < nrOfKeys && val == keys[i])
        return 1;
    if(isLeaf)
        return 0;
    return c[i]->findValue(val);
}

void treeNode::Remove(int val) {
    //Functia sterge cheia val din arborele avand drept radacina nodul curent
    int pos = findKey(val);
    // Verificam daca valoarea pe care dorim s-o stergem se afla in B-arbore
    if(pos < nrOfKeys && keys[pos] == val) { // cheia se afla in B-arbore
        if(isLeaf) // Daca nodul curent este frunza, apelam functia de stergere corespunzatoare
            removeFromLeaf(pos);
        else removeFromNonLeaf(pos);
    } else { // Cheia nu se gaseste in nodul curent
        if(isLeaf) // Daca am ajuns intr-o frunza, cheia nu exista in B-arbore
            return;

        bool ok; // ok are valoarea true daca valoarea cautata este in subarborele avand ca radacina ultimul fiu al nodului curent
        if(pos == nrOfKeys)
            ok = true;
        else ok = false;
        if(c[pos]->nrOfKeys < t) // nodul contine mai putin de t chei
            Fill(pos);
        if(ok && pos > nrOfKeys)
            c[pos - 1]->Remove(val);
        else c[pos]->Remove(val);

    }
    return;
}

void treeNode::removeFromLeaf(int pos) {
    // Functia sterge cheia aflata pe pozitia pos in nodul curent ( de tip frunza )
    for(int i = pos + 1; i < nrOfKeys; i++)
        keys[i - 1] = keys[i]; // deplasam toate elementele de la pozitia pos + 1 cu o pozitie la stanga
    --nrOfKeys; // numarul de chei va scadea cu 1
    return;
}

void treeNode::removeFromNonLeaf(int pos) {
    // Functia sterge cheia aflata pe pozitia pos in nodul curent, care nu este frunza, ci este nod intern
    int val = keys[pos];
    // Daca fiul care precede valoarea keys[pos] are cel putin t chei, gasim predecesorul lui keys[pos] in subarborele cu radacina c[pos]
    // Inlocuim keys[poz] cu valoarea predecesorului si stergem recursiv predecesorul
    if(c[pos]->nrOfKeys >= t) {
        int predecessor = getPredecessor(pos);
        keys[pos] = predecessor;
        c[pos]->Remove(predecessor);
    }

    // Daca fiul c[pos] are mai putin de t chei, verificam nodul c[pos + 1]
    // Daca nodul c[pos + 1] are cel putin t chei, gasim succesorul lui keys[poz] in subarborele avand drept radacina nodul c[pos + 1]
    // Inlocuim keys[pos] cu valoarea succesorului sau si stergem recursiv succesorul din subarborele cu radacina in nodul c[pos + 1]
    else if(c[pos + 1]->nrOfKeys >= t) {
        int successor = getSuccessor(pos);
        keys[pos] = successor;
        c[pos + 1]->Remove(successor);
    }

    else {
        // Daca atat nodul c[pos], cat si nodul c[pos + 1] au mai putin de t chei, unim k si toate cheile din c[pos + 1] in c[pos]
        // c[pos] va contine 2 * t - 1 chei, deci va fi un nod plin.
        // Putem sterge acum nodul c[pos + 1] si, recursiv, cheia k din nodul c[pos]
        Merge(pos);
        c[pos]->Remove(val);
    }
    return;
}

int treeNode::getPredecessor(int pos) {
    // Functia determina predecesorul cheii keys[pos]
    treeNode *cur = c[pos];
    // Ne vom deplasa catre cel mai din dreapta nod pana cand ajungem la o frunza
    while(!cur->isLeaf)
        cur = cur->c[cur->nrOfKeys];
    // Returnam ultima cheie din frunza curenta
    return cur->keys[cur->nrOfKeys-1];
}

int treeNode::getSuccessor(int pos) {
    treeNode* current = c[pos + 1];
    // Pornim din nodul c[pos + 1] si ne deplasam spre stanga pana cand ajungem la o frunza
    while(!current->isLeaf)
        current = current->c[0];
    // Returnam prima cheie din frunza
    return current->keys[0];
}

void treeNode::Fill(int pos) {
    // Daca nodul curent are mai putin de t - 1 chei, va trebui sa-l completam
    // Vom imprumuta o cheie de la un fiu
    if(pos != 0 && c[pos - 1]->nrOfKeys >= t)
        borrowFromPrev(pos);

    else if(pos != nrOfKeys && c[pos + 1]->nrOfKeys >= t)
        borrowFromNext(pos);

    else {
        // Unim c[pos] cu fratele sau
        if(pos != nrOfKeys)
            Merge(pos);
        else Merge(pos - 1);
    }
    return;
}


void treeNode::borrowFromPrev(int pos) {
    // Functia imprumuta o cheie din c[pos - 1] si o introduce in c[pos]
    treeNode* child = c[pos];
    treeNode* sibling = c[pos - 1];
    // Ultima cheie de la c[pos - 1] se va duce la parinte
    // Cheia keys[pos - 1] de la parinte este introdusa ca prima cheie in c[pos]
    for(int i = child->nrOfKeys-1; i >= 0; --i)
        child->keys[i + 1] = child->keys[i];

    if(!child->isLeaf) {
        for(int i = child->nrOfKeys; i >= 0; --i)
            child->c[i + 1] = child->c[i];
    }
    // Prima cheie a fiului va fi keys[pos - 1]
    child->keys[0] = keys[pos - 1];
    if(!child->isLeaf)
        child->c[0] = sibling->c[sibling->nrOfKeys];
    keys[pos - 1] = sibling->keys[sibling->nrOfKeys-1];
    child->nrOfKeys += 1;
    sibling->nrOfKeys -= 1;
    return;
}

void treeNode::borrowFromNext(int pos) {
    treeNode* child = c[pos];
    treeNode* sibling = c[pos + 1];
    // Inseram keys[pos] ca ultima cheie in nodul c[pos]
    child->keys[(child->nrOfKeys)] = keys[pos];
    if(!(child->isLeaf))
        child->c[(child->nrOfKeys) + 1] = sibling->c[0];
    keys[pos] = sibling->keys[0]; // Inseram prima cheie din nodul frate in keys[pos]
    for(int i = 1; i < sibling->nrOfKeys; ++i)
        sibling->keys[i - 1] = sibling->keys[i]; // deplasam toate cheile din sibling cu o pozitie la stanga
    if(!sibling->isLeaf) {
        for(int i = 1; i <= sibling->nrOfKeys; ++i)
            sibling->c[i - 1] = sibling->c[i];
    }
    child->nrOfKeys += 1;
    sibling->nrOfKeys -= 1;
    return;
}

void treeNode::Merge(int pos) {
    // Functia imbina c[pos] cu c[pos + 1], iar la final sterge nodul c[pos + 1]
    treeNode* child = c[pos];
    treeNode* sibling = c[pos + 1];
    child->keys[t - 1] = keys[pos];
    for(int i = 0; i < sibling->nrOfKeys; ++i)
        child->keys[i + t] = sibling->keys[i];
    if(!child->isLeaf) {
        for(int i = 0; i <= sibling->nrOfKeys; ++i)
            child->c[i + t] = sibling->c[i];
    }

    for(int i = pos+1; i < nrOfKeys; ++i)
        keys[i - 1] = keys[i];

    for(int i = pos+2; i <= nrOfKeys; ++i)
        c[i - 1] = c[i];

    child->nrOfKeys += sibling->nrOfKeys+1;
    nrOfKeys--;
    delete(sibling);
    return;
}

void treeNode::insertNonFull(int val) {
    // Functia insereaza o noua cheie in nodul curent cu precizarea ca in acesta sunt suficiente locuri disponibile pentru a realiza operatia
    int i = nrOfKeys - 1;
    if(isLeaf) {
        while(i >= 0 && keys[i] > val) { // Deplasam toate cheile mai mari decat val cu o pozitie in fata
            keys[i + 1] = keys[i];
            i--;
        }
        keys[i + 1] = val;
        nrOfKeys = nrOfKeys + 1;
    } else { // nodul este intern
        while(i >= 0 && keys[i] > val)
            i--;
        if(c[i + 1]->nrOfKeys == 2*t-1) {
            splitChild(i+1, c[i + 1]);
            if(keys[i + 1] < val)
                i++;
        }
        c[i + 1]->insertNonFull(val);
    }
}

void treeNode::splitChild(int i, treeNode* y) {
    // Functia de scindare a nodului y care trebuie sa fie plin pentru realizarea operatiei
    treeNode* z = new treeNode(y->t, y->isLeaf);
    z->nrOfKeys = t - 1;
    for(int j = 0; j < t-1; j++)
        z->keys[j] = y->keys[j + t];
    if(y->isLeaf == false) {
        for(int j = 0; j < t; j++)
            z->c[j] = y->c[j + t];
    }
    y->nrOfKeys = t - 1; // Reducem numarul de chei din nodul y
    for(int j = nrOfKeys; j >= i+1; j--)
        c[j + 1] = c[j];
    c[i + 1] = z;
    for(int j = nrOfKeys-1; j >= i; j--)
        keys[j + 1] = keys[j];

    keys[i] = y->keys[t - 1];
    ++nrOfKeys;
}

void treeNode::traverse(int x, int y) {
    // Functia de parcurgere a arborelui
    for(int i = 0; i < nrOfKeys; ++i) {
        if(!isLeaf)
            c[i]->traverse(x, y);
        if(x <= keys[i] && keys[i] <= y)
            fprintf(fout,"%d ", keys[i]);
    }
    if(!isLeaf)
        c[nrOfKeys]->traverse(x, y);
}

treeNode* treeNode::Search(int val) {
    // Functia de cautare a cheii val
    int i = 0;
    while(i < nrOfKeys && val > keys[i]) // Gasim cea mai mare cheie mai mare sau egala cu val
        ++i;

    if(i < nrOfKeys && keys[i] == val) // Cheia se afla in B-arbore
        return this;

    if(isLeaf) // Am ajuns intr-o frunza, deci cheia nu exista in B-arbore
        return nullptr;
    return c[i]->Search(val); // Continuam recursiv cautarea mergand pe fiul corespunzator
}

int treeNode::findPredecessor(int val) {
    int i, x = INT_MIN;
    for(i = 0; i < nrOfKeys; i++) {
        if(keys[i] > val)
            break;
        if(x <= keys[i])
            x = keys[i];
    }
    if(!isLeaf)
        x = max(x, c[i]->findPredecessor(val));
    return x;
}

int treeNode::findSuccessor(int val) {
    int i,x = INT_MAX;
    for(i = nrOfKeys-1; i>= 0; --i) {
        if(keys[i] < val)
            break;
        if(x >= keys[i])
            x = keys[i];
    }
    if(!isLeaf)
        x = min(x,c[i+1]->findSuccessor(val));
    return x;
}

BTree::BTree(int _t) {
    t = _t;
    root = nullptr;
}

void BTree::traverse(int x, int y) {
    root->traverse(x, y); // apelam functia din clasa treeNode
}

treeNode* BTree::Search(int k) {
    return root->Search(k);
}

void BTree::Insert(int val) {
    // Functia insereaza cheia cu valoarea val in B-arbore
    if(root == nullptr) { // daca arborele este vid
        root = new treeNode(t, 1); // alocam memorie pentru radacina
        root->keys[0] = val; // inseram cheia
        root->nrOfKeys = 1; // actualizam numarul de chei din nodul curent
    } else
        if(root->nrOfKeys == 2*t-1) { // nodul este plin
            treeNode* s = new treeNode(t, false);
            s->c[0] = root;
            s->splitChild(0, root);
            int i = 0;
            if(s->keys[0] < val)
                i++;
            s->c[i]->insertNonFull(val);
            root = s;
        } else root->insertNonFull(val); // nodul nu este plin, deci apelam functia insertNonFull
}

void BTree::Remove(int val) {
    // Functia de stergere a cheii val
    if(!root->nrOfKeys) {
        //cout << "The tree is empty!\n";
        return;
    }
    root->Remove(val);
    if(root->nrOfKeys == 0) {
        treeNode* tmp = root;
        if(root->isLeaf)
            root = nullptr;
        else root = root->c[0];
        delete tmp;
    }
    return;
}

int BTree::BTreeGetPredecessor(int val) {
    return root->findPredecessor(val);
}

int BTree::BTreeGetSuccessor(int val) {
    return root->findSuccessor(val);
}

bool BTree::Find(int val) {
    if(root == nullptr)
        return 0;
    return root->findValue(val);
}

int main() {
    BTree t(4);
    int op, Q, x, y;
    Q = read(fin);
    for(int i = 0; i < Q; ++i) {
        op = read(fin);
        x = read(fin);
        if(op == 1)
            t.Insert(x);
        else if(op == 2)
            t.Remove(x);
        else if(op == 3)
            fprintf(fout,"%d\n", t.Find(x));
        else if(op == 4)
            fprintf(fout,"%d\n", t.BTreeGetPredecessor(x));
        else if(op == 5)
            fprintf(fout,"%d\n", t.BTreeGetSuccessor(x));
        else {
            y = read(fin);
            t.traverse(x, y);
            fprintf(fout, "\n");
        }
    }
    fclose(fin);
    fclose(fout);
    return 0;
}
