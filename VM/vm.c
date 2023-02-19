#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MEMORY_SIZE 2048

#define OPCODE_ADD 2
#define OPCODE_ADDI 3
#define OPCODE_SUB 4
#define OPCODE_SUBI 5
#define OPCODE_MUL 6
#define OPCODE_MULI 7
#define OPCODE_DIV 8
#define OPCODE_DIVI 9
#define OPCODE_AND 10
#define OPCODE_ANDI 11
#define OPCODE_OR 12
#define OPCODE_ORI 13
#define OPCODE_XOR 14
#define OPCODE_XORI 15
#define OPCODE_SHL 16
#define OPCODE_SHLI 17
#define OPCODE_SHR 18
#define OPCODE_SHRI 19
#define OPCODE_SLT 20
#define OPCODE_SLTI 21
#define OPCODE_SLE 22
#define OPCODE_SLEI 23
#define OPCODE_SEQ 24
#define OPCODE_SEQI 25
#define OPCODE_LOAD 27
#define OPCODE_STORE 29
#define OPCODE_JMPR 30
#define OPCODE_JMPI 31
#define OPCODE_BRAZ 32
#define OPCODE_BRANZ 33
#define OPCODE_SCALL 34
#define OPCODE_STOP 35

u_int32_t memory[MEMORY_SIZE];
int registers[32];

int pc = 0;

int instruction;
int opcode = 0;
int rd = 0;
int rs = 0;
int rs1 = 0;
int rs2 = 0;
int ra = 0;
int addr = 0;
int n = 0;
u_int32_t imm = 0;

int is_running = 1;


void write_registers(int index, int value)
{
    if (index == 0)
        value = 0;
    registers[index] = value;
}

void read_file(char *filename)
{
    printf("Début lecture fichier\n");
    FILE *inputFile = fopen(filename, "rb");
    if (inputFile == NULL)
    {
        perror("Erreur : ouverture du fichier");
        exit(-1);
    }
    int i = 0;
    while (fread(&memory[i], 4, 1, inputFile))
    {
        i++;
    }
    printf("Fin lecture fichoer\n");
}

void decode_r()
{
    rd = (instruction >> 21) & 0x1f;
    rs1 = (instruction >> 16) & 0x1f;
    rs2 = (instruction >> 11) & 0x1f;
}
void decode_i()
{
    rd = (instruction >> 21) & 0x1f;
    rs = (instruction >> 16) & 0x1f;
    imm = instruction & 0x0000ffff;

    if ((imm & 0x00008000) != 0)
    {
        imm |= 0xffff0000;
    }
}
void decode_jr()
{
    rd = (instruction >> 21) & 0x1f;
    ra = (instruction >> 16) & 0x1f;
}
void decode_ji()
{
    rd = (instruction >> 21) & 0x1f;
    addr = instruction & 0x1fffff;
}
void decode_b()
{
    rs = (instruction >> 21) & 0x1f;
    addr = instruction & 0x1fffff;
}
void decode_s()
{
    n = instruction & 0x3ffffff;
}

void op_add()
{
    decode_r();
    write_registers(rd, registers[rs1] + registers[rs2]);
}

void op_addi()
{
    decode_i();
    write_registers(rd, registers[rs] + imm);
}

