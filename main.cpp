#include<iostream>
#include<vector>
#include<fstream>
#include<string>
#include<iomanip>
#include<algorithm>
using namespace std;

string dec2hex(int n) {	//10진수를 16진수로 바꿔주는 함수
	string c_hex = "0123456789ABCDEF";
	string result = "";
	int x, y, num;
	num = n;
	for (int i = 0; i < 6; i++) {
		x = num / 16;
		y = num - (x * 16);
		result += c_hex[y];
		if (x <= 0) break;
		num = x;

	}
	reverse(result.begin(), result.end());
	return result;
}

string resize(string str, int size) {
	string result = "";
	while (result.size() + str.size() < size) {	//크기가 size가 되도록 0을 추가
		result += "0";
	}
	result += str;
	return result;
}


class OPTAB {
public:
	vector<string> instruction, code;
	OPTAB() {
		ifstream F;

		F.open("optab.txt");
		if (F.fail()) {	//파일 입력 예외
			cout << "파일 없음!\n";
			return;
		}

		string str;
		int count = 0;
		while (!F.eof()) {	//파일 끝까지 읽기
			F >> str;	// >>연산자를 이용해 띄어쓰기를 기준으로 끊어 읽기

			//count변수와 %연산자를 이용해 instruction과 code분리해서 저장
			if (count % 2 == 0) instruction.push_back(str);
			else code.push_back(str);

			count++;
		}

		F.close();
	}

	int checkOPTAB(string str) {
		int i = 0;
		for (; i < code.size(); i++) {
			if (str == instruction[i]) 	break;
		}
		//있다면 i의 값은 i < code.size()이고,
		//없다면 i의 값은 i == code.size()가 된다.
		return i;
	}

};

class SYMTAB {
public:
	vector<string> Label;
	vector<int> Loc, flag;

	void setField(string label, int loc, int f) {	//심볼테이블 레코드 추가
		Label.push_back(label);
		Loc.push_back(loc);
		flag.push_back(f);
	}

	int FindLabel(string label) {	//매개변수로 받은 라벨이 심볼테이블에 있는지 검색
		int i = 0;
		for (; i < Label.size(); i++) {
			if (label == Label[i]) 	break;
		}
		//있다면 i의 값은 i < Label.size()이고,
		//없다면 i의 값은 i == Label.size()가 된다.
		return i;
	}

	void printTable() {	//심볼테이블 내용 출력 ( LOC는 16진수로 변환하여 출력 )
		for (int i = 0; i < Label.size(); i++) {
			cout << "Label: " << setw(6) << Label[i] << ", LOC: " << setw(4) << hex << Loc[i] << ", flag: " << flag[i] << endl;
		}
	}
};

class SRCFILE {
	string FileName;
	SYMTAB stb;
	OPTAB otb;
	vector<int> LOC;
	vector<string> LAB, OPC, OPE, OBJCODE;
public:
	SRCFILE(string f) : FileName(f) {
		Pass1();
		Pass2();
	}

	void setSource(int _LOC, string _LAB, string _OPC, string _OPE) {
		LOC.push_back(_LOC);
		LAB.push_back(_LAB);
		OPC.push_back(_OPC);
		OPE.push_back(_OPE);
	}

