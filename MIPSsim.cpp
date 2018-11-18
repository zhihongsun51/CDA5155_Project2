/* On my honor, I have neither given nor received unauthorized aid on this assigment */

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <stdio.h>
#include <map>
#include <stack>
#include <vector>
#include <string.h>

int pc = 256;
int counter = 1;
std::string current_inst;
int registers[32];
std::vector<std::string> buf1, buf2, buf3, buf4, buf5;
std::string buf7, buf8, buf9, buf10, buf11, buf12;
int hiLo[2]; //HI is stored in 0, LO is stored in 1
std::map<int, std::string> instructions_map; //Keeps track of all instructions for jumps
std::map<int, std::string> funct_map; //Functions map
std::map<int, int> data_map; //Keeps track of all data
std::map<int, std::string> dfunct_map; //Disassembly function map
char funct[128];

int convert_offset(std::string offset){
    int dec_offset;
    if(offset.substr(0,1) == "1"){
        dec_offset = std::bitset<16>(offset).flip().to_ulong();
        dec_offset++;
        dec_offset *= -1;
    }
    else{
        dec_offset = std::bitset<16>(offset).to_ulong();
    }
    return dec_offset;
}

void jump(std::string line){
    int dest = std::bitset<32>(line).to_ulong();
    dest <<= 2;
    sprintf(funct, "%32s \t %d \t J #%d \n", instructions_map[pc].c_str(), pc, dest);
    funct_map[pc] = funct;
    pc = dest - 4;
}

void ins_bu(char *buff_funct){
    if(buf1.size() < 8){
        buf1.push_back(std::string(buff_funct));
    }
}

void category_1(std::string line, int opcode){
    int dest, src1, src2, offset;
    switch(opcode){
    case 0: //J 
        jump(line);
        break;
    case 1: //BEQ
        src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        src2 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        offset = convert_offset(line);
        offset <<= 2;

        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t BEQ R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, src1, src2, offset);
        funct_map[pc] = funct;
        if(registers[src1] == registers[src2]){
            pc = offset + pc;
        }
        break;
    case 2: //BNE
        src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        src2 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        offset = convert_offset(line);
        offset <<= 2;

        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t BNE R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, src1, src2, offset);
        funct_map[pc] = funct;
        if(registers[src1] != registers[src2]){
            pc = offset + pc;
        }
        break;
    case 3: //BGTZ
        src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,10);
        offset = convert_offset(line);
        offset <<= 2;
        
        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t BGTZ R%d, #%d \n", instructions_map[pc].c_str(), pc, src1, offset);
        funct_map[pc] = funct;
        if(registers[src1] > 0){
            pc = offset + pc;
        }
        break;
    case 4: //SW
        dest = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        offset = convert_offset(line);
        data_map[registers[dest]+offset] = registers[src1];

        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t SW R%d, %d(R%d) \n", instructions_map[pc].c_str(), pc, src1, offset, dest);
        funct_map[pc] = funct;
        break;
    case 5: //LW
        src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        dest = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        offset = convert_offset(line);
        if(data_map.find(registers[src1]+offset) != data_map.end()){
            registers[dest] = data_map[registers[src1]+offset];
        }
        else{
            registers[dest] = registers[registers[src1]+offset];
        }
        
        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t LW R%d, %d(R%d) \n", instructions_map[pc].c_str(), pc, dest, offset, src1);
        funct_map[pc] = funct;
        break;
    case 6: //Break
        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t BREAK \n", instructions_map[pc].c_str(), pc);
        funct_map[pc] = funct;
        break;
    default:
        break;
    }
    std::string f(funct);
    f.erase(0, 35);
    current_inst = f;
}


