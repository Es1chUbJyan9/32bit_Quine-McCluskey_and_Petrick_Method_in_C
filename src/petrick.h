//Version 5.03
//更新：修復多處 Bug，測試穩定版
//Created by Es1chUb.Jyan9@gmail.com
//
//todo:
//1.效能:MultiplyOut 表首 Node 加入排序
//2.效能:清除內存泄漏
//3.效能:新增:支援多輸出函數化簡


/******************************資料結構******************************/
struct idnode{
	unsigned int id;
	struct idnode *maxterm;
	struct idnode *prev;
    struct idnode *next;
};
typedef struct idnode idnodeT;

/******************************函數原型******************************/

//List 相關
void CreateMaxtermList(bool **needCovered, idnodeT **maxtermList, int cMintermsCovered, int cPrimeImplicantsCovered);
void nodeInit(idnodeT **root, unsigned int id);
void nodeInsert(idnodeT *root, unsigned int id);
void nodeCopy(idnodeT *root,idnodeT **maxtermList);
void nodeAdd(idnodeT *root, idnodeT *addnode, idnodeT *prevnode);
void nodeRemove(idnodeT *root, idnodeT *prev);

//運算相關
void MultiplyOut(idnodeT **maxtermList, unsigned int lenMul);
void RuleOne(idnodeT **maxtermList);
void RuleTwo(idnodeT **maxtermList);
void LeastCover(idnodeT **maxtermList,idnodeT **shortMaxterm, unsigned int *cShortMaxterm);

/******************************函數實現******************************/

void CreateMaxtermList(bool **needCovered, idnodeT **maxtermList, int cMintermsCovered, int cPrimeImplicantsCovered){
    unsigned int i,j;
    bool flag = false;
    
    for (j=0; j < cMintermsCovered; j++,flag = false) {
        for (i=0; i < cPrimeImplicantsCovered; i++) {
            if (needCovered[i][j] == true) {
                if (flag == false) {
                    nodeInit(&maxtermList[j], i);
                    flag = true;
                }
                else
                    nodeInsert(maxtermList[j], i);
            }
        }
    }
}


void nodeInit(idnodeT **root, unsigned int id){
    idnodeT *newNode = (idnodeT *)malloc(sizeof(idnodeT));
    newNode->id = id;
    newNode->next = newNode->prev = newNode->maxterm = NULL;
    
    *root = newNode;
}

void nodeInsert(idnodeT *root, unsigned int id){
	idnodeT *newNode = (idnodeT *)malloc( sizeof(idnodeT) );
	newNode->id = id;
	newNode->next = newNode->prev = newNode->maxterm = NULL;
    
    //尋找適合的位置
    idnodeT *curr;
    for (curr = root; curr->next != NULL; curr = curr->next)
        if(curr->next->id > newNode->id)
			break;
	
	//調整前後節點的指標
	if(curr->next != NULL){
		curr->next->prev = newNode;
		newNode->next = curr->next;
		newNode->prev = curr;
		curr->next = newNode;
	}
	else{ 
		newNode->prev = curr;
		curr->next = newNode;
	}
}

void nodeCopy(idnodeT *root, idnodeT **maxtermList){
    idnodeT *newNode = (idnodeT *)malloc( sizeof(idnodeT) );
    newNode->id = root->id;
	newNode->next = newNode->prev = newNode->maxterm = NULL;
    
    
    //複製所有 Maxterm
    idnodeT *curr0, *curr1;
    for (curr0 = root, curr1 = newNode; curr0->maxterm != NULL; curr0 = curr0->maxterm){
        idnodeT *idNode = (idnodeT *)malloc( sizeof(idnodeT) );
        idNode->id = curr0->maxterm->id;
        curr1->maxterm = idNode;
        curr1 = curr1->maxterm;
        
    }
	
	//調整前後節點的指標
	if(root->next != NULL){
		root->next->prev = newNode;
		newNode->next = root->next;
		newNode->prev = root;
		root->next = newNode;
	}
	else{ 
		newNode->prev = root;
		root->next = newNode;
	}
}
//在 Maxterm 中插入新的 id
void nodeAdd(idnodeT *root, idnodeT *addnode, idnodeT *prevnode){
    idnodeT *idNode = (idnodeT *)malloc( sizeof(idnodeT) );
    idNode->id = addnode->id;
	idNode->next = idNode->prev = idNode->maxterm = NULL;
    
    //尋找適合的位置
//    if (root->id <= idNode->id){
        idnodeT *curr;
        for (curr = root; curr->maxterm != NULL; curr = curr->maxterm)
            if(curr->maxterm->id > idNode->id)
                break;
	
        //調整前後節點的指標
        if(curr->maxterm != NULL){
            idNode->maxterm = curr->maxterm;
            curr->maxterm = idNode;
        }
        else
            curr->maxterm = idNode;
//    }
//    else{
//        idNode->next = root->next;
//        idNode->prev = root->prev;
//        idNode->maxterm = root;
//        if (root->next != NULL)
//            root->next->prev = idNode;
//        prevnode->next = idNode;
//    }
}