	void Pass1() {
		ifstream F;
		F.open(FileName);
		if (F.fail()) {
			cout << "[Error] '" << FileName << "' File Not Found\n";
			return;
		}
		string str;	// SRCFILE의 한 line을 저장
		string LABEL, OPCODE, OPERAND;
		int LOCCTR;

		getline(F, str);
		//str에서 LABEL, OPCODE, OPERAND 파싱해서 저장
		LABEL = str.substr(0, str.find(' '));
		OPCODE = str.substr(9, str.find(" ", 9) - 9);
		OPERAND = str.substr(17, str.find(" ", 17) - 17);
		LOCCTR = (OPCODE == "start") ? stoi(OPERAND, NULL, 16) : 0;

		this->setSource(LOCCTR, LABEL, OPCODE, OPERAND);
		stb.setField(LABEL, LOCCTR, 0);


		while (OPCODE != "end") {	//OPCODE가 end일때까지 반복
			getline(F, str);
			if (str[0] != '.') {	//주석이 아닐경우
				//str에서 OPCODE, OPERAND 파싱해서 저장
				LABEL = " ";
				OPCODE = str.substr(9, str.find(" ", 9) - 9);
				OPERAND = str.substr(17, str.find(" ", 17) - 17);


				if (str[0] != ' ') { //라벨필드에 값이 있는 경우
					LABEL = str.substr(0, str.find(' '));	//str에서 LABEL 파싱해서 저장
					int i = stb.FindLabel(LABEL);	//LABEL이 심볼 테이블에 있는지 확인

					if (i != stb.Label.size()) stb.flag[i] = 1;	//있으면 오류
					else stb.setField(LABEL, LOCCTR, 0);		//없으면 해당 라벨 심볼테이블에 추가
				}
				this->setSource(LOCCTR, LABEL, OPCODE, OPERAND);
				//OPCODE를 보고 LOCCTR값 조정
				if (otb.checkOPTAB(OPCODE) < otb.code.size()) LOCCTR += 3;	//OPTAB에 있는 경우 그냥 +3(1word)
				//OPCODE가 아래의 어셈블러 지시자인 경우
				else if (OPCODE == "word")	LOCCTR += 3;
				else if (OPCODE == "resw") 	LOCCTR += 3 * stoi(OPERAND);
				else if (OPCODE == "resb")  LOCCTR += stoi(OPERAND);
				else if (OPCODE == "byte") {
					// 파싱을 통해 ' '사이에 있는 값 크기 체크
					//예외처리 ( OPERAND 값이 10진수가 아니라면 )
					if (OPERAND[0] == 'c') {
						int split = str.find("'") + 1;
						OPERAND = str.substr(split, str.find("'", split) - split);
					}
					LOCCTR += OPERAND.size();
				}
				else { LOCCTR += 3; } //아무것도 해당하지 않는다면 오류

			}

		}

		F.close();
	}

	void Pass2() {
		ifstream F;
		F.open(FileName);
		if (F.fail()) {
			return;
		}

		string str;	// SRCFILE의 한 line을 저장
		string OPCODE, OPERAND, OBJ;	//str에서 파싱을 통해 OPCDOE, OPERAND에 저장 + 목적코드를 저장할 OBJ변수

		while (OPCODE != "end") {	//OPCODE가 end일때까지 반복
			getline(F, str);
			OBJ = "";		//OBJ 초기화

			if (str[0] != '.') {	//주석이 아닐경우
				//str에서 OPCODE, OPERAND 파싱
				OPCODE = str.substr(9, str.find(" ", 9) - 9);
				OPERAND = str.substr(17, str.find(" ", 17) - 17);

				int OPCODE_i = otb.checkOPTAB(OPCODE);	//OPCODE가 OPTAB에 있는지 확인

				if (OPCODE_i != otb.code.size()) { //OPTAB에 있는 경우

					OBJ += otb.code[OPCODE_i];	//OPCODE에 해당하는 목적코드 저장

					if (OPERAND != "") {	//OPERAND에 심볼이 있는 경우

						bool indexing = false;	//예외처리 ( OPERAND에 ','있는 경우 => indexing인 경우 )
						if (OPERAND.find(",") != OPERAND.find("NOT_INDEXING")) {
							OPERAND = OPERAND.substr(0, OPERAND.find(",")); //인덱스(,x)접근 처리
							indexing = true;
						}

						int SYMBOL_i = stb.FindLabel(OPERAND);	//OPERAND가 SYMTAB에 있는지 확인

						if (SYMBOL_i != stb.Label.size()) {	//SYMTAB에 있는 경우
							int temp = stb.Loc[SYMBOL_i];	//SYMTAB에 저장되어있는 10진수 값 가져와 임시 변수에 저장
							if (indexing) temp += stoi("8000", NULL, 16);	//예외처리( indexing인 경우 )
							OBJ += dec2hex(temp);	//10진수를 16진수로 변환하여 OBJ에 저장
						}
						else OBJ = "SYM_ERROR";	//SYMTAB에 없는 경우

					}
					else OBJ += "0000";	//OPERAND에 심볼이 없는 경우 ( rsub )
				}

				else if (OPCODE == "byte" || OPCODE == "word") {	//OPTAB에 없는 OPCODE인 경우

					bool isDec = true;	//예외처리 ( OPERAND 값이 10진수가 아니라면 )
					if (OPERAND[0] == 'c') {
						int split = str.find("'") + 1;
						OPERAND = str.substr(split, str.find("'", split) - split);
						isDec = false;
					}

					if (isDec) {	//OPERAND 값이 10진수라면
						OBJ = "0";	//OPERAND 값으로 목적 코드 생성
						string temp = dec2hex(stoi(OPERAND));	//10진수를 16진수로 변환하여 임시 변수에 저장
						while (OBJ.size() + temp.size() < 6) {	//크기가 6이 되도록 0을 추가
							OBJ += "0";
						}
						OBJ += temp;
					}
					else {	//OPERAND 값이 10진수가 아닌경우
						for (int i = 0; i < OPERAND.size(); i++) {
							OBJ += dec2hex(OPERAND[i]);
						}
					}
				}
				//기타 OPCODE 예외처리 => 목적코드 생성x
				else if (OPCODE == "resb" || OPCODE == "resw") {}
				else if (OPCODE == "start" || OPCODE == "end") {}
				else OBJ = "OP_ERROR";


			}
			//위 과정을 통해 만들어진 OBJ를 OBJCODE에 추가
			OBJCODE.push_back(OBJ);
		}
		F.close();
	}