void op_sub()
{
    decode_r();
    write_registers(rd, registers[rs1] - registers[rs2]);
}
void op_subi()
{
    decode_i();
    write_registers(rd, registers[rs] - imm);
}
void op_mul()
{
    decode_r();
    write_registers(rd, registers[rs1] * registers[rs2]);
}
void op_muli()
{
    decode_i();
    write_registers(rd, registers[rs] * imm);
}
void op_div()
{
    decode_r();
    write_registers(rd, registers[rs1] / registers[rs2]);
}
void op_divi()
{
    decode_i();
    write_registers(rd, registers[rs] / imm);
}
void op_and()
{
    decode_r();
    write_registers(rd, registers[rs1] && registers[rs2]);
}
void op_andi()
{
    decode_i();
    write_registers(rd, registers[rs] && imm);
}
void op_or()
{
    decode_r();
    write_registers(rd, registers[rs1] || registers[rs2]);
}
void op_ori()
{
    decode_i();
    write_registers(rd, registers[rs] || imm);
}
void op_xor()
{
    decode_r();
    write_registers(rd, registers[rs1] ^ registers[rs2]);
}
void op_xori()
{
    decode_i();
    write_registers(rd, registers[rs] ^ imm);
}
void op_shl()
{
    decode_r();
    write_registers(rd, registers[rs1] << registers[rs2]);
}
void op_shli()
{
    decode_i();
    write_registers(rd, registers[rs] << imm);
}
void op_shr()
{
    decode_r();
    write_registers(rd, registers[rs1] >> registers[rs2]);
}
void op_shri()
{
    decode_i();
    write_registers(rd, registers[rs] >> imm);
}
void op_slt()
{
    decode_r();
    write_registers(rd, registers[rs1] < registers[rs2]);
}
void op_slti()
{
    decode_i();
    write_registers(rd, registers[rs] < imm);
}
void op_sle()
{
    decode_r();
    write_registers(rd, registers[rs1] <= registers[rs2]);
}
void op_slei()
{
    decode_i();
    write_registers(rd, registers[rs] <= imm);
}
void op_seq()
{
    decode_r();
    write_registers(rd, registers[rs1] == registers[rs2]);
}
void op_seqi()
{
    decode_i();
    write_registers(rd, registers[rs] == imm);
}
void op_load()
{
    decode_i();
    write_registers(rd, memory[registers[rs] + imm]);
}
void op_store()
{
    decode_i();
    memory[registers[rs] + imm] = registers[rd];
}
void op_jmpr()
{
    decode_jr();
    write_registers(rd, pc);
    pc = registers[ra];
}
void op_jmpi()
{
    decode_ji();
    write_registers(rd, pc);
    pc = addr;
}
void op_braz()
{
    decode_b();
    if (registers[rs] == 0)
        pc = addr;
}
void op_branz()
{
    decode_b();
    if (registers[rs] != 0)
        pc = addr;
}
void op_scall()
{
    decode_s();
    int input;
    switch (n)
    {
    case 0:
        printf("Input number : ");
        scanf("%d", &input);
        write_registers(20, input);
        break;
    case 1:
        printf("%d\n", registers[20]);
        break;
    case 3:
        printf("%c\n", registers[20] & 0x7f);
        break;
    default:
        break;
    }
}
void op_stop()
{
    is_running = 0;
}

void eval()
{
    switch (opcode)
    {
    case OPCODE_ADD:
        op_add();
        break;
    case OPCODE_ADDI:
        op_addi();
        break;
    case OPCODE_SUB:
        op_sub();
        break;
    case OPCODE_SUBI:
        op_subi();
        break;
    case OPCODE_MUL:
        op_mul();
        break;
    case OPCODE_MULI:
        op_muli();
        break;
    case OPCODE_DIV:
        op_div();
        break;
    case OPCODE_DIVI:
        op_divi();
        break;
    case OPCODE_AND:
        op_and();
        break;
    case OPCODE_ANDI:
        op_andi();
        break;
    case OPCODE_OR:
        op_or();
        break;
    case OPCODE_ORI:
        op_ori();
        break;
    case OPCODE_XOR:
        op_xor();
        break;
    case OPCODE_XORI:
        op_xori();
        break;
    case OPCODE_SHL:
        op_shl();
        break;
    case OPCODE_SHLI:
        op_shli();
        break;
    case OPCODE_SHR:
        op_shr();
        break;
    case OPCODE_SHRI:
        op_shri();
        break;
    case OPCODE_SLT:
        op_slt();
        break;
    case OPCODE_SLTI:
        op_slti();
        break;
    case OPCODE_SLE:
        op_sle();
        break;
    case OPCODE_SLEI:
        op_slei();
        break;
    case OPCODE_SEQ:
        op_seq();
        break;
    case OPCODE_SEQI:
        op_seqi();
        break;
    case OPCODE_LOAD:
        op_load();
        break;
    case OPCODE_STORE:
        op_store();
        break;
    case OPCODE_JMPR:
        op_jmpr();
        break;
    case OPCODE_JMPI:
        op_jmpi();
        break;
    case OPCODE_BRAZ:
        op_braz();
        break;
    case OPCODE_BRANZ:
        op_branz();
        break;
    case OPCODE_SCALL:
        op_scall();
        break;
    case OPCODE_STOP:
        op_stop();
        break;
    }
}

void run()
{
    printf("Début programme \n");
    while (is_running)
    {
        instruction = memory[pc++];
        opcode = (instruction >> 26) & 0x3f;
        eval();
    }
    printf("Fin programme \n");
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Erreur : nombre de paramètres \n");
        return EXIT_FAILURE;
    }

    char *filename = (char *)malloc(100 * sizeof(char));
    strcpy(filename, argv[1]);
    read_file(filename);
    free(filename);
    run();

    return EXIT_SUCCESS;
}