//乘開 Maxterm
void MultiplyOut(idnodeT **maxtermList, unsigned int lenMul){
    idnodeT *curr0, *curr1, *currprev;
    
    //計算有幾個要乘
    unsigned int cNextMaxterm = 0 , cMul;
    for (curr0 = maxtermList[lenMul]; curr0->next != NULL; curr0 = curr0->next)
        cNextMaxterm++;
    
    //先將被乘項複製出足夠的數量
    for (curr0 = maxtermList[0]; curr0 != NULL;curr0 = curr1){
        for (cMul = 0 , curr1 = curr0->next ; cMul < cNextMaxterm; cMul++) {
            nodeCopy(curr0,maxtermList);
        }
    }
    
    //將乘項接上
    for (curr0 = maxtermList[0] , currprev = NULL, curr1 = maxtermList[lenMul];curr0 != NULL; 
         curr0 = curr0->next, curr1 = curr1->next){
        if (curr1 != NULL){
            nodeAdd(curr0, curr1, currprev);
            currprev = curr0;
        }
        else{
            curr1 = maxtermList[lenMul];
            nodeAdd(curr0, curr1, currprev);
            currprev = curr0;
        }
    }
}

void nodeRemove(idnodeT *root, idnodeT *prev){
    prev->maxterm = root->maxterm;
  //  free(root);
  //  root = NULL;
}

//第一化簡規則 XX = X
void RuleOne(idnodeT **maxtermList){
    idnodeT *curr0,*curr1,*curr2,*currTemp;
    
    for (curr0 = maxtermList[0]; curr0 != NULL; curr0 = curr0->next){
        for (curr1 = curr0; curr1 != NULL; curr1 = curr1->maxterm) {
            for (curr2 = curr1->maxterm,currTemp = curr1; curr2 != NULL;){
                if (curr2->id == curr1->id) {
                    nodeRemove(curr2,currTemp);
                    curr2 = currTemp->maxterm;
                }
                else{
                    currTemp = curr2;
                    curr2 = curr2->maxterm;
                }
            }
        }
    }
}

//將找出最短 Maxterm
void LeastCover(idnodeT **maxtermList,idnodeT **shortMaxterm, unsigned int *cShortMaxterm){
    unsigned int i,min = 4294967295;
    idnodeT *curr0 , *curr1;
        for (curr0 = maxtermList[0], i = 0; curr0 != NULL; curr0 = curr0->next, i = 0) {
            for (curr1 = curr0; curr1 != NULL; curr1 = curr1->maxterm) {
                i++;
            }
            if(i < min){
                min = i; 
                *shortMaxterm = curr0;
            }
        }
    *cShortMaxterm = min;
    
    //中斷 shortMaxterm 在 maxtermList 中的連結
    if ((*shortMaxterm)->prev != NULL)
        (*shortMaxterm)->prev->next = (*shortMaxterm)->next;
    else if((*shortMaxterm)->prev == NULL && (*shortMaxterm)->next != NULL)
        maxtermList[0] = (*shortMaxterm)->next;
    else
        maxtermList[0] = NULL;
    if ((*shortMaxterm)->next != NULL)
        (*shortMaxterm)->next->prev = (*shortMaxterm)->prev;
}

//第二化簡規則 X+XY = X
void RuleTwo(idnodeT **maxtermList){
    idnodeT *curr0,*curr1,*curr0temp,*curr1temp;
    bool flag;
    
    for (curr0 = maxtermList[0]; curr0 != NULL; curr0 = curr0->next){
        for (curr1 = curr0->next,flag = true; curr1 != NULL; curr1 = curr1->next, flag = true) {
            for (curr0temp = curr0, curr1temp = curr1; curr0temp != NULL && curr1temp != NULL
                 ; curr0temp = curr0temp->maxterm, curr1temp = curr1temp->maxterm) {
                if (curr0temp->id != curr1temp->id) {
                    flag = false;
                    break;
                }
            }
            //這裡會將先短的覆蓋後長的
            if (flag == true && curr0temp == NULL) {
                curr1->prev->next = curr1->next;
                if(curr1->next != NULL)
                    curr1->next->prev = curr1->prev;
            }
            //這裡會將後短的覆蓋先長的
            else if(flag == true && curr1temp == NULL && curr0->prev != NULL){
                curr0->prev->next = curr0->next;
                if(curr0->next != NULL)
                    curr0->next->prev = curr0->prev;
                curr0 = curr0->next;
                
            }
            //這裡會將後短的覆蓋表頭的
            else if(flag == true && curr1temp == NULL && curr0->prev == NULL){
                maxtermList[0] = curr0->next;
                if(curr0->next != NULL)
                    curr0->next->prev = NULL;
                curr0 = curr0->next;
            }
                
        }
    }
    
}