	void printINTFILE() {	//SRCFILE의 내용을 INTFILE 파일로 출력해주는 함수
		ofstream F("INTFILE");
		for (int i = 0; i < LOC.size(); i++) {
			//조건문을 통해 OBJCODE 오류 처리 해주기
			if (OBJCODE[i] == "SYM_ERROR") {
				F << setw(4) << hex << LOC[i] << setw(8) << " " << setw(8) << LAB[i];
				F << setw(8) << OPC[i] << setw(4) << " " << OPE[i] << endl;
				F << "  **** undefined symbol in operand\n";
			}
			else if (OBJCODE[i] == "OP_ERROR") {
				F << setw(4) << hex << LOC[i] << setw(8) << " " << setw(8) << LAB[i];
				F << setw(8) << OPC[i] << setw(4) << " " << OPE[i] << endl;
				F << "  **** unrecognized operation code\n";
			}
			//조건문을 통해 OBJCODE의 크기가 6보다 클때 처리 ( byte 예외처리 )
			else if (OBJCODE[i].size() > 6) {
				F << setw(4) << hex << LOC[i] << setw(8) << OBJCODE[i].substr(0, 6) << setw(8) << LAB[i];
				F << setw(8) << OPC[i] << setw(4) << " " << OPE[i] << endl;

				//6개씩 잘라서 출력
				int j = 6;
				while (j < OBJCODE[i].size()) {
					F << setw(6) << " " << OBJCODE[i].substr(j, 6) << endl;
					j += 6;
				}
			}
			else {	//일반 출력
				F << setw(4) << hex << LOC[i] << setw(8) << OBJCODE[i] << setw(8) << LAB[i];
				F << setw(8) << OPC[i] << setw(4) << " " << OPE[i] << endl;
			}
		}
		F.close();
	}