void category_2(std::string line, int msb, int opcode){
    char funct[128];
    char buff_funct[128];
    std::string inst;
    int dest = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    int src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    int src2 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);

    switch(opcode){
    case 0: //ADD
        inst = "ADD";
        registers[dest] = registers[src1] + registers[src2];
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, R%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), dest, src1, src2);
        sprintf(buff_funct, "[%s R%d, R%d, R%d]", inst.c_str(), dest, src1, src2);
        break;
    case 1: //SUB
        inst = "SUB";
        registers[dest] = registers[src1] - registers[src2];
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, R%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), dest, src1, src2);
        sprintf(buff_funct, "[%s R%d, R%d, R%d]", inst.c_str(), dest, src1, src2);
        break;
    case 2: //AND
        inst = "AND";
        registers[dest] = registers[src1] & registers[src2];
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, R%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), dest, src1, src2);
        sprintf(buff_funct, "[%s R%d, R%d, R%d]", inst.c_str(), dest, src1, src2);
        break;
    case 3: //OR
        inst = "OR";
        registers[dest] = registers[src1] | registers[src2];
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, R%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), dest, src1, src2);
        sprintf(buff_funct, "[%s R%d, R%d, R%d]", inst.c_str(), dest, src1, src2);
        break;
    case 4: //SRL
        inst = "SRL";
        registers[dest] = (unsigned int)(registers[src1]) >> src2;
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), dest, src1, src2);
        sprintf(buff_funct, "[%s R%d, R%d, #%d]", inst.c_str(), dest, src1, src2);
        break;
    case 5: //SRA
        inst = "SRA";
        registers[dest] = registers[src1] >> src2;
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), dest, src1, src2);
        sprintf(buff_funct, "[%s R%d, R%d, #%d]", inst.c_str(), dest, src1, src2);
        break;
    default:
        break;
    }
    
    //put formatted string into a map for later use
    std::string f(funct);
    funct_map[pc] = funct;
    f.erase(0, 35);
    current_inst = f;
}

