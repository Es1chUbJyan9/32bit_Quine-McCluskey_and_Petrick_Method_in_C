//Version 5.03
//更新：修復多處 Bug，測試穩定版
//Created by Es1chUb.Jyan9@gmail.com
//todo:
//1.效能:MultiplyOut 表首 Node 加入排序
//2.效能:清除內存泄漏
//3.效能:新增:支援多輸出函數化簡

/******************************包含文件******************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>


/******************************可調參數******************************/
#define DELIMITER ','
#define PARSE_DELIMETERS " ,"
#define EXPRESSION_MAX_LENGTH 1001
#define VARIABLES_MAX 32


/******************************資料結構******************************/
//用來儲存 Implicants 中 Minterm id 的串列節點
struct lnode{
	unsigned int id;
	struct lnode *next;
	struct lnode *prev;
};
typedef struct lnode lnodeT;

//用來儲存 Implicants 實體，其二進位字串、1的個數、包含的 minterm 的 id 串列
struct mintermGroup{
	char *repr;
    unsigned int cPosBits;
	lnodeT *root;
};
typedef struct mintermGroup mintermGroupT;


/******************************函數原型******************************/
//輸入類
void ReadInput(char *exp_minterms, char *exp_dontCare);
void CountTerms(char *exp_minterms, unsigned int *cMinterms);
void ParseInput(char *exp_minterms, mintermGroupT *mintermArray,unsigned int cMinterms,unsigned int *cVariables);
void ParseCareInput(char *exp_minterms, mintermGroupT *mintermArray,unsigned int cMinterms,unsigned int *cVariables);
//補助類
int CompareById(const void *a, const void *b);
int CompareByRepr(const void *a , const void *b);
bool CanFormGroup(mintermGroupT firstGroup, mintermGroupT secondGroup,unsigned int cVariables);
void CreateNewRepr(char *newGroupRepr, char *firstGroupRepr, char *secondGroupRepr, int cVariables);
//串列類
void list_init(lnodeT **root, unsigned int id);
void list_insert(lnodeT *root, unsigned int id);
void list_merge(lnodeT **newRoot, lnodeT *firstRoot, lnodeT *secondRoot );
bool list_equal(lnodeT *firstRoot, lnodeT *secondRoot);
void list_print(lnodeT *root);
//歸納類
void GetPrimeImplicants( mintermGroupT **table, bool **termsUsed, mintermGroupT *primeImplicantsArray,unsigned int *lenCol, int cColumns);
void CreatePrimeChart(bool **primeChart, mintermGroupT *minterms, int cMinterms,mintermGroupT *primeImplicantsArray, int cPrimeImplicants);
void GetEssentialImplicants(bool **primeChart, int cPrimeImplicants, int cMinterms, bool *isEssential );


/******************************函數實現******************************/
//讀取 minterm 輸入，儲存為字符串
void ReadInput(char *exp_minterms, char *exp_dontCare){
    printf("\n32bit Quine-McCluskey and Petrick's Method in C v5.03");
    printf("\nCreated by Es1chUb.Jyan9@gmail.com");
    printf("\n\n#########################################################################\n");
    printf("################   Quine-McCluskey Algorithm is Started  ################\n");
    printf("#########################################################################\n\n");
    
	//讀取輸入
	printf("Please enter the minterms(Divided by ,):\n\t>> ");
    scanf("%s", exp_minterms);
    
    printf("\nPlease enter the don't care(Divided by ,):\n!!! If you don't need don't care, Please enter -1 !!!\n\t>> ");
    scanf("%s", exp_dontCare);
}

//透過分隔符來計算有幾個 minterm
void CountTerms(char *exp_minterms, unsigned int *cMinterms){
	char *ptr = strchr(exp_minterms,DELIMITER);
    for (*cMinterms = 1; ptr != NULL; (*cMinterms)++)
        ptr = strchr(ptr + 1, DELIMITER);
}

//初始化 Implicants id 鏈表
void list_init(lnodeT **root, unsigned int id){
	lnodeT *newNode = (lnodeT *)malloc( sizeof(lnodeT) );
	newNode->id = id;
	newNode->next = newNode->prev = NULL;
	
	*root = newNode;
}

