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
int hiLo[2]; //HI is stored in 0, LO is stored in 1
std::map<int, int> register_map; //Keeps track of registers in use. 1 if it is 0 if not
std::map<int, std::string> instructions_map; //Keeps track of all instructions for jumps
std::map<int, std::string> funct_map; //Functions map
std::map<int, int> data_map; //Keeps track of all data
std::map<int, std::string> dfunct_map; //Disassembly function map
char funct[128];

struct buff_entry{
    std::string inst, entry;
    int cat, opcode, dest, src1, src2, offset, imm_val, result;
};

std::vector<buff_entry> buf1, buf2, buf3, buf4, buf5;
std::vector<buff_entry> buf7, buf8, buf9, buf10, buf11, buf12;

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

void beq(std::string line){
    int src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    int src2 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    int offset = convert_offset(line);
    offset <<= 2;

    //put formatted string into a map for later use
    sprintf(funct, "%32s \t %d \t BEQ R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, src1, src2, offset);
    funct_map[pc] = funct;
    if(registers[src1] == registers[src2]){
        pc = offset + pc;
    }
}

void bne(std::string line){
    int src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    int src2 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    int offset = convert_offset(line);
    offset <<= 2;

    //put formatted string into a map for later use
    sprintf(funct, "%32s \t %d \t BNE R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, src1, src2, offset);
    funct_map[pc] = funct;
    if(registers[src1] != registers[src2]){
        pc = offset + pc;
    }   
}

void bgtz(std::string line){
    int src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,10);
    int offset = convert_offset(line);
    offset <<= 2;
    
    //put formatted string into a map for later use
    sprintf(funct, "%32s \t %d \t BGTZ R%d, #%d \n", instructions_map[pc].c_str(), pc, src1, offset);
    funct_map[pc] = funct;
    if(registers[src1] > 0){
        pc = offset + pc;
    }
}

void ins_buf1(buff_entry b){
    if(buf1.size() < 8){
        buf1.push_back(b);
    }
}

void ins_buf5(std::vector<buff_entry>::iterator it, buff_entry b){
    buf5.push_back(b);
    buf1.erase(it);
}

void ins_buf9(std::vector<buff_entry>::iterator it, buff_entry b){
    buf9.push_back(b);
    buf5.erase(it);
}

void category_1(std::string line, int opcode){
    int dest, src1, src2, offset;
    switch(opcode){
    case 0: //J 
        jump(line);
        break;
    case 1: //BEQ
        beq(line);
        break;
    case 2: //BNE
        bne(line);
        break;
    case 3: //BGTZ
        bgtz(line);
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
    memset(funct, 0, 128);
}


void category_2(std::string line, int opcode){
    char buff_funct[128];
    buff_entry b;
    b.cat = 2;
    b.opcode = opcode;
    std::string inst;
    b.dest = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    b.src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    b.src2 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);

    switch(opcode){
    case 0: //ADD
        b.inst = "ADD";
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, R%d \n", instructions_map[pc].c_str(), pc, b.inst.c_str(), b.dest, b.src1, b.src2);
        sprintf(buff_funct, "[%s R%d, R%d, R%d]", b.inst.c_str(), b.dest, b.src1, b.src2);
        break;
    case 1: //SUB
        inst = "SUB";
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, R%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), b.dest, b.src1, b.src2);
        sprintf(buff_funct, "[%s R%d, R%d, R%d]", b.inst.c_str(), b.dest, b.src1, b.src2);
        break;
    case 2: //AND
        inst = "AND";
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, R%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), b.dest, b.src1, b.src2);
        sprintf(buff_funct, "[%s R%d, R%d, R%d]", b.inst.c_str(), b.dest, b.src1, b.src2);
        break;
    case 3: //OR
        inst = "OR";
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, R%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), b.dest, b.src1, b.src2);
        sprintf(buff_funct, "[%s R%d, R%d, R%d]", b.inst.c_str(), b.dest, b.src1, b.src2);
        break;
    case 4: //SRL
        inst = "SRL";
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), b.dest, b.src1, b.src2);
        sprintf(buff_funct, "[%s R%d, R%d, #%d]", b.inst.c_str(), b.dest, b.src1, b.src2);
        break;
    case 5: //SRA
        inst = "SRA";
        sprintf(funct, "%32s \t %d \t %s R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, inst.c_str(), b.dest, b.src1, b.src2);
        sprintf(buff_funct, "[%s R%d, R%d, #%d]", b.inst.c_str(), b.dest, b.src1, b.src2);
        break;
    default:
        break;
    }
    b.entry = std::string(buff_funct);
    //put formatted string into a map for later use
    std::string f(funct);
    funct_map[pc] = funct;
    f.erase(0, 35);
    current_inst = f;
    memset(funct, 0, 128);
    memset(buff_funct, 0, 128);
    ins_buf1(b);
}

