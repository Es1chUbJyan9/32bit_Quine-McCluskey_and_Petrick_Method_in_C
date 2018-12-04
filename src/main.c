//Version 5.03
//更新：修復多處 Bug，測試穩定版
//Created by Es1chUb.Jyan9@gmail.com
//todo:
//1.效能:MultiplyOut 表首 Node 加入排序
//2.效能:清除內存泄漏
//3.效能:新增:支援多輸出函數化簡

#include "mccluskey.h"
#include "petrick.h"

int main(void){

	//讀取輸入字符串
    char exp_Care[EXPRESSION_MAX_LENGTH];
    char exp_dontCare[EXPRESSION_MAX_LENGTH];
    ReadInput(exp_Care,exp_dontCare);
    
    //開始計時
    clock_t start, finish;
    start = clock();
    
    //計算 minterm 數
    unsigned int cMintermsCare,cDontCare;
    CountTerms(exp_Care, &cMintermsCare);
    CountTerms(exp_dontCare, &cDontCare);
    
    //合併 Care & Don't care
    char *exp_minterms = (char *)malloc(EXPRESSION_MAX_LENGTH * sizeof(char));
    memcpy(exp_minterms, exp_Care, EXPRESSION_MAX_LENGTH * sizeof(char));
    unsigned cMinterms = cMintermsCare;
    
    if (strstr(exp_dontCare,"-1") == NULL) {
        exp_minterms = strcat(strcat(exp_minterms, ","), exp_dontCare);
        cMinterms += cDontCare;
    }
    
    //將輸入字符串轉入 mintermArray，並分析變數數
    mintermGroupT *mintermArray = (mintermGroupT *)malloc(cMinterms * sizeof(mintermGroupT));
    unsigned int cVariables;
	ParseInput(exp_minterms, mintermArray, cMinterms, &cVariables);
	
	//利用 qsort 使 mintermArray 依 id 排序
	qsort(mintermArray, cMinterms, sizeof(mintermGroupT), CompareById);
	
    //記錄 Table 中直列的長度（即 Implicants 個數）
	unsigned int lenCol[VARIABLES_MAX];
	lenCol[0] = cMinterms;
    
	//將 mintermsArray 存入 table[0] 中當第一列（即一個一圈的），並依 1 的個數排序
    mintermGroupT *table[VARIABLES_MAX + 1];
	table[0] = (mintermGroupT *)malloc( cMinterms * sizeof(mintermGroupT));
	memcpy(table[0], mintermArray, lenCol[0] * sizeof(mintermGroupT));
	qsort(table[0], cMinterms, sizeof(mintermGroupT), CompareByRepr);
	
	//記錄每個 Implicants 的合併情況
    bool *termsUsed[VARIABLES_MAX + 1];
	termsUsed[0] = (bool *)calloc( lenCol[0], sizeof(bool));
	
	//記錄 Prime Implicants 數
	unsigned int cPrimeImplicants = 0;
	
	//構造 Table，以找出所有的 Implicants，每第 i 列以 i+1 個為一圈
	unsigned int i,j,k;
	for(i = 0; i <= cVariables; i++){
		//初始化新的一列的暫存空間與項數
		mintermGroupT *nextCol = (mintermGroupT *)malloc( (lenCol[i] * lenCol[i])  * sizeof(mintermGroupT) );
		int nextColPos = 0;
		
		//開始遍歷檢查該列中的項是否有 Implicants
		for(j = 0,k = 1; j < lenCol[i]; j++,k=j+1){
			//跳過 1 的個數相同的項
			while( k < lenCol[i] && table[i][k].cPosBits == table[i][j].cPosBits )
				k++;
			
			//若兩項的 1 的個數只差 1
			while(k < lenCol[i] && (table[i][k].cPosBits - table[i][j].cPosBits) == 1){
				//且兩項的二進位值只相差一位
				if(CanFormGroup(table[i][k], table[i][j], cVariables)){
					//初始化新的 Implicants
					lnodeT *newGroupRoot;
					list_merge( &newGroupRoot, table[i][j].root, table[i][k].root);
					
					//檢查該 Implicants 是否以經存在於新的一列中
					bool alreadyInColumn = false;
					for(unsigned int c = 0; c < nextColPos; c++){
						if(list_equal(nextCol[c].root , newGroupRoot) != 0 ){
							alreadyInColumn = true;
							break;
						}
					}
					//若新的一列中未存在該項，則將 Implicants 存入新的一列
					if(alreadyInColumn == false){
						nextCol[nextColPos].root = newGroupRoot;
						nextCol[nextColPos].cPosBits = table[i][j].cPosBits;
						nextCol[nextColPos].repr = (char *)malloc( (cVariables + 1) * sizeof(char) );
						CreateNewRepr(nextCol[nextColPos].repr, table[i][j].repr, table[i][k].repr, cVariables);
						
						nextColPos++;
					}
					
					//將此兩項標註為已合併
					termsUsed[i][j] = termsUsed[i][k] = true;
				}
				k++;
			}
		}
		
        //計算這列未使用的 Implicants 即 Prime Implicants
		for(j = 0; j < lenCol[i]; j++)
			if(termsUsed[i][j] == false)
				cPrimeImplicants++;
        
		//記錄新的一列的 Implicants 數
		lenCol[i+1] = nextColPos;
		
		//若已無法進行合併，則結束構造 Table
		if( lenCol[i+1] == 0 )
			break;
		
		//將新的一列轉入 Table 中
		table[i+1] = (mintermGroupT *)malloc( lenCol[i+1] * sizeof(mintermGroupT) );
		for( j = 0; j < lenCol[i+1]; j++)
			table[i+1][j] = nextCol[j];
		
		//初始化下一列的合併檢查表，並釋放暫存空間
		termsUsed[i+1] = (bool *)calloc( lenCol[i+1], sizeof(bool));
		free(nextCol);
	}
	//至此 table 構造完畢，已找出所有的 Implicants
	
	//確定 table 的列數
	int cColumns = i;
	
	//整理出 Prime Implicants
	mintermGroupT *primeImplicantsArray = (mintermGroupT *)malloc( cPrimeImplicants * sizeof(mintermGroupT));
	GetPrimeImplicants(table, termsUsed, primeImplicantsArray, lenCol, cColumns);
	
	//二維 Prime Implicants Cover 表，表示第 j 個 minterm 可被第 i 個 Prime Implicants 覆蓋
	bool **primeChart = (bool **)malloc( cPrimeImplicants * sizeof(bool *) );
	for(i = 0; i < cPrimeImplicants; i++)
		primeChart[i] = (bool *)calloc( cMintermsCare, sizeof(bool));
    
    //構造無 Don't care 的 MintermArray
    mintermGroupT *mintermCareArray = (mintermGroupT *)malloc(cMintermsCare * sizeof(mintermGroupT));
    unsigned int cVariablesCare;
	ParseCareInput(exp_Care, mintermCareArray, cMintermsCare, &cVariablesCare);
    //利用 qsort 使 mintermCareArray 依 id 排序
	qsort(mintermCareArray, cMintermsCare, sizeof(mintermGroupT), CompareById);
    
	CreatePrimeChart(primeChart, mintermCareArray, cMintermsCare, primeImplicantsArray, cPrimeImplicants);
	
	//製作 Essential Prime Implicant 表
	bool *isEssential = (bool *)calloc( cPrimeImplicants, sizeof(bool) );
	GetEssentialImplicants(primeChart, cPrimeImplicants, cMintermsCare, isEssential);
	
	//製作 Essential Prime Implicant - Minterms 覆蓋表
	bool *mintermsCovered = (bool *)calloc( cMintermsCare, sizeof(bool) );
	for(i = 0; i < cPrimeImplicants; i++)
		if(isEssential[i] == true)
			for(j = 0; j < cMintermsCare; j++)
				if(primeChart[i][j] == true)
					mintermsCovered[j] = true;
	
    
	//******************************** Q-M 開始輸出********************************
	
    
	//打印 Table 
	for(i = 0; i <= cColumns; i++){
		printf("\n\n###############                  Size %d                   ###############\n\n",(int)pow(2,i));
		for(j = 0; j < lenCol[i]; j++){
			printf("[ %s ] %s [ ", (termsUsed[i][j] ? "OK" : " X"), table[i][j].repr);
            list_print( table[i][j].root );
            printf("]\n");
		}
	}
	
	//打印 Prime Implicant
	printf("\n\n###############              Prime Implicant              ###############\n\n");
	for(i = 0; i < cPrimeImplicants; i++){
		printf("[P%2d]: %s  [ ", i, primeImplicantsArray[i].repr);
		list_print( primeImplicantsArray[i].root );
		printf("]\n");
	}
	
	//打印 Prime Implicants Cover
	printf("\n\n###############          Prime Implicants Cover           ###############\n\n");
	printf("        ");
    
	for(j = 0; j < cMintermsCare; j++)
		printf("| %2u", mintermCareArray[j].root->id);
	printf("|\n---------");
    
	for(j = 0; j < cMintermsCare; j++)
		printf("----");
	printf("\n");
	
	for(i = 0; i < cPrimeImplicants; i++){
		printf("[P%2d]:  ", i);
		for(j = 0; j < cMintermsCare; j++)
			printf("|  %c", ( primeChart[i][j] == true) ? 'X' : ' ');
		printf("|\n");
	}
	
	//打印 Essential Prime Implicant
	printf("\n\n###############         Essential Prime Implicant         ###############\n\n");
	for(i = 0; i < cPrimeImplicants; i++){
		if(isEssential[i] == true){
			printf("[P%2d]: %s  [ ", i, primeImplicantsArray[i].repr);
			list_print(primeImplicantsArray[i].root);
			printf(" ]\n");
		}
	}
	
	//打印 Essential Prime Implicant 的覆蓋情況
	printf("\n\n###############      Essential Prime Implicant Cover      ###############\n\n");
	for(j = 0; j < cMintermsCare; j++)
		printf("| %2u", mintermCareArray[j].root->id );
	printf("|\n");
    
	for(j = 0; j < cMintermsCare; j++)
		printf("----");
	printf("-\n");
    
	for(i = 0; i < cMintermsCare; i++)
		printf("|  %c", (mintermsCovered[i] == true) ? 'X' : ' ');
	printf("|\n");
    
    //打印 Essential Prime Implicant 的 Function
    char ch[32] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f'};
    unsigned int cEssential = 0;
    for (i=0; i < cPrimeImplicants ; i++)
        if (isEssential[i] == true)
            cEssential++;
    
    //檢查 minterm 是否已全部覆蓋
    bool beOver = true;
    for(i = 0; i < cMintermsCare; i++)
		if(mintermsCovered[i] == false){
            beOver = false;
            break;
        }
    
    if (beOver == true) {
    
    printf("\n\n###############            The Final Function             ###############\n\n");
    printf("F(");
    for (i = 0; i < cVariables; i++)
        printf("%c%s",ch[i],( i+1 < cVariables) ? "," : ") = ");
        
    bool isOne = true;
    
    for(unsigned int flag = cEssential, i = 0; i < cPrimeImplicants; i++){
		if(isEssential[i] == true){
            for (j=0; primeImplicantsArray[i].repr[j] != '\0'; j++) {
                if(primeImplicantsArray[i].repr[j] == '1'){
                    printf("%c",ch[j]);
                    isOne = false;
                }
                else if(primeImplicantsArray[i].repr[j] == '0'){
                    printf("%c\'",ch[j]);
                    isOne = false;
                }
            }
            if(--flag)
                printf(" + ");
		}
	}
        
    if (isOne == true)
        printf("1");
        
    }
    printf("\n\n#########################################################################\n");
    printf("###############     Quine-McCluskey Algorithm is Over     ###############\n");
    printf("#########################################################################\n");
    
    //******************************** Q-M 輸出結束********************************
    //Quine-McCluskey Algorithm 至此結束

    if(beOver == false){
        
    printf("\n#########################################################################\n");
    printf("###############        Petrick's Method is Started        ###############\n");
    printf("#########################################################################\n");
        
    //製作新的覆蓋用 PrimeImplicants
    unsigned int cPrimeImplicantsCovered = cPrimeImplicants - cEssential;
    
    mintermGroupT *primeImplicantsCoveredArray = (mintermGroupT *)malloc( cPrimeImplicantsCovered * sizeof(mintermGroupT));
    for (i=0,j=0; i<cPrimeImplicantsCovered; i++) {
        for(;j<cPrimeImplicants;j++) 
            if (isEssential[j] == false){
                memcpy(&primeImplicantsCoveredArray[i], &primeImplicantsArray[j], sizeof(mintermGroupT));
                j++;
                break;
            }
    }
    
    //製作新的需覆蓋 Minterms
    unsigned int cMintermsCovered = 0;
    for (i = 0; i < cMintermsCare ; i++)
        if (mintermsCovered[i]  == false)
            cMintermsCovered++;
    mintermGroupT *mintermCoveredArray = (mintermGroupT *)malloc(cMintermsCovered * sizeof(mintermGroupT));
    for (i = 0,j=0; i < cMintermsCovered; i++) {
        for (;j < cMintermsCare;j++) 
            if (mintermsCovered[j] == false){
                memcpy(&mintermCoveredArray[i], &mintermCareArray[j], sizeof(mintermGroupT));
                j++;
                break;
            }
    }
    
    //新的二維 Cover 表，表示第 j 個 minterm 可被第 i 個 Prime Implicants 覆蓋
    bool **needCovered = (bool **)malloc( cPrimeImplicants * sizeof(bool *) );
	for(i = 0; i < cPrimeImplicants; i++)
		needCovered[i] = (bool *)calloc( cMintermsCare, sizeof(bool));
	CreatePrimeChart(needCovered, mintermCoveredArray, cMintermsCovered, primeImplicantsCoveredArray, cPrimeImplicantsCovered);
    
    //打印新的二維 Cover 表
    printf("\n###############        NEW Prime Implicants Cover         ###############\n\n");
	printf("        ");
    
	for(j = 0; j < cMintermsCovered; j++)
		printf("| %2u", mintermCoveredArray[j].root->id);
	printf("|\n---------");
    
	for(j = 0; j < cMintermsCovered; j++)
		printf("----");
	printf("\n");
	
	for(i = 0; i < cPrimeImplicantsCovered; i++){
		printf("[P%2d]:  ", i);
		for(j = 0; j < cMintermsCovered; j++)
			printf("|  %c", ( needCovered[i][j] == true) ? 'X' : ' ');
		printf("|\n");
	}
    
    idnodeT *maxtermList[cMintermsCovered];
    CreateMaxtermList(needCovered, maxtermList, cMintermsCovered, cPrimeImplicantsCovered);
    
    printf("\n\n###############               Maxterm List 0              ###############");
    //打印 maxtermList
    for (j=0; j < cMintermsCovered; j++) {
        printf("\n\nColumns %d :\n",j);
        for (idnodeT *curr = maxtermList[j]; curr != NULL; curr = curr->next) 
            printf("%2u\n",curr->id);
    }
        printf("\n");
    
    //開始乘開 Maxterm ，並用 Rule 簡化
    for (i = 1; i < cMintermsCovered; i++) {
    printf("\n\n###############               Maxterm List %d              ###############\n",i);
        MultiplyOut(maxtermList, i);
        
        // 打印 maxtermList
        for (j=0; j < cMintermsCovered; j++) {
            printf("\nColumns %d :\n",j);
            for (idnodeT *curr0 = maxtermList[j]; curr0 != NULL; curr0 = curr0->next) {
                for (idnodeT *curr1 = curr0; curr1 != NULL; curr1 = curr1->maxterm) {
                    printf("%2u ",curr1->id);
                }
                printf("\n");
            }
        }
        
        printf("\n\n###############         Maxterm List %d After XX = X       ###############\n",i);
        
        RuleOne(maxtermList);
        
        for (j=0; j < cMintermsCovered; j++) {
            printf("\nColumns %d :\n",j);
            for (idnodeT *curr0 = maxtermList[j]; curr0 != NULL; curr0 = curr0->next) {
                for (idnodeT *curr1 = curr0; curr1 != NULL; curr1 = curr1->maxterm) {
                    printf("%2u ",curr1->id);
                }
                printf("\n");
            }
        }
        
        printf("\n\n###############       Maxterm List %d After X + XY = X     ###############\n",i);
        
        RuleTwo(maxtermList);
        
        for (j=0; j < cMintermsCovered; j++) {
            printf("\nColumns %d :\n",j);
            for (idnodeT *curr0 = maxtermList[j]; curr0 != NULL; curr0 = curr0->next) {
                for (idnodeT *curr1 = curr0; curr1 != NULL; curr1 = curr1->maxterm) {
                    printf("%2u ",curr1->id);
                }
                printf("\n");
            }
        }
    }
    
        
    //找出所有的最小 Maxterm 覆蓋
        idnodeT *shortMaxterm;
        unsigned int cShortMaxterm;
        LeastCover(maxtermList, &shortMaxterm, &cShortMaxterm);
        if (cMintermsCovered > 1) {
            idnodeT *TempMaxterm, *curr0 = shortMaxterm;
            unsigned int  cTempMaxterm;
            for (LeastCover(maxtermList, &TempMaxterm, &cTempMaxterm), curr0 = shortMaxterm;
                 cTempMaxterm == cShortMaxterm;
                 LeastCover(maxtermList, &TempMaxterm, &cTempMaxterm)) {
            
                curr0->next = TempMaxterm;
                curr0 = curr0->next;
            
            }
            curr0->next = NULL;
        }
    
    //打印最小 Maxterm 覆蓋
        printf("\n\n###############            The least Maxterm              ###############\n\n");
        for (idnodeT *curr0 = shortMaxterm; curr0 != NULL; curr0 = curr0->next) {
            for (idnodeT *curr1 = curr0; curr1 != NULL; curr1 = curr1->maxterm) {
                printf("%2u ",curr1->id);
            }
            printf("\n");
        }
        
    //開始打印最終結果
    printf("\n\n###############            The Final Function             ###############\n");
    
    for (idnodeT *curr2 = shortMaxterm; curr2 != NULL; curr2 = curr2->next) {
    
    printf("\nF(");
    for (i = 0; i < cVariables; i++)
        printf("%c%s",ch[i],( i+1 < cVariables) ? "," : ") = ");
    
    //打印 Essential Prime Implicant
    for(unsigned int flag = cEssential, i = 0; i < cPrimeImplicants; i++){
		if(isEssential[i] == true){
            for (j=0; primeImplicantsArray[i].repr[j] != '\0'; j++) {
                if(primeImplicantsArray[i].repr[j] == '1')
                    printf("%c",ch[j]);
                else if(primeImplicantsArray[i].repr[j] == '0'){
                    printf("%c\'",ch[j]);
                }
            }
            if(--flag)
                printf(" + ");
		}
    }
        
        if (cShortMaxterm != 0 && cEssential != 0)
            printf(" + ");
    
    //打印最小 Maxterm
        unsigned int cadd = cShortMaxterm;
    for (idnodeT *curr1 = curr2; curr1 != NULL; curr1 = curr1->maxterm) {
        for (k = 0; primeImplicantsCoveredArray[curr1->id].repr[k] != '\0'; k++) {
            if(primeImplicantsCoveredArray[curr1->id].repr[k] == '1')
                printf("%c",ch[k]);
            else if(primeImplicantsCoveredArray[curr1->id].repr[k] == '0'){
                printf("%c\'",ch[k]);
            }
        }
        if(--cadd)
            printf(" + ");
        
    }
        printf("\n");
    
    }
    printf("\n#########################################################################\n");
    printf("#################        Petrick's Method is End        #################\n");
    printf("#########################################################################\n\n");
    
    }
    
    //結束計時
    finish = clock();
    printf("The Running Time is %f sec.\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    system("PAUSE");
    
    return 0;
}
