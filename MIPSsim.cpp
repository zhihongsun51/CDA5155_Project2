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
int stalled = 0; //If function is stalled dont pull any more instructions
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
    bool isWaiting, self_dirty;
};

std::vector<buff_entry> bbuf, buf1, buf2, buf3, buf4, buf5, buf6;
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

void ins_bbuf(buff_entry b){
    if(bbuf.size() < 1){
        b.isWaiting = true;
        bbuf.push_back(b);
    }
}

void ins_buf1(buff_entry b){
    if(buf1.size() < 8){
        buf1.push_back(b);
    }
}

void ins_buf2(std::vector<buff_entry>::iterator it, buff_entry b){
    buf2.push_back(b);
    buf1.erase(it);
}

void ins_buf4(std::vector<buff_entry>::iterator it, buff_entry b){
    buf4.push_back(b);
    buf1.erase(it);
}

void ins_buf5(std::vector<buff_entry>::iterator it, buff_entry b){
    buf5.push_back(b);
    buf1.erase(it);
}

void ins_buf6(buff_entry b){
    buf6.push_back(b);
    buf2.erase(buf2.begin());
}

void ins_buf8(buff_entry b){
    buf8.push_back(b);
    buf4.erase(buf4.begin());
}

void ins_buf9(buff_entry b){
    buf9.push_back(b);
    buf5.erase(buf5.begin());
}

void ins_buf10(buff_entry b){
    buf10.push_back(b);
    buf6.erase(buf6.begin());
}

void ins_buf11(buff_entry b){
    buf11.push_back(b);
    buf8.erase(buf8.begin());
}

void ins_buf12(buff_entry b){
    buf12.push_back(b);
    buf11.erase(buf11.begin());
}

void category_1(std::string line, int opcode){
    char buff_funct[128];
    buff_entry b;
    b.cat = 1;
    b.opcode = opcode;
    switch(b.opcode){
    case 0: //J 
        b.dest = std::bitset<32>(line).to_ulong();
        b.dest <<= 2;
        sprintf(funct, "%32s \t %d \t J #%d \n", instructions_map[pc].c_str(), pc, b.dest);
        sprintf(buff_funct, "[J #%d]", b.dest);
        b.entry = std::string(buff_funct);
        ins_bbuf(b);
        funct_map[pc] = funct;
        pc = b.dest - 4;
        break;
    case 1: //BEQ
        b.src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        b.src2 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        b.offset = convert_offset(line);
        b.offset <<= 2;

        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t BEQ R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, b.src1, b.src2, b.offset);
        sprintf(buff_funct, "[BEQ R%d, R%d, #%d]", b.src1, b.src2, b.offset);
        b.entry = std::string(buff_funct);
        ins_bbuf(b);
        funct_map[pc] = funct;
        break;
    case 2: //BNE
        b.src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        b.src2 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        b.offset = convert_offset(line);
        b.offset <<= 2;

        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t BNE R%d, R%d, #%d \n", instructions_map[pc].c_str(), pc, b.src1, b.src2, b.offset);
        sprintf(buff_funct, "[BNE R%d, R%d, #%d]", b.src1, b.src2, b.offset);
        b.entry = std::string(buff_funct);
        ins_bbuf(b);
        funct_map[pc] = funct;
        if(registers[b.src1] != registers[b.src2]){
            pc = b.offset + pc;
        }  
        break;
    case 3: //BGTZ
        b.src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,10);
        b.offset = convert_offset(line);
        b.offset <<= 2;
        
        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t BGTZ R%d, #%d \n", instructions_map[pc].c_str(), pc, b.src1, b.offset);
        sprintf(buff_funct, "[BGTZ R%d, #%d]", b.src1, b.offset);
        b.entry = std::string(buff_funct);
        
        ins_bbuf(b);
        funct_map[pc] = funct;
        break;
    case 4: //SW
        b.dest = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        b.src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        b.offset = convert_offset(line);

        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t SW R%d, %d(R%d) \n", instructions_map[pc].c_str(), pc, b.src1, b.offset, b.dest);
        sprintf(buff_funct, "[SW R%d, %d(R%d)]", b.src1, b.offset, b.dest);
        b.entry = std::string(buff_funct);
        ins_buf1(b);
        if(register_map[b.dest] == 0 && register_map[b.src1] == 0){
            register_map[b.dest] = 1;
            b.isWaiting = false;
        } else {
            b.isWaiting = true;
        }
        funct_map[pc] = funct;
        break;
    case 5: //LW
        b.src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        b.dest = std::bitset<5>(line.substr(0,5)).to_ulong();
        line.erase(0,5);
        b.offset = convert_offset(line);
        
        //put formatted string into a map for later use
        sprintf(funct, "%32s \t %d \t LW R%d, %d(R%d) \n", instructions_map[pc].c_str(), pc, b.dest, b.offset, b.src1);
        sprintf(buff_funct, "[LW R%d, %d(R%d)]", b.dest, b.offset, b.src1);
        b.entry = std::string(buff_funct);
        if(register_map[b.dest] == 0 && register_map[b.src1] == 0){
            register_map[b.dest] = 1;
            b.isWaiting = false;
        } else {
            b.isWaiting = true;
        }
        ins_buf1(b);
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
    memset(buff_funct, 0, 128);
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
    if(b.opcode != 4 || b.opcode != 5){
        if(register_map[b.dest] == 0 && register_map[b.src1] == 0 && register_map[b.src2] == 0){
            register_map[b.dest] = 1;
            b.isWaiting = false;
        } else {
            b.isWaiting = true;
        }
    } else {
        if(register_map[b.dest] == 0 && register_map[b.src1] == 0){
            register_map[b.dest] = 1;
            b.isWaiting = false;
        } else {
            b.isWaiting = true;
        }
    }
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
    b.opcode = opcode;
    b.dest = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    b.src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    b.imm_val = convert_offset(line);

    switch(b.opcode){
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
    if(register_map[b.dest] == 0 && register_map[b.src1] == 0){
        register_map[b.dest] = 1;
        b.isWaiting = false;
    } else {
        b.isWaiting = true;
    }
    std::string f(funct);
    funct_map[pc] = funct;
    f.erase(0, 35);
    current_inst = f;
    memset(funct, 0, 128);
    memset(buff_funct, 0, 128);
    ins_buf1(b);
}

