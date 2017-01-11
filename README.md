# 32bit_Quine-McCluskey_and_Petrick_Method_in_C
## Features
1. Support optimize Boolean functions with 32 variables, so minterm range is 0 ~ 4294967295
2. Support for don't care conditions
3. Support for input out of order
4. Support for Petrick's method to minimize Boolean functions
5. Support output all minimized Boolean functions
6. Support show the full operation for each process

## Features
1. 支援化簡三十二位變數， 即 Minterm 範圍為 0 ~ 4294967295
2. 支援輸入 Don't Care
3. 支援亂序輸入，即不須按大小順序排列
3. 支援視情況自動調用 Petrick's Method，輸出最小覆蓋函數
4. 支援輸出所有等價的最小覆蓋函數
5. 輸入表達式的長度限制為 1000 字符，但理論上限以運行時電腦的記憶體空間決定，
	可於 mccluskey.h 中直接修改以下宏定義，擴充輸入的限制範圍：
	#define EXPRESSION_MAX_LENGTH 1001

## Instructions for use
運行後依提示分兩行輸入 Minterm 與 Don't care，並用逗號「,」隔開各項即可，若無 Don't care 請輸入 -1，因無輸入偵錯，請避免額外的空白與字符。

注意：在某些特殊情況下，會輸出不同順序的同一函數，但並不影響正確性。
EX: m(9,12,13,15)+d(1,4,5,7,8,11,14)

## References & Gratitude
- QM算法及其他
	http://mprc.pku.edu.cn/courses/digital/2013spring/pdf/lec8.QM.pdf
- 以自動化程式化簡布林代數
	http://web.fg.tp.edu.tw/~tfghdb/blog/wp-content/uploads/2016/01/WL21_pp501-pp511_以自動化程式化簡布林代數.pdf
- 增強型 Qunie-Mccluskey 算法及其實驗
	http://www.paper.edu.cn/journal/downCount/1674-2869(2011)01-0100-04
- Petrick's Method 
	http://www.mrc.uidaho.edu/mrc/people/jff/349/lect.10
- qmc-algo in C language
	https://github.com/kkanellis/qmc-algo
- QuineMcCluskeySolver
	http://quinemccluskey.com/index.php
