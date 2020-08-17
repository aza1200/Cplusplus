#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#pragma warning(disable:4996)

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
void binary(char* what, char* target, int target_length);
int change2binary(char * what);

int library_cnt = 0;
int line_cnt = 0;
bool last_line = false;

struct Library {
	char order[1000];
	int where;
	Library() {
		where = 0;
	}

	Library(char* a, int b) {
		strcpy(order, a);
		where = b;
	}
};

int main(int argc, char * argv[]) {
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr;

	char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
		arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

	if (argc != 3) {
		printf("error : usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
		exit(1);
	}

	inFileString = argv[1];
	outFileString = argv[2];

	inFilePtr = fopen(inFileString, "r");

	Library library[MAXLINELENGTH];
	for (int i = 0; i <= 999; i++) library[i] = Library();

	if (inFilePtr == NULL) {
		printf("error in opening %s\n", inFileString);
		exit(1);
	}

	outFilePtr = fopen(outFileString, "w");

	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}

	/*
	1. 문자배열과 그에해당하는 숫자 집어넣을수있는 자료구조 하나만들기
	2. 하나씩돌면서 fill일때 정지하고 문자배열과 숫자 집어넣어주기
	3.
	*/

	while (true) {
		readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2);
		if (strcmp(label, "")) {
			library[library_cnt] = Library(label, line_cnt);
			library_cnt++;
			line_cnt++;
			continue;
		}

		if (!strcmp(opcode, ".fill")) {
			if (isNumber(arg0)) {
				library[library_cnt] = Library(label, line_cnt);
				library_cnt++;
			}
		}
		line_cnt++;
		if (last_line) break;
	}


	last_line = false;
	rewind(inFilePtr); //인파일 포인터 초기화

	line_cnt = -1;
	//----------내가 고생하는 부분 ↓ -------------------
	//처음부터 읽으면서 숫자 집어넣장 
	//here is an example for how to use readAndParseto read a line from in FilePtr
	while (true) {
		line_cnt++;
		readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2);
		char what[MAXLINELENGTH];
		char* finalstring = what;
		what[0] = '\0';

		if (!strcmp(opcode, "add")) strcat(finalstring, "000");
		else if (!strcmp(opcode, "nor"))strcat(finalstring, "001");
		else if (!strcmp(opcode, "lw")) strcat(finalstring, "010");
		else if (!strcmp(opcode, "sw")) strcat(finalstring, "011");
		else if (!strcmp(opcode, "beq")) strcat(finalstring, "100");
		else if (!strcmp(opcode, "jalr")) strcat(finalstring, "101");
		else if (!strcmp(opcode, "halt")) {
			strcat(finalstring, "1100000000000000000000000");
			fprintf(outFilePtr, "%d\n", change2binary(finalstring));

			continue;

		}
		else if (!strcmp(opcode, "noop")) {
			strcat(finalstring, "1110000000000000000000000");
			fprintf(outFilePtr, "%d\n", change2binary(finalstring));


			continue;
		}
		else if (!strcmp(opcode, ".fill")) {
			if (isNumber(arg0)) {
				strcat(finalstring, arg0);
				fprintf(outFilePtr, "%s\n", finalstring);
			}
			else {
				for (int i = 0; i < library_cnt; i++) {
					if (!strcmp(library[i].order, arg0)) {
						int now = library[i].where;
						fprintf(outFilePtr, "%d\n", now);
						break;
					}
				}
			}
			continue;
		}

		//reg1 reg2 이진수 로 추가 하깅
		binary(arg0, finalstring, 3);
		binary(arg1, finalstring, 3);

		if (!strcmp(opcode, "jalr")) {
			strcat(finalstring, "0000000000000000");
			fprintf(outFilePtr, "%d\n", change2binary(finalstring));

			continue;
		}
		else if (!strcmp(opcode, "add") || !strcmp(opcode, "nor")) {
			strcat(finalstring, "0000000000000");
			binary(arg2, finalstring, 3);
			fprintf(outFilePtr, "%d\n", change2binary(finalstring));

		}
		else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq")) {
			if (isNumber(arg2)) {
				//숫자일땐 걍 offsetfield 에 숫자 집어넣으면 된다 
				binary(arg2, finalstring, 16);

			}
			else {
				//영어일때에는 미리저장해둔 자료배열에다가 찾아서 해당 숫자 집어넣어주기 .....
				for (int i = 0; i < library_cnt; i++) {
					if (!strcmp(library[i].order, arg2)) {

						int now_line = line_cnt;

						int now = library[i].where;


						if (!strcmp(opcode, "beq")) {
							now = now - (now_line + 1);
						}

						bool negative = false;
						if (now < 0) {
							negative = true;
							now = -now;
						}

						char ex[1000];
						int cnt = 0;

						//음수면 코드를 살짝 바꾸장!! 
						while (now) {
							char what = (now % 10) + '0';
							ex[cnt++] = what;
							now /= 10;
						}

						char real[1000];
						char* what1 = real;
						if (!negative) {
							what1[cnt] = '\0';
							for (int i = 0; i < cnt; i++) {
								what1[i] = ex[cnt - i - 1];
							}
						}
						else {
							what1[cnt + 1] = '\0';
							what1[0] = '-';
							for (int i = 1; i <= cnt; i++) {
								what1[i] = ex[cnt - i];
							}
						}
						//now 를 what1로 즉, int 에서 char 로 형변환 했음 
						binary(what1, finalstring, 16);
						break;


					}

				}
			}
			fprintf(outFilePtr, "%d\n", change2binary(finalstring));

		}
		if (last_line) break;
	}

	//main문 종료
}
/*
Read and parse a line of the assembly- language file. Fields are returned in label,opcode,arg0,arg1,arg2
(these strings must have memory already allocated to them).

Return values:
0 if reached end of file
1 if all went well
exit(1) if line is too long
*/

