#include<iostream>
#include<stdio.h>
#include<string>
#include<unordered_map>
#include<locale>
#include<filesystem>
#include<stack>

using namespace std;

#define Assert(X,Y) if(!(X)) \
{ printf(Y); while (1); }

enum CMD_TYPE
{
	C_NOCMD =0,
	C_ARITHMETIC = 1,
	C_PUSH,
	C_POP,
	C_LABEL,
	C_GOTO,
	C_IF,
	C_FUNCTION,
	C_RETURN,
	C_CALL
};
enum ARTH_OP
{
	OP_ADD = 0,
	OP_SUB,
	OP_AND,
	OP_OR,
	OP_EQ,
	OP_LT,
	OP_GT,
	OP_NEG,
	OP_NOT,
	OP_ARTH_CNT

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
	{ "eq", C_ARITHMETIC },
	{ "lt", C_ARITHMETIC },
	{ "gt", C_ARITHMETIC },
	{ "sub", C_ARITHMETIC },
	{ "and", C_ARITHMETIC },
	{ "or", C_ARITHMETIC },
	{ "neg", C_ARITHMETIC },
	{ "not", C_ARITHMETIC },
	{"label",C_LABEL},
	{"goto",C_GOTO},
	{ "if-goto",C_IF },
	{"function",C_FUNCTION},
	{"return",C_RETURN},
	{ "call",C_CALL }
};
unordered_map <string, ARTH_OP> MathsMap = {
	{ "add", OP_ADD },
	{ "eq", OP_EQ },
	{ "lt", OP_LT },
	{ "gt", OP_GT },
	{ "sub", OP_SUB },
	{ "and", OP_AND },
	{ "or", OP_OR },
	{ "neg", OP_NEG },
	{ "not", OP_NOT },

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
unordered_map<SEGMENTS, string> SegLabel = {
	{ SEG_LOCAL, "LCL" },
	{ SEG_ARG, "ARG" },
	{ SEG_THIS, "THIS" },
	{ SEG_THAT, "THAT" },
	{ SEG_TEMP, "5" },
};
unordered_map<ARTH_OP, string> CompJump = {
	{ OP_EQ,"JEQ" },
	{ OP_LT, "JLT" },
	{ OP_GT, "JGT" },
	{ OP_SUB, "M-D" },
	{ OP_AND, "D&M" },
	{ OP_OR, "D|M" },
	{ OP_NEG, "-M" },
	{ OP_NOT, "!M" },
};

stack<string> FunctionStack;
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
		//Assert(space1 >= 0, "Wrong parse arg1");
		if (space1 >= 0)
		{
			seg = ins.substr(0, space1);
			pstCmd->arg1 = seg;

			ins = ins.substr(space1 + 1);
			Assert(ins.size() > 0, "Wrong Parse arg2");
			Assert(ins[0] >= '0'&&ins[0] <= '9', "Wrong Parse arg2");
			pstCmd->arg2 = stoi(ins);
		}
		else
		{
			//branch Commands
			pstCmd->arg1 = ins; //Label in arg1
		}

	}
	else
	{
		pstCmd->eCmdType = cmdMap[ins];
		Assert(pstCmd->eCmdType == C_ARITHMETIC || pstCmd->eCmdType == C_RETURN, "Wrong Parse Arithmetic");
	}

}

int ganMathOpCnt[OP_ARTH_CNT] = { 0, };

#define DEC_SP(hp) fprintf((hp), "@SP\n");fprintf((hp), "M=M-1\n");
#define INC_SP(hp) fprintf((hp), "@SP\n");fprintf((hp), "M=M+1\n");

#define AT_SP_D(hp) fprintf(hp, "@SP\n");\
fprintf(hp, "A=M\n");\
fprintf(hp, "M=D\n");

