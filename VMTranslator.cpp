#include<iostream>
#include<stdio.h>
#include<string>
#include<unordered_map>

using namespace std;

#define Assert(X,Y) if(!(X)) \
{ printf(Y); while (1); }

enum CMD_TYPE
{
	C_NOCMD =0,
	C_ARITHMETIC = 1,
	C_PUSH,
	C_POP
};
enum ARTH_OP
{
	OP_ADD = 1,

};
enum SEGMENTS
{
	SEG_LOCAL = 0,
	SEG_ARG,
	SEG_THIS,
	SEG_THAT,
	SEG_CONSTANT,
	SEG_STATIC,
	SEG_POINTER,
	SEG_TEMP
};
typedef struct CMD{
	CMD_TYPE eCmdType;
	string arg1;
	int arg2;
	CMD()
	{
		eCmdType = C_NOCMD;
		arg1 = "";
		arg2 = -1;
	}
}CMD;
unordered_map <string, CMD_TYPE> cmdMap= {
	{ "push", C_PUSH },
	{ "pop", C_POP },
	{ "add", C_ARITHMETIC },
};
unordered_map <string, ARTH_OP> MathsMap = {
	{ "add", OP_ADD },

};
unordered_map <string, SEGMENTS> SegMap = {
	{ "local", SEG_LOCAL },
	{ "argument", SEG_ARG },
	{ "this", SEG_THIS },
	{ "that", SEG_THAT },
	{ "constant", SEG_CONSTANT },
	{ "static", SEG_STATIC },
	{ "pointer", SEG_POINTER },
	{ "temp", SEG_TEMP }
};

void ParseCmd(string ins,CMD * pstCmd)
{
	string cmdType, seg, index;
	int space1 = ins.find(' ');
	if (space1 >= 0)
	{
		cmdType = ins.substr(0, space1);
		pstCmd->eCmdType = cmdMap[cmdType];

		ins = ins.substr(space1 + 1);
		space1 = ins.find(' ');
		Assert(space1 >= 0, "Wrong parse arg1");
		seg = ins.substr(0, space1);
		pstCmd->arg1 = seg;

		ins = ins.substr(space1 + 1);
		Assert(ins.size() > 0, "Wrong Parse arg2");
		Assert(ins[0] >= '0'&&ins[0] <= '9', "Wrong Parse arg2");
		pstCmd->arg2 = stoi(ins);

	}
	else
	{
		pstCmd->eCmdType = cmdMap[ins];
		Assert(pstCmd->eCmdType == C_ARITHMETIC, "Wrong Parse Arithmetic");
	}

}

#define DEC_SP(hp) fprintf((hp), "@SP\n");fprintf((hp), "M=M-1\n");
#define INC_SP(hp) fprintf((hp), "@SP\n");fprintf((hp), "M=M+1\n");

#define AT_SP_D(hp) fprintf(hp, "@SP\n");\
fprintf(hp, "A=M\n");\
fprintf(hp, "M=D\n");

#define D_AT_SP(hp,op) fprintf(hp, "@SP\n");\
fprintf(hp, "A=M\n");\
fprintf(hp, #op##"\n");


void ConvertPushCmd(CMD * pstCmd, FILE* hp)
{
	switch (SegMap[pstCmd->arg1])
	{
	case SEG_CONSTANT:
		fprintf(hp, "@%d\n", pstCmd->arg2);
		fprintf(hp, "D=A\n");
		AT_SP_D(hp);
		break;
	default:
		Assert(false, "Invalid Segment");
	}
	INC_SP(hp);
}
void ConvertArithmeticCmd(CMD * pstCmd, FILE* hp)
{
	switch (MathsMap[pstCmd->arg1])
	{
		case OP_ADD:
			DEC_SP(hp);
			D_AT_SP(hp,D=M);
			DEC_SP(hp);
			D_AT_SP(hp, D=D+M);
			AT_SP_D(hp);
			INC_SP(hp);
			break;
		default:
			Assert(false, "Incorrect Maths Command");
			break;
	}
}

void ConvertCmd(CMD * pstCmd,FILE* hp)
{
	switch (pstCmd->eCmdType)
	{
		case C_ARITHMETIC:
			ConvertArithmeticCmd(pstCmd, hp);
			break;
		case C_PUSH:
			ConvertPushCmd(pstCmd, hp);
			break;
		case C_POP:
			break;
		default:
			Assert(false, "Incorrect Command");
			break;
	}
}

int main(int argc, char*argv[])
{
	system("cd");
	char hackFn[50];
	Assert(argc >= 2,"Give file name to open");
	FILE* fp = fopen(argv[1], "r");
	char *fn = strtok(argv[1], ".");
	strcpy(hackFn, fn);
	strcat(hackFn, ".asm");
	if (!fp)
	{
		printf("\nFile Not found");
		while (1);
	}
	FILE *hp = fopen(hackFn, "w");
	if (!hp)
	{
		printf("\nUnable to Create hack File");
		while (1);
	}

	char line[100];
	//read each line
	while (fgets(line, 100, fp) != NULL)
	{
		if (line[0] != '\n')//ignore Empty Line
		{
			int i = 0;
			while (i < 99 && line[i] == ' ')i++;
			if (line[i] == '/')//Ignore line Having only comments
				continue;
			else
			{
				int j = 0;
				string ins;
				char c = line[i + j];
				CMD stCurCmd;
				do
				{
					ins.push_back(c);
					j++;
					c = line[i + j];
				} while (c != '\n' &&  c != '/');
				ParseCmd(ins,&stCurCmd);
				fprintf(hp, "//%s\n", ins.c_str()); //Print comment in asm file for each instruction
				if (stCurCmd.eCmdType == C_ARITHMETIC)
				{
					Assert(stCurCmd.arg1.size() == 0, "Arithmetic Command Parse Error");
					stCurCmd.arg1 = ins;
				}
				ConvertCmd(&stCurCmd, hp);

			}
		}
	}


	fclose(fp);
	fclose(hp);
}