int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0, char *arg1, char *arg2) {
	char line[MAXLINELENGTH];
	char *ptr = line;
	label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

	/* read the line from the assembly-language file */


	//한 줄 단위로 읽어들이는 함수임 
	if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
		//cout << "마지막줄";
		return 1;

	}


	/*check for line too long (by looking for a \n)*/
	if (strchr(line, '\n') == NULL) {
		ptr = line;

		if (sscanf(ptr, "%[^\t\n\r]", label)) {
			/* successfully read label: advance pointer over the label*/
			ptr += strlen(label);
		}

		sscanf(ptr, "%*[\t\n\r   ]%[^\t\n\r   ]%*[\t\n\r   ]%[^\t\n\r   ]%*[\t\n\r   ]%[^\t\n\r   ]%*[\t\n\r   ]%[^\t\n\r   ]"
			, opcode, arg0, arg1, arg2);

		/* line is too long*/
		//printf("error : line too long \n");
		//exit(1);
		last_line = true;
		return 0;
	}


	/* is there a label? */
	ptr = line;

	if (sscanf(ptr, "%[^\t\n\r]", label)) {
		/* successfully read label: advance pointer over the label*/
		ptr += strlen(label);
	}
	/*
	parse the rest of the line. Would be nice to have real regular
	expressions , but scanf will suffice.
	*/
	sscanf(ptr, "%*[\t\n\r   ]%[^\t\n\r   ]%*[\t\n\r   ]%[^\t\n\r   ]%*[\t\n\r   ]%[^\t\n\r   ]%*[\t\n\r   ]%[^\t\n\r   ]"
		, opcode, arg0, arg1, arg2);

	return(1);

}

int isNumber(char *string) {
	/*return 1 if string is a number*/
	int i;
	return((sscanf(string, "%d", &i)) == 1);
}

void binary(char* what, char* target, int target_length) {
	int now = atoi(what);
	int number = now;
	if (now < 0) now = -now;


	char ex[1000];
	char *hey = ex;
	ex[0] = '\0';
	int length = 0;

	while (now) {
		if (now % 2) strcat(hey, "1");
		else strcat(hey, "0");
		length++;
		now = now / 2;
	}

	while (length != target_length) {
		strcat(hey, "0");
		length++;
	}
	char whatwhat[1000];
	char * ex2 = whatwhat;
	ex2[0] = '\0';

	for (int i = target_length - 1; i >= 0; i--) {
		ex2[target_length - 1 - i] = hey[i];
	}

	ex2[target_length] = '\0';
	if (number >= 0) {
		strcat(target, ex2);
	}
	else {
		for (int i = 0; i < target_length; i++) {
			if (ex2[i] == '1') ex2[i] = '0';
			else ex2[i] = '1';
		}
		int plus_add = target_length - 1;
		ex2[plus_add]++;
		while (ex2[plus_add] == '2') {
			ex2[plus_add] = '0';
			ex2[plus_add - 1]++;
			plus_add--;
		}

		strcat(target, ex2);

	}
}

int change2binary(char * what) {
	int now = 1;
	int ret = 0;
	int length = 0;

	for (length = 0; what[length] != NULL; length++);

	for (int i = length - 1; i >= 0; i--) {
		if (what[i] == '1') {
			ret += now;
		}
		now *= 2;
	}

	return ret;
}