void category_4(std::string line, int opcode){
    char buff_funct[128];
    buff_entry b;
    b.cat = 4;
    b.opcode = opcode;
    b.src1 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);
    b.src2 = std::bitset<5>(line.substr(0,5)).to_ulong();
    line.erase(0,5);

    switch(opcode){
    case 0:
        b.inst = "MULT";
        break;
    case 1:
        b.inst = "DIV";
        hiLo[0] = registers[b.src1] / registers[b.src2];
        hiLo[1] = registers[b.src1] % registers[b.src2];
        break;
    default:
        break;
    }

    //put formatted string into a map for later use
    sprintf(funct, "%32s \t %d \t %s R%d, R%d \n", instructions_map[pc].c_str(), pc, b.inst.c_str(), b.src1, b.src2);
    sprintf(buff_funct, "[%s R%d, R%d]", b.inst.c_str(), b.src1, b.src2);
    b.entry = std::string(buff_funct);
    if(register_map[b.src1] == 0 && register_map[b.src2] == 0 && register_map[32] == 0){
        register_map[32] = 1;
        b.isWaiting = false;
    } else {
        b.isWaiting = true;
        register_map[32] = 1;
    }
    ins_buf1(b);
    std::string f(funct);
    funct_map[pc] = funct;
    f.erase(0, 35);
    current_inst = f;
    memset(funct, 0, 128);
}