#define AT_SP_VAL(hp,VAL) fprintf(hp, "@SP\n");\
fprintf(hp, "A=M\n");\
fprintf(hp, "M="###VAL##"\n");

#define D_AT_SP(hp,op) fprintf(hp, "@SP\n");\
fprintf(hp, "A=M\n");\
fprintf(hp, #op##"\n");

#define BINARY_OP(hp,op) DEC_SP(hp);\
D_AT_SP(hp, D=M);\
DEC_SP(hp);\
D_AT_SP(hp,op);\

#define UNCOND_JMP(hp,L) fprintf(hp,"@%s\n",L.c_str());\
	fprintf(hp,"0,JMP\n");

#define PUSH_TRUE(hp) AT_SP_VAL(hp,-1);INC_SP(hp);

#define PUSH_FALSE(hp) AT_SP_VAL(hp,0);INC_SP(hp);

void ConvertPushCmd(CMD * pstCmd, FILE* hp,char* fn)
{
	Assert(pstCmd->arg1.size() > 0, "Wrong Segment");
	Assert(pstCmd->arg2 >= 0, "Wrong offset");
	SEGMENTS eSeg = SegMap[pstCmd->arg1];
	Assert(eSeg <= SEG_TEMP, "Wrong Segment");
	switch (eSeg)
	{
	case SEG_CONSTANT:
		fprintf(hp, "@%d\n", pstCmd->arg2);
		fprintf(hp, "D=A\n");
		break;
	case SEG_ARG:
	case SEG_THIS:
	case SEG_THAT:
	case SEG_LOCAL:
		fprintf(hp, "@%d\nD=A\n@%s\nAD=D+M\n", pstCmd->arg2, SegLabel[eSeg].c_str());
		fprintf(hp, "D=M\n");
		break;
	case SEG_TEMP:
		fprintf(hp, "@%d\nD=A\n@%s\nAD=D+A\n", pstCmd->arg2, SegLabel[eSeg].c_str());
		fprintf(hp, "D=M\n");
		break;
	case SEG_POINTER:
		if (pstCmd->arg2 == 0)
			fprintf(hp, "@THIS\nD=M\n");
		else if (pstCmd->arg2 == 1)
			fprintf(hp, "@THAT\nD=M\n");
		else 
		{
			Assert(false, "Wrong Pointer Arg2");
		}
		break;
	case SEG_STATIC:
		fprintf(hp, "@%s.%d\nD=M\n", fn, pstCmd->arg2);
		break;

	default:
		Assert(false, "Invalid Segment");
	}
	AT_SP_D(hp);
	INC_SP(hp);
}

void ConvertPopCmd(CMD * pstCmd, FILE* hp,char*fn)
{
	Assert(pstCmd->arg1.size() > 0, "Wrong Segment");
	Assert(pstCmd->arg2 >= 0, "Wrong offset");
	SEGMENTS eSeg = SegMap[pstCmd->arg1];
	Assert(eSeg <= SEG_TEMP, "Wrong Segment");
	string Label = SegLabel[eSeg];
	if (eSeg == SEG_POINTER)
	{
		fprintf(hp,"@SP\nAM=M-1\nD=M\n");
		if (pstCmd->arg2 == 0)
			fprintf(hp, "@THIS\nM=D\n");
		else if (pstCmd->arg2 == 1)
			fprintf(hp, "@THAT\nM=D\n");
		else
		{
			Assert(false, "Wrong Pointer Arg2");
		}
	}
	else if (eSeg == SEG_STATIC)
	{
		DEC_SP(hp);
		D_AT_SP(hp,D=M);
		fprintf(hp, "@%s.%d\nM=D\n", fn, pstCmd->arg2);
	}
	else
	{
		if (eSeg == SEG_TEMP)
		{
			fprintf(hp, "@%d\nD=A\n@%s\nD=D+A\n", pstCmd->arg2, Label.c_str());
		}
		else
		{
			fprintf(hp, "@%d\nD=A\n@%s\nD=D+M\n", pstCmd->arg2, Label.c_str());
		}
		AT_SP_D(hp);
		fprintf(hp, "@SP\nA=M-1\nD=M\n");
		fprintf(hp, "@SP\nA=M\nA=M\nM=D\n");
		DEC_SP(hp)
	}
}

string CreateLabel(string op)
{
	std::locale loc;
	std::string str = op;
	for (std::string::size_type i = 0; i<str.length(); ++i)
		str[i]= std::toupper(str[i], loc);
	str += "_";
	str += to_string(ganMathOpCnt[MathsMap[op]]++);
	return str;
}
void ConvertBinaryMathsOp(FILE* hp, ARTH_OP MathsOp)
{
	switch (MathsOp)
	{
	case OP_ADD:
		BINARY_OP(hp,M=D+M);
		break;
	case OP_SUB:
		BINARY_OP(hp,M=M-D);
		break;
	case OP_AND:
		BINARY_OP(hp,M=M&D);
		break;
	case OP_OR:
		BINARY_OP(hp,M=M|D);
		break;
	default:
		Assert(false, "Incorrect Maths Command");
		break;
	}
	INC_SP(hp);
}
void ConvertArithmeticCmd(CMD * pstCmd, FILE* hp)
{
	string Label;
	ARTH_OP MathsOp = MathsMap[pstCmd->arg1];
	switch (MathsOp)
	{
		case OP_ADD:
		case OP_SUB:
		case OP_AND:
		case OP_OR:
			ConvertBinaryMathsOp(hp, MathsOp);
			break;
		case OP_LT:
		case OP_EQ:
		case OP_GT:
			BINARY_OP(hp, D=M-D);
			Label = CreateLabel(pstCmd->arg1);
			fprintf(hp, "@%s\n", Label.c_str());
			fprintf(hp, "D;%s\n", CompJump[MathsOp].c_str());
			PUSH_FALSE(hp);
			fprintf(hp, "@END_%s\n", Label.c_str());
			fprintf(hp, "0;JMP\n");
			fprintf(hp, "(%s)\n", Label.c_str());
			PUSH_TRUE(hp);
			fprintf(hp, "(END_%s)\n", Label.c_str());
			break;
		case OP_NEG:
		case OP_NOT:
			fprintf(hp, "@SP\n");
			fprintf(hp, "A=M-1\n");
			fprintf(hp, "M=%s\n",CompJump[MathsOp].c_str());
			break;
		default:
			Assert(false, "Incorrect Maths Command");
			break;
	}
}
void WriteLabel(CMD * pstCmd, FILE* hp)
{
	Assert(pstCmd->arg1.size() > 0, "Wrong Label String");
	Assert(pstCmd->arg2 == -1, "Wrong Label Command Parse");
	fprintf(hp, "(%s)\n", pstCmd->arg1.c_str());
}
void WriteIfGoTo(CMD * pstCmd, FILE* hp)
{
	Assert(pstCmd->arg1.size() > 0, "Wrong Label String");
	Assert(pstCmd->arg2 == -1, "Wrong Label Command Parse");
	//DEC_SP(hp);
	fprintf(hp, "@SP\nAM=M-1\nD=M\n");
	fprintf(hp, "@%s\nD;JNE\n", pstCmd->arg1.c_str());

}
void WriteGoTo(CMD * pstCmd, FILE* hp)
{
	Assert(pstCmd->arg1.size() > 0, "Wrong Label String");
	Assert(pstCmd->arg2 == -1, "Wrong Label Command Parse");
	
	fprintf(hp, "@%s\n0;JMP\n", pstCmd->arg1.c_str());

}
void WriteFunction(CMD * pstCmd, FILE* hp)
{
	Assert(pstCmd->arg1.size() > 0, "Wrong Function Name");
	Assert(pstCmd->arg2 >= 0, "Wrong Function Locals Count");
	//string LabelForLocalPush(pstCmd);
	FunctionStack.push(pstCmd->arg1);
	fprintf(hp, "(%s)\n", pstCmd->arg1.c_str());
	fprintf(hp, "@%d\nD=A\n",pstCmd->arg2);
	fprintf(hp, "(LOCAL_PUSH_%s_%d)\n",pstCmd->arg1.c_str(),pstCmd->arg2);
	fprintf(hp, "@END_LOCAL_PUSH_%s_%d\nD=D-1;JLT\n", pstCmd->arg1.c_str(), pstCmd->arg2);
	PUSH_FALSE(hp);
	fprintf(hp, "@LOCAL_PUSH_%s_%d\n0;JMP\n", pstCmd->arg1.c_str(), pstCmd->arg2);
	fprintf(hp, "(END_LOCAL_PUSH_%s_%d)\n", pstCmd->arg1.c_str(), pstCmd->arg2);
	//fprintf(hp, "@LOCAL_PUSH_%s_%d\nD=D-1;JGT\n", pstCmd->arg1.c_str(), pstCmd->arg2);
}

void WriteReturn(CMD * pstCmd, FILE* hp)
{
	Assert(pstCmd->arg1.size() == 0, "Wrong Label String");
	Assert(pstCmd->arg2 == -1, "Wrong Label Command Parse");
	//FRAME=LCL
	fprintf(hp, "@LCL\t//FRAME=LCL\nD=M\n@FRAME\nM=D\n\n");
	//RET=*(FRAME-5)
	fprintf(hp, "@5\t//RET=*(FRAME-5)\nAD=D-A\nD=M\n@ret_%s\nM=D\n\n", FunctionStack.top().c_str());
	//*ARG=POP()
	fprintf(hp, "@SP\t//*ARG=POP()\nAM=M-1\nD=M\n");
	//DEC_SP(hp);
	//D_AT_SP(hp,D=M);
	fprintf(hp, "@ARG\nA=M\nM=D\n\n");
	//SP=ARG+1
	fprintf(hp, "D=A\t//SP=ARG+1\n@SP\nM=D+1\n\n");

	//THAT = *(FRAME-1)
	fprintf(hp, "@FRAME\t//THAT=*(FRAME-1)\nAM=M-1\nD=M\n@THAT\nM=D\n\n");
	//THIS = *(FRAME-2)
	fprintf(hp, "@FRAME\t//THIS=*(FRAME-2)\nAM=M-1\nD=M\n@THIS\nM=D\n\n");
	//ARG= *(FRAME-3)
	fprintf(hp, "@FRAME\t//ARG=*(FRAME-3)\nAM=M-1\nD=M\n@ARG\nM=D\n\n");
	//LCL= *(FRAME-4)
	fprintf(hp, "@FRAME\t//LCL=*(FRAME-4)\nAM=M-1\nD=M\n@LCL\nM=D\n\n");
	fprintf(hp, "@ret_%s\n", FunctionStack.top().c_str());
	fprintf(hp, "A=M\n0;JMP\n");
	FunctionStack.pop();

}
int gnRetCall = 0;
void WriteCall(CMD * pstCmd, FILE* hp)
{
	Assert(pstCmd->arg1.size() > 0, "Wrong Function Name");
	Assert(pstCmd->arg2 >= 0, "Wrong Function Locals Count");
	int n = pstCmd->arg2;
	//push return Address
	fprintf(hp, "@RETURN_ADD_CALL%d\t//push return Address\nD=A\n", gnRetCall);
	AT_SP_D(hp); INC_SP(hp);
	//push LCL
	fprintf(hp, "@LCL\t//push LCL\nD=M\n");
	AT_SP_D(hp); INC_SP(hp);
	//push ARG
	fprintf(hp, "@ARG\t//push ARG\nD=M\n");
	AT_SP_D(hp); INC_SP(hp);
	//push THIS
	fprintf(hp, "@THIS\t//push THIS\nD=M\n");
	AT_SP_D(hp); INC_SP(hp);
	//push THAT
	fprintf(hp, "@THAT\t//push THAT\nD=M\n");
	AT_SP_D(hp); INC_SP(hp);
	//ARG=SP-n-5
	fprintf(hp, "@SP\t//ARG=SP-n-5\nD=M\n@%d\nD=D-A\n@5\nD=D-A\n@ARG\nM=D\n", n);
	//LCL=SP
	fprintf(hp, "@SP\t//LCL=SP\nD=M\n@LCL\nM=D\n");
	//goto f
	fprintf(hp, "@%s\t//goto f\n0;JMP\n", pstCmd->arg1.c_str());
	//(ret address)
	fprintf(hp, "(RETURN_ADD_CALL%d)\n",gnRetCall++);
}


void ConvertCmd(CMD * pstCmd,FILE* hp,char*fn)
{
	switch (pstCmd->eCmdType)
	{
		case C_ARITHMETIC:
			ConvertArithmeticCmd(pstCmd, hp);
			break;
		case C_PUSH:
			ConvertPushCmd(pstCmd, hp,fn);
			break;
		case C_POP:
			ConvertPopCmd(pstCmd, hp,fn);
			break;
		case C_LABEL:
			WriteLabel(pstCmd, hp);
			break;
		case C_IF:
			WriteIfGoTo(pstCmd, hp);
			break;
		case C_GOTO:
			WriteGoTo(pstCmd, hp);
			break;
		case C_FUNCTION:
			WriteFunction(pstCmd,hp);
			break;
		case C_RETURN:
			WriteReturn(pstCmd, hp);
			break;
		case C_CALL:
			WriteCall(pstCmd, hp);
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
	//if (argc == 3)//dir
	//{
	//	printf("Directory Name = %s", argv[2]);
	//	strcpy(hackFn, argv[2]);
	//}
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
				while (ins.back() == ' ')ins.pop_back();
				ParseCmd(ins,&stCurCmd);
				fprintf(hp, "//%s\n", ins.c_str()); //Print comment in asm file for each instruction
				if (stCurCmd.eCmdType == C_ARITHMETIC)
				{
					Assert(stCurCmd.arg1.size() == 0, "Arithmetic Command Parse Error");
					stCurCmd.arg1 = ins;
				}
				ConvertCmd(&stCurCmd, hp,fn);

			}
		}
	}


	fclose(fp);
	fclose(hp);
}