void category_3(std::string line, int opcode){
    char buff_funct[128];
    buff_entry b;
    b.cat = 3;
    b.dest = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    b.src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    b.imm_val = convert_offset(line);

    switch(opcode){
    case 0: //ADDI
        b.inst = "ADDI";
        break;
    case 1: //ANDI
        b.inst = "ANDI";
        break;
    case 2: //ORI
        b.inst = "ORI";
        break;
    default:
        break;
    }

    //put formatted string into a map for later use
    sprintf(funct, "%32s \t %d \t %s R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, b.inst.c_str(), b.dest, b.src1, b.imm_val);
    sprintf(buff_funct, "[%s R%d, R%d, #%d]", b.inst.c_str(), b.dest, b.src1, b.imm_val);
    b.entry = std::string(buff_funct);
    std::string f(funct);
    funct_map[pc] = funct;
    f.erase(0, 35);
    current_inst = f;
    memset(funct, 0, 128);
    memset(buff_funct, 0, 128);
    ins_buf1(b);
}

void category_4(std::string line, int opcode){
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
        hiLo[1] = registers[src1] % registers[src2];
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
    memset(funct, 0, 128);
}

void category_5(std::string line, int opcode){
    char buff_funct[128];
    buff_entry b;
    b.dest = std::bitset<5>(line.substr(0,5)).to_ulong();
    switch(opcode){
    case 0: 
        b.inst = "MFHI";
        registers[b.dest] = hiLo[0];
        break;
    case 1:
        b.inst = "MFLO";
        registers[b.dest] = hiLo[1];
        break;
    default:
        break;
    }

    //put formatted string into a map for later use
    sprintf(funct, "%32s \t %d \t %s R%d \n", instructions_map[pc].c_str(), pc, b.inst.c_str(), b.dest);
    sprintf(buff_funct, "[%s R%d]", b.inst.c_str(), b.dest);
    b.entry = std::string(buff_funct);
    std::string f(funct);
    funct_map[pc] = funct;
    f.erase(0, 35);
    current_inst = f;
    memset(funct, 0, 128);
    memset(buff_funct, 0, 128);
}

//Puts data into map
void input_data(std::string line, int pc){
    int data = std::bitset<32>(line).to_ulong();
    data_map[pc] = data;
    sprintf(funct, "%32s \t %d \t %d \n", line.c_str(), pc, data);
    dfunct_map[pc] = funct;
    memset(funct, 0, 128);
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
    std::string buffer1 = "";
    std::string buffer5 = "";
    std::string buffer9 = "";
    char temp[128];
    char line_break[128] = "--------------------\n";
    sprintf(first_line, "Cycle %d\n", counter);
    for(int i = 0; i < 8; i++){
        sprintf(temp, "\n\tEntry %d:\t", i);
        buffer1 += temp;
        if(buf1.size() > i){
            sprintf(temp, "%s", buf1[i].entry.c_str());
            buffer1 += temp;
        }
    }
    for(int i = 0; i < 2; i++){
        sprintf(temp, "\n\tEntry %d:\t", i);
        buffer5 += temp;
        if(buf5.size() > i){
            sprintf(temp, "%s", buf5[i].entry.c_str());
            buffer5 += temp;
        }
    }
    if(buf9.size() > 0){
        sprintf(temp, "%s", buf9[0].entry.c_str());
        buffer9 += temp;
    }
    buffer1 += "\n";
    buffer5 += "\n";
    buffer9 += "\n";
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
    //simulation << "IF:";
    simulation << "Buf1:";
    simulation << buffer1;
    simulation << "Buf5";
    simulation << buffer5;
    simulation << "Buf9:\t";
    simulation << buffer9;
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
        category_1(line, opcode);
        break;
    case 1:
        line.erase(0,6);
        category_2(line, opcode);
        break;
    case 2:
        line.erase(0,6);
        category_3(line, opcode);
        break;
    case 3:
        line.erase(0,6);
        category_4(line, opcode);
        break;
    case 4:
        line.erase(0,6);   
        category_5(line, opcode);
        break;
    default:
        break;
    }
}