//分析輸入
void ParseInput(char *exp_minterms, mintermGroupT *mintermArray,unsigned int cMinterms,unsigned int *cVariables){
	unsigned int i,j,maxMinterm = 0;
	
    //以逗點為分隔，讀取一個 minterm 字串
	char *pch = strtok(exp_minterms, PARSE_DELIMETERS);
	
	//將 minterm 存入 mintermArray
	for(i = 0; i < cMinterms; i++){
        mintermArray[i].repr = (char *)malloc( (VARIABLES_MAX + 1) * sizeof(char));
        mintermArray[i].cPosBits = 0;
		mintermArray[i].root = NULL;
        
        //存入 id
		unsigned int id = atoi(pch);
		list_init(&mintermArray[i].root, id);
        
        //檢查是否為最大 minterm
		maxMinterm = (maxMinterm > id) ? maxMinterm : id;
		
		//將 minterm 轉換為二進位字串，並計算1的個數，注意是反的
		for(j = 0; j < VARIABLES_MAX; j++,id /= 2){
            if(id % 2 == 1){
                mintermArray[i].repr[j] = '1';
                mintermArray[i].cPosBits++;
            }
            else
                mintermArray[i].repr[j] = '0';
		}
		mintermArray[i].repr[VARIABLES_MAX] = '\0';
        
		//讀取下一個minterm
		if(i + 1 < cMinterms)
			pch = strtok(NULL, PARSE_DELIMETERS);
	}
	
	//全部輸入完畢後依最大 minterm 求出變數數
    (*cVariables) = 0;
	for (int binary = 1; maxMinterm > (binary - 1) ; (*cVariables)++)
        binary *= 2;
	
	//依求得的變數數縮小二進位字串，以節省空間，並反向
	for(i = 0; i < cMinterms; i++){
		mintermArray[i].repr = (char *)realloc( mintermArray[i].repr , (*cVariables + 1) * sizeof(char) );
		mintermArray[i].repr[ *cVariables ] = '\0';
        
        //反向二進位字串
        for(unsigned long int low = 0,high = *cVariables - 1; low < high; low++,high--) {
            char c = mintermArray[i].repr[low];
            mintermArray[i].repr[low] = mintermArray[i].repr[high];
            mintermArray[i].repr[high] = c;
        }
	}
}

//分析輸入
void ParseCareInput(char *exp_minterms, mintermGroupT *mintermArray,unsigned int cMinterms,unsigned int *cVariables){
	unsigned int i,j,maxMinterm = 0;
	
    //以逗點為分隔，讀取一個 minterm 字串
	char *pch = strtok(exp_minterms, PARSE_DELIMETERS);
	
	//將 minterm 存入 mintermArray
	for(i = 0; i < cMinterms; i++){
        mintermArray[i].repr = (char *)malloc( (VARIABLES_MAX + 1) * sizeof(char));
        mintermArray[i].cPosBits = 0;
		mintermArray[i].root = NULL;
        
        //存入 id
		unsigned int id = atoi(pch);
		list_init(&mintermArray[i].root, id);
        
        //檢查是否為最大 minterm
		maxMinterm = (maxMinterm > id) ? maxMinterm : id;
		
		//將 minterm 轉換為二進位字串，並計算1的個數，注意是反的
		for(j = 0; j < VARIABLES_MAX; j++,id /= 2){
            if(id % 2 == 1){
                mintermArray[i].repr[j] = '1';
                mintermArray[i].cPosBits++;
            }
            else
                mintermArray[i].repr[j] = '0';
		}
		mintermArray[i].repr[VARIABLES_MAX] = '\0';
        
		//讀取下一個minterm
		if(i + 1 < cMinterms)
			pch = strtok(NULL, PARSE_DELIMETERS);
	}
	
	//依求得的變數數縮小二進位字串，以節省空間，並反向
	for(i = 0; i < cMinterms; i++){
		mintermArray[i].repr = (char *)realloc( mintermArray[i].repr , (*cVariables + 1) * sizeof(char) );
		mintermArray[i].repr[ *cVariables ] = '\0';
        
        //反向二進位字串
        for(unsigned long int low = 0,high = *cVariables - 1; low < high; low++,high--) {
            char c = mintermArray[i].repr[low];
            mintermArray[i].repr[low] = mintermArray[i].repr[high];
            mintermArray[i].repr[high] = c;
        }
	}
}

//用於 qsort 的依 id 比較
int CompareById(const void *a, const void *b){
	mintermGroupT *first  = (mintermGroupT *)a;
	mintermGroupT *second = (mintermGroupT *)b;
	
	return ( first->root->id - second->root->id);
}

//用於 qsort 的依二進位比較
int CompareByRepr(const void *a , const void *b){
	mintermGroupT *first  = (mintermGroupT *)a;
	mintermGroupT *second = (mintermGroupT *)b;
	
	if( first->cPosBits == second->cPosBits ){
		return strcmp( first->repr, second->repr );
	}
	
	return (first->cPosBits - second->cPosBits);
}

//檢查兩 Implicants 是否只差一位
bool CanFormGroup(mintermGroupT firstGroup, mintermGroupT secondGroup,unsigned int cVariables){
	int i, bitsDiffer = 0;
	for(i = 0; i < cVariables; i++){
		//如果 '-' 的位置不一樣 
		if((firstGroup.repr[i] == '-' && secondGroup.repr[i] != '-') ||
           (firstGroup.repr[i] != '-' && secondGroup.repr[i] == '-') )
            return 0;
		
		//如果該二進位不同
		if( firstGroup.repr[i] != secondGroup.repr[i] )
			bitsDiffer++;
	}
	//若只有一位不同
	return (bitsDiffer == 1) ? 1 : 0;
}

