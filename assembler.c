#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int* toBin(int n);
void copy(int from[], int to[], int n);
int label_get(FILE *inPtr, char* chr);
int pow(int a, int b);

int main(int argc, char *argv[]){
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr;
	char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
	if (argc != 3) {
		printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",argv[0]);
		exit(1);
	}
	inFileString = argv[1];
	outFileString = argv[2];
	inFilePtr = fopen(inFileString, "r");
	if (inFilePtr == NULL) {
		printf("error in opening %s\n", inFileString);
		exit(1);
	}
	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}
	/* here is an example for how to use readAndParse to read a line from
	inFilePtr */
	/* this is how to rewind the file ptr so that you start reading from the
	beginning of the file */
	rewind(inFilePtr);
	/* after doing a readAndParse, you may want to do the following to test the
	opcode */
	FILE* labelPtr = fopen(inFileString, "r");
	int result;
	int lineNum = -1;
	int binaryBit[32];
	int reg0, reg1, random;
	char* labelList[50];
	while( (result = readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) != 0 ){
		for(int i = 0; i < 32; i++)
			binaryBit[i] = 0;
		
		reg0 = -1;
		reg1 = -1;
		random = -1;
		lineNum++;
		
		if(label == "") labelList[lineNum] = "";
		else if(label != "") labelList[lineNum] = label;//label address set

		if(isNumber(arg0) == 1){
			//reg0 = atoi(arg1[0]);
			sscanf(arg0, "%d", &reg0);
		}	
		if(isNumber(arg1) == 1){
			//reg1 = atoi(arg1[0]);
			sscanf(arg1, "%d", &reg1);
		}
		if(isNumber(arg2) == 1){
			//random = atoi(arg2[0]);
			sscanf(arg2, "%d", &random);
		}
		//opcode
		int total = 0;
		if (!strcmp(opcode, "add") || !strcmp(opcode, "nor")){
			if(reg0 == -1 || reg1 == -1 || random == -1)
				exit(1);//error
			if( !strcmp(opcode, "nor"))
				binaryBit[22] = 1;
			int* regA = toBin(reg0);
			int* regB = toBin(reg1);
			int* dest = toBin(random);
			copy(binaryBit, dest, 0);
			copy(binaryBit, regA, 19);
			copy(binaryBit, regB, 16);
			for(int i =0; i < 32; i++){
				if( binaryBit[i] == 1)
					total += pow(2, i);
			}	
		}else if( !strcmp(opcode, "lw")){
			binaryBit[23] = 1;
			if(reg0 == -1 || reg1 == -1) exit(1);
			int* regA = toBin(reg0);
			int* regB = toBin(reg1);
			copy(binaryBit, regA, 19);
			copy(binaryBit, regB, 16);
			int dest;
			for(int i = 16; i < 32; i++){
				if(binaryBit[i] == 1) total += pow(2, i);
			}
			if(isNumber(arg2)){
				dest = random;
			}else{
				dest = label_get(labelPtr, arg2);
			}
			if(dest < 0) dest = dest + 65536;
			total += dest;
		}else if(!strcmp(opcode, "beq")){
			binaryBit[24] = 1;
			if(reg0 == -1 || reg1 == -1) exit(1);
			int* regA = toBin(reg0);
			int* regB = toBin(reg1);
			copy(binaryBit, regA, 19);
			copy(binaryBit, regB, 16);
			int dest;
			for(int i = 16; i < 32; i++){
				if(binaryBit[i] == 1) total += pow(2, i);
			}
			if(isNumber(arg2)){
				dest = random;
			}else{
				dest = label_get(labelPtr, arg2) - lineNum - 1;
			}
			if(dest < 0) dest += 65536;
			total += dest;
		}else if(!strcmp(opcode, "sw")){
			binaryBit[22] = 1;
			binaryBit[23] = 1;
			if(reg0 == -1 || reg1 == -1) exit(1);
			int* regA = toBin(reg0);
			int* regB = toBin(reg1);
			copy(binaryBit, regA, 19);
			copy(binaryBit, regB, 16);
			int dest;
			for(int i = 16; i < 32; i++){
				if(binaryBit[i] == 1) total += pow(2, i);
			}
			if(isNumber(arg2)){
				dest = random;
			}else{
				dest = label_get(labelPtr, arg2);
			}
			if( dest < 0) dest += 65536;
			total += dest;
		}else if( !strcmp(opcode, "jalr")){
			binaryBit[22] = 1;
			binaryBit[24] = 1;
			if(reg0 == -1 || reg1 == -1) exit(1);
			int* regA = toBin(reg0);
			int* regB = toBin(reg1);
			copy(binaryBit, regA, 19);
			copy(binaryBit, regB, 16);
			for(int i = 16; i < 32; i++){
				if(binaryBit[i] == 1) total += pow(2, i);
			}
		}else if( !strcmp(opcode, "halt") || !strcmp(opcode, "noop")){
			binaryBit[23] = 1;
			binaryBit[24] = 1;
			if ( !strcmp(opcode, "noop")) binaryBit[22] = 1;
			for(int i = 22; i < 25; i++){
				if(binaryBit[i] == 1) total += pow(2, i);
			}
		}else if( !strcmp(opcode, ".fill")){
			if(isNumber(arg0)){
				total = reg0;
				//labelList[lineNum] = label;
			}else{
				int q = label_get(labelPtr, arg0);
				total = q;
				//labelList[lineNum] = label;
			}
		}else{
			printf("unrecognized opcode.\n");
			exit(1);
		}
		fprintf(outFilePtr, "%d\n", total);
	}
	fclose(inFilePtr);
	fclose(outFilePtr);
	fclose(labelPtr);
	return 0;
}
/*
* Read and parse a line of the assembly-language file. Fields are returned
* in label, opcode, arg0, arg1, arg2 (these strings must have memory already
* allocated to them).
*
* Return values:
* 0 if reached end of file
* 1 if all went well
*
* exit(1) if line is too long.
*/
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0, char *arg1, char *arg2){
	char line[MAXLINELENGTH];
	char *ptr = line;
	/* delete prior values */
	label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';
	/* read the line from the assembly-language file */
	if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
		/* reached end of file */
		return(0);
	}
	/* check for line too long (by looking for a \n) */
	if (strchr(line, '\n') == NULL) {
		/* line too long */
		printf("error: line too long\n");
		exit(1);
	}
	/* is there a label? */
	ptr = line;
	if (sscanf(ptr, "%[^\t\n\r ]", label)) {
		/* successfully read label; advance pointer over the label */
		ptr += strlen(label);
	}
	/*
	* Parse the rest of the line. Would be nice to have real regular
	* expressions, but scanf will suffice.
	*/
	sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r "
                "]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
			opcode, arg0, arg1, arg2);


	return(1);
}
int isNumber(char *string){ 
	/* return 1 if string is a number */
	int i;
	return( (sscanf(string, "%d", &i)) == 1);
}

int* toBin(int n){
	int* arr = (int*)malloc(sizeof(int) * 3);
	int tmp = n;
	int i = 0;
	while(tmp != 0){
		arr[i] = tmp%2;
		tmp = tmp/2;
		i++;
	}
	return arr;
}

void copy(int* to, int* from, int n){
	to[n] = from[0];
	to[n+1] = from[1];
	to[n+2] = from[2];
}

int label_get(FILE *inPtr, char* chr){
	int num_of_label;

	int find = 0;
	int cnt = 0;

	char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
	rewind(inPtr);
	while (readAndParse(inPtr, label, opcode, arg0, arg1, arg2)) {
		if (!strcmp(label, chr)) {
			num_of_label = cnt;
			find = 1;
		}
		cnt++;
	}
	if(find) {
		return num_of_label;
	}else{
		exit(1);
	}
}

int pow(int a, int b){
	int ret = 1;
	for(int i = 0; i < b; i++){
		ret = ret * a;
	}
	return ret;
}
