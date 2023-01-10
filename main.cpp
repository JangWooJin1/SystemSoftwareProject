#include<iostream>
#include<vector>
#include<fstream>
#include<string>
#include<iomanip>
#include<algorithm>
using namespace std;

string dec2hex(int n) {	//10������ 16������ �ٲ��ִ� �Լ�
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
	while (result.size() + str.size() < size) {	//ũ�Ⱑ size�� �ǵ��� 0�� �߰�
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
		if (F.fail()) {	//���� �Է� ����
			cout << "���� ����!\n";
			return;
		}

		string str;
		int count = 0;
		while (!F.eof()) {	//���� ������ �б�
			F >> str;	// >>�����ڸ� �̿��� ���⸦ �������� ���� �б�

			//count������ %�����ڸ� �̿��� instruction�� code�и��ؼ� ����
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
		//�ִٸ� i�� ���� i < code.size()�̰�,
		//���ٸ� i�� ���� i == code.size()�� �ȴ�.
		return i;
	}

};

class SYMTAB {
public:
	vector<string> Label;
	vector<int> Loc, flag;

	void setField(string label, int loc, int f) {	//�ɺ����̺� ���ڵ� �߰�
		Label.push_back(label);
		Loc.push_back(loc);
		flag.push_back(f);
	}

	int FindLabel(string label) {	//�Ű������� ���� ���� �ɺ����̺� �ִ��� �˻�
		int i = 0;
		for (; i < Label.size(); i++) {
			if (label == Label[i]) 	break;
		}
		//�ִٸ� i�� ���� i < Label.size()�̰�,
		//���ٸ� i�� ���� i == Label.size()�� �ȴ�.
		return i;
	}

	void printTable() {	//�ɺ����̺� ���� ��� ( LOC�� 16������ ��ȯ�Ͽ� ��� )
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
		string str;	// SRCFILE�� �� line�� ����
		string LABEL, OPCODE, OPERAND;
		int LOCCTR;

		getline(F, str);
		//str���� LABEL, OPCODE, OPERAND �Ľ��ؼ� ����
		LABEL = str.substr(0, str.find(' '));
		OPCODE = str.substr(9, str.find(" ", 9) - 9);
		OPERAND = str.substr(17, str.find(" ", 17) - 17);
		LOCCTR = (OPCODE == "start") ? stoi(OPERAND, NULL, 16) : 0;

		this->setSource(LOCCTR, LABEL, OPCODE, OPERAND);
		stb.setField(LABEL, LOCCTR, 0);


		while (OPCODE != "end") {	//OPCODE�� end�϶����� �ݺ�
			getline(F, str);
			if (str[0] != '.') {	//�ּ��� �ƴҰ��
				//str���� OPCODE, OPERAND �Ľ��ؼ� ����
				LABEL = " ";
				OPCODE = str.substr(9, str.find(" ", 9) - 9);
				OPERAND = str.substr(17, str.find(" ", 17) - 17);


				if (str[0] != ' ') { //���ʵ忡 ���� �ִ� ���
					LABEL = str.substr(0, str.find(' '));	//str���� LABEL �Ľ��ؼ� ����
					int i = stb.FindLabel(LABEL);	//LABEL�� �ɺ� ���̺� �ִ��� Ȯ��

					if (i != stb.Label.size()) stb.flag[i] = 1;	//������ ����
					else stb.setField(LABEL, LOCCTR, 0);		//������ �ش� �� �ɺ����̺� �߰�
				}
				this->setSource(LOCCTR, LABEL, OPCODE, OPERAND);
				//OPCODE�� ���� LOCCTR�� ����
				if (otb.checkOPTAB(OPCODE) < otb.code.size()) LOCCTR += 3;	//OPTAB�� �ִ� ��� �׳� +3(1word)
				//OPCODE�� �Ʒ��� ����� �������� ���
				else if (OPCODE == "word")	LOCCTR += 3;
				else if (OPCODE == "resw") 	LOCCTR += 3 * stoi(OPERAND);
				else if (OPCODE == "resb")  LOCCTR += stoi(OPERAND);
				else if (OPCODE == "byte") {
					// �Ľ��� ���� ' '���̿� �ִ� �� ũ�� üũ
					//����ó�� ( OPERAND ���� 10������ �ƴ϶�� )
					if (OPERAND[0] == 'c') {
						int split = str.find("'") + 1;
						OPERAND = str.substr(split, str.find("'", split) - split);
					}
					LOCCTR += OPERAND.size();
				}
				else { LOCCTR += 3; } //�ƹ��͵� �ش����� �ʴ´ٸ� ����

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

		string str;	// SRCFILE�� �� line�� ����
		string OPCODE, OPERAND, OBJ;	//str���� �Ľ��� ���� OPCDOE, OPERAND�� ���� + �����ڵ带 ������ OBJ����

		while (OPCODE != "end") {	//OPCODE�� end�϶����� �ݺ�
			getline(F, str);
			OBJ = "";		//OBJ �ʱ�ȭ

			if (str[0] != '.') {	//�ּ��� �ƴҰ��
				//str���� OPCODE, OPERAND �Ľ�
				OPCODE = str.substr(9, str.find(" ", 9) - 9);
				OPERAND = str.substr(17, str.find(" ", 17) - 17);

				int OPCODE_i = otb.checkOPTAB(OPCODE);	//OPCODE�� OPTAB�� �ִ��� Ȯ��

				if (OPCODE_i != otb.code.size()) { //OPTAB�� �ִ� ���

					OBJ += otb.code[OPCODE_i];	//OPCODE�� �ش��ϴ� �����ڵ� ����

					if (OPERAND != "") {	//OPERAND�� �ɺ��� �ִ� ���

						bool indexing = false;	//����ó�� ( OPERAND�� ','�ִ� ��� => indexing�� ��� )
						if (OPERAND.find(",") != OPERAND.find("NOT_INDEXING")) {
							OPERAND = OPERAND.substr(0, OPERAND.find(",")); //�ε���(,x)���� ó��
							indexing = true;
						}

						int SYMBOL_i = stb.FindLabel(OPERAND);	//OPERAND�� SYMTAB�� �ִ��� Ȯ��

						if (SYMBOL_i != stb.Label.size()) {	//SYMTAB�� �ִ� ���
							int temp = stb.Loc[SYMBOL_i];	//SYMTAB�� ����Ǿ��ִ� 10���� �� ������ �ӽ� ������ ����
							if (indexing) temp += stoi("8000", NULL, 16);	//����ó��( indexing�� ��� )
							OBJ += dec2hex(temp);	//10������ 16������ ��ȯ�Ͽ� OBJ�� ����
						}
						else OBJ = "SYM_ERROR";	//SYMTAB�� ���� ���

					}
					else OBJ += "0000";	//OPERAND�� �ɺ��� ���� ��� ( rsub )
				}

				else if (OPCODE == "byte" || OPCODE == "word") {	//OPTAB�� ���� OPCODE�� ���

					bool isDec = true;	//����ó�� ( OPERAND ���� 10������ �ƴ϶�� )
					if (OPERAND[0] == 'c') {
						int split = str.find("'") + 1;
						OPERAND = str.substr(split, str.find("'", split) - split);
						isDec = false;
					}

					if (isDec) {	//OPERAND ���� 10�������
						OBJ = "0";	//OPERAND ������ ���� �ڵ� ����
						string temp = dec2hex(stoi(OPERAND));	//10������ 16������ ��ȯ�Ͽ� �ӽ� ������ ����
						while (OBJ.size() + temp.size() < 6) {	//ũ�Ⱑ 6�� �ǵ��� 0�� �߰�
							OBJ += "0";
						}
						OBJ += temp;
					}
					else {	//OPERAND ���� 10������ �ƴѰ��
						for (int i = 0; i < OPERAND.size(); i++) {
							OBJ += dec2hex(OPERAND[i]);
						}
					}
				}
				//��Ÿ OPCODE ����ó�� => �����ڵ� ����x
				else if (OPCODE == "resb" || OPCODE == "resw") {}
				else if (OPCODE == "start" || OPCODE == "end") {}
				else OBJ = "OP_ERROR";


			}
			//�� ������ ���� ������� OBJ�� OBJCODE�� �߰�
			OBJCODE.push_back(OBJ);
		}
		F.close();
	}

	void printINTFILE() {	//SRCFILE�� ������ INTFILE ���Ϸ� ������ִ� �Լ�
		ofstream F("INTFILE");
		for (int i = 0; i < LOC.size(); i++) {
			//���ǹ��� ���� OBJCODE ���� ó�� ���ֱ�
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
			//���ǹ��� ���� OBJCODE�� ũ�Ⱑ 6���� Ŭ�� ó�� ( byte ����ó�� )
			else if (OBJCODE[i].size() > 6) {
				F << setw(4) << hex << LOC[i] << setw(8) << OBJCODE[i].substr(0, 6) << setw(8) << LAB[i];
				F << setw(8) << OPC[i] << setw(4) << " " << OPE[i] << endl;

				//6���� �߶� ���
				int j = 6;
				while (j < OBJCODE[i].size()) {
					F << setw(6) << " " << OBJCODE[i].substr(j, 6) << endl;
					j += 6;
				}
			}
			else {	//�Ϲ� ���
				F << setw(4) << hex << LOC[i] << setw(8) << OBJCODE[i] << setw(8) << LAB[i];
				F << setw(8) << OPC[i] << setw(4) << " " << OPE[i] << endl;
			}
		}
		F.close();
	}

	void printOBJFILE() {
		ofstream F("OBJFILE");

		//T���ڵ� ������ �����ϴ� ������ T_recodes�� �߰��� ������ �ӽ������ϴ� ����
		string T_recodes = "", temp = "";

		for (int i = 0; i < LOC.size(); i++) {
			//����ó��( �ɺ� or ��ɾ� ������ �ִ� ���) => ����
			if (OBJCODE[i] == "SYM_ERROR" || OBJCODE[i] == "OP_ERROR") break;

			//opcode�� start�� ��� => H���ڵ� �ۼ�
			if (OPC[i] == "start") {
				string file_size = resize(dec2hex(LOC[LOC.size() - 1] - LOC[0]), 6);
				F << "H" << LAB[i] << " " << resize(dec2hex(LOC[0]), 6) << file_size << endl;
			}
			//opcode�� end�� ��� => T���ڵ� �ۼ�(T_recodes) �� E���ڵ� �ۼ�
			else if (OPC[i] == "end") {
				if (temp != "") {	//����ó�� (temp�� ä������ ���� end�� ���� ���) => temp�� �����ִ� ���� ���� ����
					T_recodes += resize(dec2hex(temp.size() / 2), 2) + temp + "\n";
				}
				F << T_recodes;	//T���ڵ� ���� ���

				int SYMBOL_i = stb.FindLabel(OPE[i]);	//�ɺ����̺��� ��ɾ� ������ġ ã��
				if (SYMBOL_i != stb.Label.size()) {	//����ó��( end�ڿ� ��ɾ������ġ�� ���°��) => E���� �Ⱦ���
					F << "E" << resize(dec2hex(stb.Loc[SYMBOL_i]), 6);
				}
			}
			//opcode�� resw�� resb�� ��� => T���ڵ� �ٹٲ� ���ֱ�
			else if (OPC[i] == "resw" || OPC[i] == "resb") {
				if (temp != "") {	//����ó��(resw/resb ������ �����ؼ� ���� ���)	=> �ǳʶٱ�
					T_recodes += resize(dec2hex(temp.size() / 2), 2) + temp + "\n";
					temp = "";
				}
			}
			//�׿��� ��� => T_recodes�� ���� �߰�
			else {
				//����ó��(T���ڵ� �� �� ũ�Ⱑ 1E�� �������) => T���ڵ� �ٹٲ� ���ֱ�
				if ((temp + OBJCODE[i]).size() / 2 > 30) {
					T_recodes += resize(dec2hex(temp.size() / 2), 2) + temp + "\n";
					T_recodes += "T" + resize(dec2hex(LOC[i]), 6);
					temp = "";
				}
				//�ٹٲ������� T���ڵ� ������ġ ����
				else if (temp == "") {
					T_recodes += "T" + resize(dec2hex(LOC[i]), 6);
				}
				temp += OBJCODE[i];	//T���ڵ� ���� ����
			}

		}
		F.close();
	}

	int find(string obj) {	//OBJCODE�� �о�� �ش��ϴ� ���� ��ȯ
		int L = stoi(obj.substr(2,6), NULL, 16);	//OBJCODE�� ��4�ڸ� 16�������� L�� ����
		//cout << "L�� : " << L << endl;

		//�ݺ����� �̿��Ͽ� LOC���� �ش��ϴ� L�� ã�Ƽ� ��ȯ
		for (int j = 0; j < LOC.size(); j++) {
			//cout << "LOC�� : " << LOC[j] << endl;
			if (L == LOC[j]) {
				//cout << "��ȯ�� : " << stoi(OBJCODE[j]) << endl;
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
		cout << "����: r, ����: q\n";
		cout << "�� ���� ������ ��ɾ� ���� : ";
		cin >> rsize;

		while (i < OBJCODE.size()) {	//OBJCODE ������ �ݺ�
			cin >> f;
			if (f == "r") {		//r�Է��ϸ� ��ɾ� ����
				for (int k = 0; k < rsize; k++) {
					cout << OBJCODE[i] << " " << OPC[i] << " " << OPE[i] << endl;	//��ɾ� ���

					//4���� ��ɾ ���� ó�� ���ֱ�
					//find�Լ��� ���� A�������� �� ������Ʈ
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
					if (OPC[i] == "word" || OPC[i] == "byte" || OPC[i] == "resw" || OPC[i] == "resb") break;	//����ó��( word, byte, resw, resb�� ������ ����)
				}
				if (OPC[i] == "word" || OPC[i] == "byte" || OPC[i] == "resw" || OPC[i] == "resb") break;
				
			}
			else if (f == "q") {	//q�Է��ϸ� ����
				break;
			}
			else {	//����ó��( f�Է¿��� )
				cout << "�߸��Է��߽��ϴ�.\n";
			}
		}
		cout << "����Ǿ����ϴ�.";

	}

};

int main() {
	string filename;
	cout << "���� �̸� �Է�: ";
	cin >> filename;
	SRCFILE src(filename);

	src.printINTFILE();
	src.printOBJFILE();
	src.play();

	return 0;
}