//插入新結點
void list_insert(lnodeT *root, unsigned int id){
	lnodeT *newElement = (lnodeT *)malloc( sizeof(lnodeT) );
	newElement->id = id;
	newElement->next = newElement->prev = NULL;
    
    //尋找適合的位置
    lnodeT *curr;
    for (curr = root; curr->next != NULL; curr = curr->next)
        if(curr->next->id > newElement->id)
			break;
	
	//調整前後節點的指標
	if(curr->next != NULL){
		curr->next->prev = newElement;
		newElement->next = curr->next;
		newElement->prev = curr;
		curr->next = newElement;
	}
	else{ 
		newElement->prev = curr;
		curr->next = newElement;
	}
}

//串列合併
void list_merge(lnodeT **newRoot, lnodeT *firstRoot, lnodeT *secondRoot){
    //初始化新串列
	list_init(newRoot, firstRoot->id );
	
    //複製第一個串列
    lnodeT *curr;
    for (curr = firstRoot->next; curr != NULL ; curr = curr->next)
        list_insert( *newRoot, curr->id );
    
    //複製第二個串列
    for (curr = secondRoot; curr != NULL ; curr = curr->next)
        list_insert( *newRoot, curr->id );
}

//判斷 id 串列是否相等
bool list_equal(lnodeT *firstRoot, lnodeT *secondRoot){
	lnodeT *fNode = firstRoot;
	lnodeT *sNode = secondRoot;
	while(fNode != NULL){
		if(fNode->id != sNode->id)
			return 0;
		
		fNode = fNode->next;
		sNode = sNode->next;
	}
	return 1;
}

//初始化新 Implicants 的二進位字串
void CreateNewRepr(char *newGroupRepr, char *firstGroupRepr, char *secondGroupRepr, int cVariables){
	for(unsigned int i = 0; i < cVariables; i++){
		if( firstGroupRepr[i] == secondGroupRepr[i] )
			newGroupRepr[i] = firstGroupRepr[i];
		else
			newGroupRepr[i] = '-';
	}
    
	newGroupRepr[cVariables] = '\0';
}

//整理出 Prime Implicants
void GetPrimeImplicants( mintermGroupT **table, bool ** termsUsed, mintermGroupT *primeImplicantsArray, unsigned int *lenCol, int cColumns){
	int i,j,index = 0;
	
	for(i = 0; i <= cColumns; i++)
		for(j = 0; j < lenCol[i]; j++)
			if(termsUsed[i][j] == false){
				primeImplicantsArray[index] = table[i][j];
				index++;
			}
}

//二維 Prime Implicants Cover 表，表示第 j 個 minterm 可被第 i 個 Prime Implicants 覆蓋
void CreatePrimeChart(bool **primeChart, mintermGroupT *minterms, int cMinterms, mintermGroupT * primeImplicantsArray, int cPrimeImplicants){
	unsigned int i,j,mintermId;
	lnodeT *curr;
	
	//遍歷所有的 Prime Implicants
	for(i = 0,j = 0; i < cPrimeImplicants; i++,j=0){
		//遍歷該 Prime Implicants 包含的 id
		for(curr = primeImplicantsArray[i].root;curr != NULL;curr = curr->next){
			//找出該 Prime Implicants 所對應的 mintermArray 位置
			mintermId = minterms[j].root->id;
			while( mintermId < (curr->id) && j < cMinterms-1){
				j++;
				mintermId = minterms[j].root->id;
			}
			//在覆蓋表中標示 j 可被 i 覆蓋
            if (curr->id == mintermId) 
                primeChart[i][j] = true;
		}
	}
}

//一維 Essential Prime Implicant 表
void GetEssentialImplicants(bool ** primeChart, int cPrimeImplicants, int cMinterms, bool * isEssential ){
	int i,j,nextEssential;
	
	//遍歷所有的 minterms
	for(j = 0; j < cMinterms; j++){
		nextEssential = -1;
		//遍歷該 minterm 中所有的質蘊函項，檢查是否只有一項覆蓋
		for(i = 0; i < cPrimeImplicants; i++){
			if( primeChart[i][j] == true ){
				if(nextEssential == -1)
					nextEssential = i;
				else{
					nextEssential = -1;
					break;
				}
			}
		}
		//若只有一個 Prime Implicants 覆蓋，此項為 Essential Prime Implicant
		if(nextEssential != -1)
			isEssential[nextEssential] = true;
	}
}

//Id 鏈表打印
void list_print(lnodeT *root){    
    for (lnodeT *curr = root; curr != NULL; curr = curr->next) 
        printf("%2u ",curr->id);
}