	void printOBJFILE() {
		ofstream F("OBJFILE");

		//T레코드 내용을 저장하는 변수와 T_recodes에 추가할 내용을 임시저장하는 변수
		string T_recodes = "", temp = "";

		for (int i = 0; i < LOC.size(); i++) {
			//예외처리( 심볼 or 명령어 문제가 있는 경우) => 종료
			if (OBJCODE[i] == "SYM_ERROR" || OBJCODE[i] == "OP_ERROR") break;

			//opcode가 start인 경우 => H레코드 작성
			if (OPC[i] == "start") {
				string file_size = resize(dec2hex(LOC[LOC.size() - 1] - LOC[0]), 6);
				F << "H" << LAB[i] << " " << resize(dec2hex(LOC[0]), 6) << file_size << endl;
			}
			//opcode가 end인 경우 => T레코드 작성(T_recodes) 및 E레코드 작성
			else if (OPC[i] == "end") {
				if (temp != "") {	//예외처리 (temp가 채워지기 전에 end를 만난 경우) => temp에 남아있는 내용 마저 저장
					T_recodes += resize(dec2hex(temp.size() / 2), 2) + temp + "\n";
				}
				F << T_recodes;	//T레코드 내용 출력

				int SYMBOL_i = stb.FindLabel(OPE[i]);	//심볼테이블에서 명령어 시작위치 찾기
				if (SYMBOL_i != stb.Label.size()) {	//예외처리( end뒤에 명령어시작위치가 없는경우) => E내용 안쓰기
					F << "E" << resize(dec2hex(stb.Loc[SYMBOL_i]), 6);
				}
			}
			//opcode가 resw나 resb인 경우 => T레코드 줄바꿈 해주기
			else if (OPC[i] == "resw" || OPC[i] == "resb") {
				if (temp != "") {	//예외처리(resw/resb 문장이 연속해서 나온 경우)	=> 건너뛰기
					T_recodes += resize(dec2hex(temp.size() / 2), 2) + temp + "\n";
					temp = "";
				}
			}
			//그외의 경우 => T_recodes에 내용 추가
			else {
				//예외처리(T레코드 한 줄 크기가 1E를 넘은경우) => T레코드 줄바꿈 해주기
				if ((temp + OBJCODE[i]).size() / 2 > 30) {
					T_recodes += resize(dec2hex(temp.size() / 2), 2) + temp + "\n";
					T_recodes += "T" + resize(dec2hex(LOC[i]), 6);
					temp = "";
				}
				//줄바꿈했으면 T레코드 시작위치 저장
				else if (temp == "") {
					T_recodes += "T" + resize(dec2hex(LOC[i]), 6);
				}
				temp += OBJCODE[i];	//T레코드 내용 저장
			}

		}
		F.close();
	}

	int find(string obj) {	//OBJCODE를 읽어와 해당하는 값을 반환
		int L = stoi(obj.substr(2,6), NULL, 16);	//OBJCODE의 뒤4자리 16진수값을 L에 저장
		//cout << "L값 : " << L << endl;

		//반복문을 이용하여 LOC에서 해당하는 L값 찾아서 반환
		for (int j = 0; j < LOC.size(); j++) {
			//cout << "LOC값 : " << LOC[j] << endl;
			if (L == LOC[j]) {
				//cout << "반환값 : " << stoi(OBJCODE[j]) << endl;
				return stoi(OBJCODE[j]);
			}
		}
		return -1;
	}


	void play() {
		int rsize;	
		int i=1;
		string f;
		int A;
		cout << "실행: r, 종료: q\n";
		cout << "한 번에 실행할 명령어 갯수 : ";
		cin >> rsize;

		while (i < OBJCODE.size()) {	//OBJCODE 끝까지 반복
			cin >> f;
			if (f == "r") {		//r입력하면 명령어 실행
				for (int k = 0; k < rsize; k++) {
					cout << OBJCODE[i] << " " << OPC[i] << " " << OPE[i] << endl;	//명령어 출력

					//4가지 명령어에 대한 처리 해주기
					//find함수를 통해 A레지스터 값 업데이트
					if (OPC[i] == "lda") {
						A = find(OBJCODE[i]);
					}
					else if (OPC[i] == "add") {
						A += find(OBJCODE[i]);
					}
					else if (OPC[i] == "sub") {
						A -= find(OBJCODE[i]);
					}
					else if (OPC[i] == "mul") {
						A *= find(OBJCODE[i]);
					}

					cout << "REGISTER A: " << A << endl << endl;
					i++;
					if (OPC[i] == "word" || OPC[i] == "byte" || OPC[i] == "resw" || OPC[i] == "resb") break;	//예외처리( word, byte, resw, resb가 나오면 종료)
				}
				if (OPC[i] == "word" || OPC[i] == "byte" || OPC[i] == "resw" || OPC[i] == "resb") break;
				
			}
			else if (f == "q") {	//q입력하면 종료
				break;
			}
			else {	//예외처리( f입력오류 )
				cout << "잘못입력했습니다.\n";
			}
		}
		cout << "종료되었습니다.";

	}

};

int main() {
	string filename;
	cout << "파일 이름 입력: ";
	cin >> filename;
	SRCFILE src(filename);

	src.printINTFILE();
	src.printOBJFILE();
	src.play();

	return 0;
}