void category_3(std::string line, int msb, int opcode){
    char funct[128];
    std::string inst;
    int dest = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    int src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    int imm_val = convert_offset(line);

    switch(opcode){
    case 0: //ADDI
        inst = "ADDI";
        registers[dest] = registers[src1] + imm_val;
        break;
    case 1: //ANDI
        inst = "ANDI";
        registers[dest] = registers[src1] & imm_val;
        break;
    case 2: //ORI
        inst = "ORI";
        registers[dest] = registers[src1] | imm_val;
        break;
    default:
        break;
    }

    //put formatted string into a map for later use
    sprintf(funct, "%32s \t %d \t %s R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), dest, src1, imm_val);
    std::string f(funct);
    funct_map[pc] = funct;
    f.erase(0, 35);
    current_inst = f;
}

void category_4(std::string line, int msb, int opcode){
    char funct[128];
    std::string inst;
    int src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    int src2 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    std::bitset<64> bresult;
    std::string hi, lo;

    switch(opcode){
    case 0:
        inst = "MULT";
        bresult = std::bitset<64>(registers[src1] * registers[src2]);
        hi = (bresult >> 32).to_string();
        hi = hi.substr(32, 32);
        lo = (bresult << 32).to_string();
        lo = lo.substr(0, 32);
        hiLo[0] = std::bitset<32>(hi).to_ulong();
        hiLo[1] = std::bitset<32>(lo).to_ulong();
        break;
    case 1:
        inst = "DIV";
        hiLo[0] = registers[src1] / registers[src2];
        hiLo[0] = registers[src1] % registers[src2];
        break;
    default:
        break;
    }

    //put formatted string into a map for later use
    sprintf(funct, "%32s \t %d \t %s R%d, R%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), src1, src2);
    std::string f(funct);
    funct_map[pc] = funct;
    f.erase(0, 35);
    current_inst = f;
}

void category_5(std::string line, int msb, int opcode){
    char funct[128];
    std::string inst;
    int dest = std::bitset<5>(line.substr(0,5)).to_ulong();
    switch(opcode){
    case 0: 
        inst = "MFHI";
        registers[dest] = hiLo[0];
        break;
    case 1:
        inst = "MFLO";
        registers[dest] = hiLo[1];
        break;
    default:
        break;
    }

    //put formatted string into a map for later use
    sprintf(funct, "%32s \t %d \t %s R%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), dest);
    std::string f(funct);
    funct_map[pc] = funct;
    f.erase(0, 35);
    current_inst = f;
}

//Puts data into map
void input_data(std::string line, int pc){
    char funct[128];
    int data = std::bitset<32>(line).to_ulong();
    data_map[pc] = data;
    sprintf(funct, "%32s \t %d \t %d \n", line.c_str(), pc, data);
    dfunct_map[pc] = funct;
}

//Handles disassembly formatting
void disassembly_output(){
    std::ofstream disassembly;
    disassembly.open("disassembly.txt");
    for(std::map<int, std::string>::iterator it = funct_map.begin(); it != funct_map.end(); ++it){
        disassembly << it->second;
    }
    for(std::map<int, std::string>::iterator it = dfunct_map.begin(); it != dfunct_map.end(); ++it){
        disassembly << it->second;
    }
    disassembly.close();
}

//Handles simulation formatting
void simulation_output(){
    char first_line[50];
    std::string reg = "";
    std::string data = "";
    char temp[128];
    char line_break[128] = "--------------------\n";
    sprintf(first_line, "Cycle %d: \t %s \n", counter, current_inst.c_str());
    for(int i = 0; i < 32; i++){
        if(i % 8 == 0){
        sprintf(temp, "\nR%02d:\t", i);
        reg += temp;
        }
        sprintf(temp, "%d\t", registers[i]);
        reg += temp;
    }

    for(std::map<int, int>::iterator it = data_map.begin(); it != data_map.end(); ++it){
        if((it->first - data_map.begin()->first) % 32 == 0){
        sprintf(temp, "\n%d:\t", it->first);
        data += temp;
        }
        sprintf(temp, "%d\t", it->second);
        data += temp;
    }
    
    std::ofstream simulation;
    simulation.open("simulation.txt", std::ios_base::app);
    simulation << line_break;
    simulation << first_line;
    simulation << "Registers";
    simulation << reg;
    simulation << "\n\n";
    simulation << "Data";
    simulation << data;
    simulation << "\n";
}

//Handles the different kind of instructions
void i_fetch(std::string line){
    int msb = std::bitset<32>(line.substr(0,3)).to_ulong();
    int opcode = std::bitset<32>(line.substr(3,3)).to_ulong();
    switch(msb){
    case 0:
        line.erase(0,6);
        category_1(line, msb, opcode);
        break;
    case 1:
        line.erase(0,6);
        category_2(line, msb, opcode);
        break;
    case 2:
        line.erase(0,6);
        category_3(line, msb, opcode);
        break;
    case 3:
        line.erase(0,6);
        category_4(line, msb, opcode);
        break;
    case 4:
        line.erase(0,6);   
        category_5(line, msb, opcode);
        break;
    default:
        break;
    }
}

int main(int argc, char* argv[]){
    if(remove( "simulation.txt" ) == 0);
    if(remove( "disassembly.txt" ) == 0);
    int pc_data = 256;
    std::ifstream input;
    std::string line;
    std::stack<std::string> mips_input, mips_data, mips_instructions;
    input.open(argv[1]);
    if(input.is_open()){
        while(std::getline(input, line)){
            if(line[line.size()-1] == '\r'){
                line = line.substr(0, line.size()-1);
            }
            mips_input.push(line);
        }
    }
    
    input.close();

    //Separate data from instructions
    while(mips_input.top() != "00011000000000000000000000000000" && !mips_input.empty()){
        mips_data.push(mips_input.top());
        mips_input.pop();
    }
    //Pushes instructions into the stack in order
    while(!mips_input.empty()){
        mips_instructions.push(mips_input.top());
        mips_input.pop();
        pc_data += 4;
    }
    //Pushes data in right order
    while(!mips_data.empty()){
        input_data(mips_data.top(), pc_data);
        mips_data.pop();
        pc_data += 4;
    }
    //Putting instructions into a map
    while(!mips_instructions.empty()){
        instructions_map[pc] = mips_instructions.top();
        pc += 4;
        mips_instructions.pop();
    }

    pc = 256;
    
    //Executing the instructions
    while(instructions_map.find(pc) != instructions_map.end()){
        for(int i = 0; i < 4; i++){
            i_fetch(instructions_map[pc]);
            simulation_output();
            counter ++;
            pc += 4;
            if(instructions_map.find(pc) == instructions_map.end())
                break;
        }
    }

    disassembly_output();
    return 0;
}