void category_5(std::string line, int opcode){
    char buff_funct[128];
    buff_entry b;
    b.cat = 5;
    b.opcode = opcode;
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
    if(register_map[b.dest] == 0 && register_map[32] == 0){
        register_map[b.dest] = 1;
        b.self_dirty = true;
        b.isWaiting = false;
    } 
    else if(register_map[b.dest] == 0 && register_map[32] == 1){
        register_map[b.dest] = 1;
        b.self_dirty = true;
        b.isWaiting = true;
    } else {
        b.isWaiting = true;
    }
    ins_buf1(b);
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
    std::string IF = "";
    std::string buffer1 = "";
    std::string buffer2 = "";
    std::string buffer4 = "";
    std::string buffer5 = "";
    std::string buffer6 = "";
    std::string buffer8 = "";
    std::string buffer9 = "";
    std::string buffer10 = "";
    std::string buffer11 = "";
    std::string buffer12 = "";
    char temp[128];
    char line_break[128] = "--------------------\n";
    sprintf(first_line, "Cycle %d\n", counter);
    IF += "\tWaiting:\t";
    if(!bbuf.empty() && bbuf[0].isWaiting){
        IF += bbuf[0].entry;
        IF += "\n\tExecuted:\n";
    } 
    else if(!bbuf.empty() && !bbuf[0].isWaiting) {
        IF += "\n\tExecuted:\t";
        IF += bbuf[0].entry;
        IF += "\n";
    } else {
        IF += "\n\tExecuted:\n";
    }
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
        buffer2 += temp;
        if(buf2.size() > i){
            sprintf(temp, "%s", buf2[i].entry.c_str());
            buffer2 += temp;
        }
    }
    for(int i = 0; i < 2; i++){
        sprintf(temp, "\n\tEntry %d:\t", i);
        buffer4 += temp;
        if(buf4.size() > i){
            sprintf(temp, "%s", buf4[i].entry.c_str());
            buffer4 += temp;
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
    if(buf6.size() > 0){
        sprintf(temp, "%s", buf6[0].entry.c_str());
        buffer6 += temp;
    }
    if(buf8.size() > 0){
        sprintf(temp, "%s", buf8[0].entry.c_str());
        buffer8 += temp;
    }
    if(buf9.size() > 0){
        sprintf(temp, "%s", buf9[0].entry.c_str());
        buffer9 += temp;
    }
    if(buf10.size() > 0){
        sprintf(temp, "%s", buf10[0].entry.c_str());
        buffer10 += temp;
    }
    if(buf11.size() > 0){
        sprintf(temp, "%s", buf11[0].entry.c_str());
        buffer11 += temp;
    }
    if(buf12.size() > 0){
        sprintf(temp, "%s", buf12[0].entry.c_str());
        buffer12 += temp;
    }
    
    buffer1 += "\n";
    buffer2 += "\n";
    buffer4 += "\n";
    buffer5 += "\n";
    buffer6 += "\n";
    buffer8 += "\n";
    buffer9 += "\n";
    buffer10 += "\n";
    buffer11 += "\n";
    buffer12 += "\n";
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
    simulation << "\n";
    simulation << "IF:\n";
    simulation << IF;
    simulation << "Buf1:";
    simulation << buffer1;
    simulation << "Buf2:";
    simulation << buffer2;
    simulation << "Buf4:";
    simulation << buffer4;
    simulation << "Buf5:";
    simulation << buffer5;
    simulation << "Buf6:\t";
    simulation << buffer6;
    simulation << "Buf8:\t";
    simulation << buffer8;
    simulation << "Buf9:\t";
    simulation << buffer9;
    simulation << "Buf10:\t";
    simulation << buffer10;
    simulation << "Buf11:\t";
    simulation << buffer11;
    simulation << "Buf12:\t";
    simulation << buffer12;
    simulation << "\n";
    simulation << "Registers";
    simulation << reg;
    simulation << "HI:";
    simulation << "LO:";
    simulation << "\n\n";
    simulation << "Data";
    simulation << data;
    simulation << "\n";
}

void b_waiting(){
    if(!bbuf[0].isWaiting){
        bbuf.clear();
    }
    switch(bbuf[0].opcode){
    case 1:
        if(register_map[bbuf[0].src1] == 1 || register_map[bbuf[0].src2] == 1){
            stalled = 1;
            bbuf[0].isWaiting = true;
        } else {
            stalled = 0;
            bbuf[0].isWaiting = false;
            if(registers[bbuf[0].src1] == registers[bbuf[0].src2]){
                pc = bbuf[0].offset + pc;
            }
        }
        break;
    case 3: //BGTZ
        if(register_map[bbuf[0].src1] == 1){
            stalled = 1;
            bbuf[0].isWaiting = true;
        } else {
            stalled = 0;
            bbuf[0].isWaiting = false;
            if(registers[bbuf[0].src1] > 0){
                pc = bbuf[0].offset + pc;
            }
        }
        break;
    }
    
}

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
            if(buf2.size() < 2){
                if(!b.isWaiting){
                    ins_buf2(it, b);
                    --it;
                }
            }
            break;
        case 2:
            if(buf5.size() < 2){
                if(!b.isWaiting){
                    ins_buf5(it, b);
                    --it;
                } else {
                    if(b.opcode != 4 || b.opcode != 5){
                        if(register_map[b.dest] == 0 && register_map[b.src1] == 0 && register_map[b.src2] == 0){
                            register_map[b.dest] = 1;
                            b.isWaiting = false;
                            ins_buf2(it, b);
                            --it;
                        }
                    } else {
                        if(register_map[b.dest] == 0 && register_map[b.src1] == 0){
                            register_map[b.dest] = 1;
                            b.isWaiting = false;
                            ins_buf2(it, b);
                            --it;
                        }
                    }
                }
            }
            break;
        case 3:
            if(buf5.size() < 2){
                if(!b.isWaiting){
                    ins_buf5(it, b);
                    --it;
                } else {
                    if(register_map[b.dest] == 0 && register_map[b.src1] == 0){
                        register_map[b.dest] = 1;
                        b.isWaiting = false;
                        ins_buf5(it, b);
                        --it;
                    }
                }
            }
            break;
        case 4:
            if(b.opcode == 0){
                if(buf4.size() < 2){
                    if(!b.isWaiting){
                        ins_buf5(it, b);
                        --it;
                    } else {
                        if(register_map[b.src1] == 0 && register_map[b.src2] == 0){
                            register_map[32] = 1;
                            b.isWaiting = false;
                            ins_buf4(it, b);
                            --it;
                        }
                    }
                }
            }
            break;
        case 5:
        if(buf5.size() < 2){
            if(!b.isWaiting){
                ins_buf5(it, b);
                --it;
            } else {
                if(register_map[b.dest] == 1 && register_map[32] == 0 && b.self_dirty){
                    register_map[b.dest] = 1;
                    b.isWaiting = false;
                    ins_buf5(it, b);
                    --it;
                }
                else if(register_map[b.dest] == 1 && register_map[32] == 0){
                    register_map[b.dest] = 1;
                    b.isWaiting = false;
                    ins_buf5(it, b);
                    --it;
                }
            }
            
        }
            break;    
        default:
            break;
        }
        if(it == buf1.end()){
            break;
        }
    }
}