void i_issue(){
    for(std::vector<buff_entry>::iterator it = buf1.begin(); it != buf1.end(); ++it){
        buff_entry b = *it;
        switch(b.cat){
        case 1:
            break;
        case 2:
            if(buf5.size() < 2){
                if(register_map[b.dest] == 0 && register_map[b.src1] == 0){
                    if(b.opcode == 4 || b.opcode == 5){
                        ins_buf5(it, b);
                        register_map[b.dest] = 1;
                    } else {
                        if(register_map[b.src2] == 0){
                            ins_buf5(it, b);
                            register_map[b.dest] = 1;
                        }
                    }
                }
                
            }
            break;
        case 3:
            if(buf5.size() < 2){
                if(register_map[b.dest] == 0 && register_map[b.src1] == 0){
                    ins_buf5(it, b);
                    register_map[b.dest] = 1;
                }
                
            }
            break;
        case 4:
            break;
        case 5:
            break;    
        
        }
        if(it == buf1.end()){
            break;
        }
    }
}

void ex_alu1(){
    for(std::vector<buff_entry>::iterator it = buf5.begin(); it != buf5.end(); ++it){
        char buff_funct[128];
        buff_entry b = *it;
        switch(b.cat){
        case 2:
            switch(b.opcode){
            case 0: //ADD
                b.result = registers[b.src1] + registers[b.src2];
                break;
            case 1: //SUB
                b.result = registers[b.src1] - registers[b.src2];
                break;
            case 2: //AND
                b.result = registers[b.src1] & registers[b.src2];
                break;
            case 3: //OR
                b.result = registers[b.src1] | registers[b.src2];
                break;
            case 4: //SRL
                b.result = (unsigned int)(registers[b.src1]) >> b.src2;
                break;
            case 5: //SRA
                b.result = registers[b.src1] >> b.src2;
                break;
            default:
                break;
            }
            break;
        case 3:
            switch(b.opcode){
            case 0: //ADDI
                b.result = registers[b.src1] + b.imm_val;
                break;
            case 1: //ANDI
                b.result = registers[b.src1] & b.imm_val;
                break;
            case 2: //ORI
                b.result = registers[b.src1] | b.imm_val;
                break;
            default:
                break;
            }
            break;
        case 5:
            switch(b.opcode){
            case 0: 
                b.result = hiLo[0];
                break;
            case 1:
                b.result = hiLo[1];
                break;
            default:
                break;
            }
        }
        
        sprintf(buff_funct, "[%d, R%d]", b.result, b.dest);
        b.entry = std::string(buff_funct);
        ins_buf9(it, b);
        if(it == buf5.end()){
            break;
        }
    }
}

void write_back(){
    //std::cout << "testing";
    if(buf9.size() > 0){
        registers[buf9[0].dest] = buf9[0].result;
        register_map[buf9[0].dest] = 0;
        buf9.erase(buf9.begin());
    }
}

int main(int argc, char* argv[]){
    for(int i = 0; i < 32; i++){
        register_map.insert(std::pair<int, int>(i, 0)); //Initialize all registers as unused
    }

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
        write_back();
        if(buf9.size() < 1){
            ex_alu1();
        }
        for(int i = 0; i < 2; i++){
            i_issue();
        }
        for(int i = 0; i < 4; i++){
            i_fetch(instructions_map[pc]);
            pc += 4;
            if(instructions_map.find(pc) == instructions_map.end())
                break;
        }
        simulation_output();
        counter++;
    }

    disassembly_output();
    return 0;
}