void ex_alu2(){
    buff_entry b = buf2[0];
    switch(b.opcode){
    case 4://SW
        b.result = registers[b.dest] + b.offset;
        break;
    case 5://LW
        b.result = registers[b.src1] + b.offset;
        break;
    }
    ins_buf6(b);
}

void ex_mem(){
    buff_entry b = buf6[0];
    switch(b.opcode){
    case 4://SW
        data_map[b.result] = registers[b.src1];
        break;
    case 5://LW
        if(data_map.find(b.result) != data_map.end()){
            b.result = data_map[b.result];
        }
        else{
            b.result = registers[b.result];
        }
        break;
    }
    ins_buf10(b);
}

void ex_mul1(){
    buff_entry b = buf4[0];
    ins_buf8(b);
}

void ex_mul2(){
    buff_entry b = buf8[0];
    ins_buf11(b);
}

void ex_mul3(){
    char buff_funct[128];
    std::bitset<64> bresult;
    std::string hi, lo;
    buff_entry b = buf11[0];
    bresult = std::bitset<64>(registers[b.src1] * registers[b.src2]);
    lo = (bresult << 32).to_string();
    lo = lo.substr(0, 32);
    hiLo[1] = std::bitset<32>(lo).to_ulong();
    sprintf(buff_funct, "[%d]", hiLo[1]);
    b.entry = std::string(buff_funct);
    ins_buf12(b);
}

void ex_alu1(){
    char buff_funct[128];
    buff_entry b = buf5[0];
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
    ins_buf9(b);
}

void write_back(){
    //std::cout << "testing";
    if(!buf12.empty()){
        register_map[32] = 0;
        buf12.clear();
    }
    if(!buf10.empty()){
        registers[buf10[0].dest] = buf10[0].result;
        register_map[buf10[0].dest] = 0;
        buf10.clear();
    }
    if(buf9.size() > 0){
        registers[buf9[0].dest] = buf9[0].result;
        register_map[buf9[0].dest] = 0;
        buf9.erase(buf9.begin());
    }
}

int main(int argc, char* argv[]){
    for(int i = 0; i < 33; i++){
        register_map.insert(std::pair<int, int>(i, 0)); //Initialize all registers as unused, Registers 32 represents the HI/LO registers
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
        if(buf10.empty() && !buf6.empty()){
            ex_mem();
        }
        if(buf6.empty() && !buf2.empty()){
            ex_alu2();
        }
        if(buf12.empty() && !buf11.empty()){
            ex_mul3();
        }
        if(buf11.empty() && !buf8.empty()){
            ex_mul2();
        }
        if(buf8.empty() && !buf4.empty()){
            ex_mul1();
        }
        if(buf9.empty() && !buf5.empty()){
            ex_alu1();
        }
        i_issue();

        for(int i = 0; i < 4; i++){
            if(stalled == 0){
                i_fetch(instructions_map[pc]);
                pc += 4;
                if(!bbuf.empty())
                    b_waiting();
                if(instructions_map.find(pc) == instructions_map.end())
                    break;
            } else {
                break;
            }
        }
        if(!bbuf.empty())
            b_waiting();
        
        
        simulation_output();
        
        counter++;
    }

    disassembly_output();
    return 0;
}