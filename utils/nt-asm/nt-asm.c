#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum {
    I_LB    = 0,
    I_LBU   = 1,
    I_LH    = 2,
    I_LHU   = 3,
    I_LW    = 4,
    I_LL    = 5,
    I_SB    = 6,
    I_SH    = 7,
    I_SW    = 8,
    I_SC    = 9,
    
    I_ADDI  = 10,
    I_ADDIU = 11,
    I_SLTI  = 12,
    I_SLTIU = 13,
    I_ANDI  = 14,
    I_ORI   = 15,
    I_XORI  = 16,
    I_LUI   = 17,

    I_ADD   = 18,
    I_ADDU  = 19,
    I_SUB   = 20,
    I_SUBU  = 21,
    I_SLT   = 22,
    I_SLTU  = 23,
    I_AND   = 24,
    I_OR    = 25,
    I_XOR   = 26,
    I_NOR   = 27,

    I_SLL   = 28,
    I_SRL   = 29,
    I_SRA   = 30,
    I_SLLV  = 31,
    I_SRLV  = 32,
    I_SRAV  = 33,

    I_MULT  = 34,
    I_MULTU = 35,
    I_DIV   = 36,
    I_DIVU  = 37,
    I_MFHI  = 38,
    I_MTHI  = 39,
    I_MFLO  = 40,
    I_MTLO  = 41,

    I_J     = 42,
    I_JAL   = 43,
    I_JR    = 44,
    I_JALR  = 45,

    I_BEQ   = 46,
    I_BNE   = 47,
    I_BLTZ  = 48,
    I_BGEZ  = 49,

    I_SCALL = 50,
    I_BREAK = 51,
    I_ERET  = 52,

    I_MFC0  = 53,
    I_MTC0  = 54,

    I_SYNC  = 55,

    // pseudo insts
    I_NOP   = 56,
    I_MOVE  = 57,
    I_BEQZ  = 58,
    I_BNEZ  = 59,
    I_NOT   = 60,

    NR_INST
};

const char INSTS_LITERIAL[] = 
    "lb lbu lh lhu lw ll sb sh sw sc addi addiu slti sltiu andi ori xori lui "
    "add addu sub subu slt sltu and or xor nor sll srl sra sllv srlv srav mult "
    "multu div divu mfhi mthi mflo mtlo j jal jr jalr beq bne bltz bgez "
    "syscall break eret mfc0 mtc0 sync nop move beqz bnez not"
    " ";

const char REG_LITERIAL_ALIAS[] =
    "$zero $at $v0 $v1 $a0 $a1 $a2 $a3 $t0 $t1 $t2 $t3 $t4 $t5 $t6 $t7 $s0 $s1 "
    "$s2 $s3 $s4 $s5 $s6 $s7 $t8 $t9 $k0 $k1 $gp $sp $fp $ra"
    " ";

const char REG_LITERIAL_NUMBER[] =
    "$0 $1 $2 $3 $4 $5 $6 $7 $8 $9 $10 $11 $12 $13 $14 $15 $16 $17 $18 $19 $20 "
    "$21 $22 $23 $24 $25 $26 $27 $28 $29 $30 $31"
    " ";

struct Section
{
    char *name;
    unsigned int length : 16;
    unsigned int hash : 16;
    size_t offset;
    size_t size;
    unsigned int *content;
    struct Section *next;
};

struct Section *
find_section(struct Section **head, char *name, int length, unsigned int hash)
{
    hash &= 0xFFFF;

    while (*head) {
        if ((*head)->hash == hash &&
            (*head)->length == length && 
            strncmp(name, (*head)->name, length) == 0) {
            return *head;
        }
        head = &((*head)->next);
    }

    *head = (struct Section*)malloc(sizeof(struct Section));
    (*head)->name   = malloc(length + 1);
    memcpy((*head)->name, name, length + 1);
    (*head)->length = length;
    (*head)->hash   = hash;
    (*head)->offset = 0xFFFFFFFF;
    (*head)->size   = 0;
    (*head)->content= NULL;
    (*head)->next   = NULL;

    return *head;
}

struct Section *section_table;
struct Section comm_section;
struct Section *current_section;

struct Symbol
{
    char *name;
    unsigned int externel    : 1;
    unsigned int global      : 1;
    unsigned int length      : 14;
    unsigned int hash        : 16;
    size_t offset;
    struct Section *section;
    struct Symbol *next;
};

struct Symbol *
lookup_symbol(struct Symbol *head, char *name, int length, unsigned int hash)
{
    hash &= 0xFFFF;

    while (head) {
        if (head->hash == hash &&
            head->length == length &&
            strncmp(name, head->name, length) == 0) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

struct Symbol *
find_symbol(struct Symbol **head, char *name, int length, unsigned int hash)
{
    struct Symbol *sym = lookup_symbol(*head, name, length, hash);
    if (sym) return sym;

    sym = (struct Symbol*)(malloc(sizeof(struct Symbol)));
    sym->name       = malloc(length + 1);
    memcpy(sym->name, name, length + 1);
    sym->externel   = 1;
    sym->global     = 0;
    sym->length     = length;
    sym->hash       = hash;
    sym->offset     = 0xFFFFFFFF;
    sym->section    = current_section;
    sym->next       = *head;

    return (*head = sym);
}

struct Symbol *label_table;
struct Symbol *inst_table;
struct Symbol *reg_table;

void
print_help()
{
    printf(
            "nt-asm: New Technology Assembler\n"
            "\n"
            "Usage:\n"
            "  nt-asm <options> input_files\n"
            "\n"
            "Options:\n"
            "  -a <alignment>       specfic alignment of .ent, default 16\n"
            "  -b <base>            specfic base address of output\n"
            "  -h                   print this help and exit\n"
            "  -l <limit>           specfic section limit\n"
            "  -o <output_path>     specfic output file\n"
            "  -v                   print version and exit\n"
            "\n"
        );
}

void
print_version()
{
    printf(
            "nt-asm 0.1\n"
            "Author: Clarkok Zhang(mail@clarkok.com)\n"
        );
}

size_t
file_size(FILE *fp)
{
    size_t ret;

    fseek(fp, 0, SEEK_END);
    ret = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return ret;
}

unsigned int
string_hash(const char *str, int length)
{
    unsigned int hash = 0;
    while (length--) {
        hash *= 147;
        hash ^= *str++;
    }

    return hash;
}

char *
pass_white(char *scan_ptr, int *line)
{
    while (*scan_ptr && isspace(*scan_ptr)) {
        if (*scan_ptr == '\n') ++*line;
        ++scan_ptr;
    }
    return scan_ptr;
}

char *
pass_non_white(char *scan_ptr, int *line)
{
    while (*scan_ptr && !isspace(*scan_ptr)) { ++scan_ptr; }
    return scan_ptr;
    (void)line;
}

char *
pass_token(char *scan_ptr, int *line)
{
    while (
            *scan_ptr &&
            (isalnum(*scan_ptr) || 
             (*scan_ptr == '_') ||
             (*scan_ptr == '$') ||
             (*scan_ptr == '.')
            )
        ) ++scan_ptr;
    return scan_ptr;
    (void)line;
}

char *
parse_reg(char *scan_ptr, int *line)
{
    scan_ptr = pass_white(scan_ptr, line);

    char *token_start = scan_ptr;
    char *token_limit = pass_token(scan_ptr, line);

    if (!lookup_symbol(
                reg_table,
                token_start,
                token_limit - token_start,
                string_hash(token_start, token_limit - token_start)
            )) {
        printf("ERROR: Expect register, met `%.*s', line %d\n",
                token_limit - token_start, token_start, *line);
        exit(-1);
    }

    return token_limit;
}

char *
parse_chr(char *scan_ptr, char c, int *line)
{
    while (*scan_ptr && *scan_ptr != c) {
        if (!isspace(*scan_ptr)) {
            printf("ERROR: Expect char `%c', met `%c'(%d), line %d\n",
                    c,
                    *scan_ptr,
                    *scan_ptr,
                    *line
                );
            exit(-1);
        }
        ++scan_ptr;
    }
    return scan_ptr ? scan_ptr + 1 : scan_ptr;
}

char *
parse_imm(char *scan_ptr, int *line)
{
    scan_ptr = pass_white(scan_ptr, line);

    if (*scan_ptr == '%') {
        ++scan_ptr;
        if (
            strncmp(scan_ptr, "hi", 2) == 0 ||
            strncmp(scan_ptr, "lo", 2) == 0
        ) {
            scan_ptr = parse_chr(scan_ptr + 2, '(', line);
            scan_ptr = parse_imm(scan_ptr, line);
            scan_ptr = parse_chr(scan_ptr, ')', line);

            return scan_ptr;
        }
        else {
            printf("ERROR: Unknown helper function `%.*s', line %d\n",
                    pass_token(scan_ptr, line) - scan_ptr,
                    scan_ptr,
                    *line
                );
            exit(-1);
        }
    }

    if (*scan_ptr == '-') {
        ++scan_ptr;
    }

    if (*scan_ptr == '0') {
        switch (*++scan_ptr) {
            case 'x':   while (isxdigit(*++scan_ptr)); break;
            case 'b':   do { ++scan_ptr; } while (*scan_ptr == '0' || *scan_ptr == '1'); break;
            case '0':   case '1':   case '2':   case '3':
            case '4':   case '5':   case '6':   case '7':
                while (*scan_ptr >= '0' && *scan_ptr <= '7') ++scan_ptr;
                break;
            default:
                break;
        }
    }
    else if (*scan_ptr > '0' && *scan_ptr <= '9') {
        while (*scan_ptr >= '0' && *scan_ptr <= '9') ++scan_ptr;
    }
    else {
        scan_ptr = pass_token(scan_ptr, line);
    }
    return scan_ptr;
}

char *
parse_directive(char *scan_ptr, int *line, int alignment)
{
    char *directive_limit = pass_non_white(scan_ptr, line);
    char *label_limit;

    if (strncmp(scan_ptr, ".globl", directive_limit - scan_ptr) == 0) {
        scan_ptr = pass_white(directive_limit, line);
        label_limit = pass_non_white(scan_ptr, line);
        find_symbol(
            &label_table,
            scan_ptr,
            label_limit - scan_ptr,
            string_hash(scan_ptr, label_limit - scan_ptr)
        )->global = 1;
        return label_limit;
    }
    else if (strncmp(scan_ptr, ".comm", directive_limit - scan_ptr) == 0) {
        scan_ptr = pass_white(directive_limit, line);
        label_limit = pass_token(scan_ptr, line);
        struct Symbol* sym = find_symbol(
            &label_table,
            scan_ptr,
            label_limit - scan_ptr,
            string_hash(scan_ptr, label_limit - scan_ptr)
        );
        scan_ptr = parse_chr(label_limit, ',', line);

        size_t size = strtoul(scan_ptr, &scan_ptr, 10);
        scan_ptr = parse_chr(scan_ptr, ',', line);
        size_t align = strtoul(scan_ptr, &scan_ptr, 10);

        printf("comm %.*s size %u align %d\n", 
                sym->length, sym->name,
                size, align
            );

        sym->offset = comm_section.size = (comm_section.size + align - 1) & -align;
        sym->section = &comm_section;
        comm_section.size += size;
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".4byte", directive_limit - scan_ptr) == 0) {
        current_section->size += 4;
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".text", directive_limit - scan_ptr) == 0) {
        current_section = find_section(
                &section_table,
                ".text",
                strlen(".text"),
                string_hash(".text", strlen(".text"))
            );
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".data", directive_limit - scan_ptr) == 0) {
        current_section = find_section(
                &section_table,
                ".data",
                strlen(".data"),
                string_hash(".data", strlen(".data"))
            );
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".section", directive_limit - scan_ptr) == 0) {
        scan_ptr = pass_white(directive_limit, line);
        if (*scan_ptr == '\"') {
            ++scan_ptr;
            label_limit = strpbrk(scan_ptr, "\"");
        }
        else {
            label_limit = strpbrk(scan_ptr, ",\n\t");
        }
        current_section = find_section(
                &section_table,
                scan_ptr,
                label_limit - scan_ptr,
                string_hash(scan_ptr, label_limit - scan_ptr)
            );
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".ent", directive_limit - scan_ptr) == 0) {
        current_section->size = (current_section->size + alignment - 1) & -alignment;
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".asciiz", directive_limit - scan_ptr) == 0 ||
             strncmp(scan_ptr, ".asciz", directive_limit - scan_ptr) == 0) {
        scan_ptr = parse_chr(directive_limit, '"', line);
        while (*scan_ptr != '"') {
            current_section->size ++;
            if (*scan_ptr == '\\') {
                ++scan_ptr;
            }
            ++scan_ptr;
        }
        current_section->size = (current_section->size + 4) & -4;
        ++scan_ptr;
    }
    else if (strncmp(scan_ptr, ".ascii", directive_limit - scan_ptr) == 0) {
        scan_ptr = parse_chr(directive_limit, '"', line);
        while (*scan_ptr != '"') {
            current_section->size ++;
            if (*scan_ptr == '\\') {
                ++scan_ptr;
            }
            ++scan_ptr;
        }
        current_section->size = (current_section->size + 3) & -4;
        ++scan_ptr;
    }
    else if (
        strncmp(scan_ptr, ".abicalls",  directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".align",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".end",       directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".file",      directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".fmask",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".frame",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".ident",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".local",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".mask",      directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".nan",       directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".option",    directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".set",       directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".size",      directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".type",      directive_limit - scan_ptr) == 0 ||
        0
    ) {
        // known directive but do nothing
        return strpbrk(scan_ptr, "\n");     // jump to end of line
    }
    else {
        printf("WARNING: unknown directive `%.*s', line %d\n",
                directive_limit - scan_ptr,
                scan_ptr,
                *line
            );
        printf("%.*s\n",
                strpbrk(scan_ptr, "\n") - scan_ptr,
                scan_ptr
            );
        return strpbrk(scan_ptr, "\n");     // jump to end of line
    }

    return scan_ptr;
}

char *
parse_inst_label(char *scan_ptr, int *line)
{
    char *token_start = scan_ptr;
    char *token_limit = pass_token(scan_ptr, line);
    struct Symbol *sym;

    sym = lookup_symbol(
            inst_table,
            token_start,
            token_limit - token_start,
            string_hash(token_start, token_limit - token_start)
        );
    if (!sym) {
        scan_ptr = pass_white(token_limit, line);
        if (*scan_ptr != ':') {
            printf("ERROR: Expect `:' after label, `%c'(%d) met, line %d\n",
                    *scan_ptr,
                    *scan_ptr,
                    *line
                );
            printf("    after label `%.*s'\n", token_limit - token_start, token_start);
            exit(-1);
        }
        ++scan_ptr;

        sym = find_symbol(
                &label_table,
                token_start,
                token_limit - token_start,
                string_hash(token_start, token_limit - token_start)
            );
        sym->externel = 0;
        sym->offset = current_section->size;
    }
    else {
        scan_ptr    = token_limit;
        switch (sym->offset) {
            case I_LB:      case I_LBU:     case I_LH:      case I_LHU:
            case I_LW:      case I_LL:      case I_SB:      case I_SH:
            case I_SW:      case I_SC:
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_imm(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, '(', line);
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ')', line);
                break;
            case I_ADDI:    case I_ADDIU:   case I_SLTI:    case I_SLTIU:
            case I_ANDI:    case I_ORI:     case I_XORI:
            case I_SLL:     case I_SRL:     case I_SRA:
            case I_BEQ:     case I_BNE:
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_imm(scan_ptr, line);
                break;
            case I_LUI:
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_imm(scan_ptr, line);
                break;
            case I_ADD:     case I_ADDU:    case I_SUB:     case I_SUBU:
            case I_SLT:     case I_SLTU:    case I_AND:     case I_OR:
            case I_XOR:     case I_NOR:
            case I_SLLV:    case I_SRLV:    case I_SRAV:
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_reg(scan_ptr, line);
                break;
            case I_MULT:    case I_MULTU:   case I_DIV:     case I_DIVU:
            case I_MOVE:
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_reg(scan_ptr, line);
                break;
            case I_MFHI:    case I_MTHI:    case I_MFLO:    case I_MTLO:
                scan_ptr = parse_reg(scan_ptr, line);
                break;
            case I_J:       case I_JAL:
                scan_ptr = parse_imm(scan_ptr, line);
                break;
            case I_JR:
                scan_ptr = parse_reg(scan_ptr, line);
                break;
            case I_JALR:
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_reg(scan_ptr, line);
                break;
            case I_BLTZ:    case I_BGEZ:    case I_BEQZ:    case I_BNEZ:
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_imm(scan_ptr, line);
                break;
            case I_SCALL:   case I_BREAK:   case I_ERET:    case I_SYNC:
            case I_NOP:
                break;
            case I_MFC0:    case I_MTC0:
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_imm(scan_ptr, line);
                break;
            case I_NOT:
                scan_ptr = parse_reg(scan_ptr, line);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = parse_reg(scan_ptr, line);
        };
        current_section->size += 4;
    }

    return scan_ptr;
}

char *
trans_reg(char *scan_ptr, unsigned int *reg)
{
    int _line;
    int *line = &_line;

    scan_ptr = pass_white(scan_ptr, line);

    char *token_start = scan_ptr;
    char *token_limit = pass_token(scan_ptr, line);

    *reg = lookup_symbol(
            reg_table,
            token_start,
            token_limit - token_start,
            string_hash(token_start, token_limit - token_start)
        )->offset;

    return token_limit;
}

char *
trans_imm(char *scan_ptr, unsigned int *imm)
{
    int _line;
    int *line = &_line;
    int neg = 0;

    scan_ptr = pass_white(scan_ptr, line);

    if (*scan_ptr == '%') {
        ++scan_ptr;
        if (strncmp(scan_ptr, "hi", 2) == 0) {
            scan_ptr = parse_chr(scan_ptr + 2, '(', line);
            scan_ptr = trans_imm(scan_ptr, imm);
            scan_ptr = parse_chr(scan_ptr, ')', line);

            *imm = (*imm >> 16) - ((*imm & 0x8000) ? 0xFFFF : 0);   // sign-ext
        }
        else {
            scan_ptr = parse_chr(scan_ptr + 2, '(', line);
            scan_ptr = trans_imm(scan_ptr, imm);
            scan_ptr = parse_chr(scan_ptr, ')', line);

            *imm &= 0xFFFF;
        }

        return scan_ptr;
    }
    if (*scan_ptr == '-') {
        ++scan_ptr;
        neg = 1;
    }

    *imm = 0;
    if (*scan_ptr == '0') {
        switch (*++scan_ptr) {
            case 'x':
                while (isxdigit(*++scan_ptr)) {
                    *imm <<= 4;
                    if (isdigit(*scan_ptr)) {
                        *imm += *scan_ptr - '0';
                    }
                    else if (*scan_ptr >= 'a' && *scan_ptr <= 'f') {
                        *imm += *scan_ptr - 'a' + 10;
                    }
                    else {
                        *imm += *scan_ptr - 'A' + 10;
                    }
                }
                break;
           case 'b':
                ++scan_ptr;
                while (*scan_ptr == '0' || *scan_ptr == '1') {
                    *imm = (*imm << 1) + (*scan_ptr++ - '0');
                }
                break;
           default:
                while (*scan_ptr >= '0' && *scan_ptr <= '7') {
                    *imm = (*imm << 3) + (*scan_ptr++ - '0');
                }
        }
    }
    else if (*scan_ptr > '0' && *scan_ptr <= '9') {
        while (isdigit(*scan_ptr)) {
            *imm = *imm * 10 + (*scan_ptr++ - '0');
        }
    }
    else {
        char *token_limit = pass_token(scan_ptr, line);
        struct Symbol *sym = lookup_symbol(
                label_table,
                scan_ptr,
                token_limit - scan_ptr,
                string_hash(scan_ptr, token_limit - scan_ptr)
            );

        if (!sym) {
            printf("ERROR: Unresolved symbol `%.*s'\n",
                    token_limit - scan_ptr,
                    scan_ptr
                );
            exit(-1);
        }

        *imm = sym->section->offset + sym->offset;
        return token_limit;
    }

    if (neg) {
        *imm = -*imm;
    }

    return scan_ptr;
}

char *
trans_directive(char *scan_ptr, int alignment)
{
    int _line;
    int *line = &_line;

    char *directive_limit = pass_non_white(scan_ptr, line);
    char *label_limit;

    if (strncmp(scan_ptr, ".4byte", directive_limit - scan_ptr) == 0) {
        scan_ptr = parse_chr(directive_limit, '(', line);
        scan_ptr = trans_imm(
                scan_ptr,
                &current_section->content[current_section->size >> 2]
            );
        scan_ptr = parse_chr(scan_ptr, ')', line);
        current_section->size += 4;
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".text", directive_limit - scan_ptr) == 0) {
        current_section = find_section(
                &section_table,
                ".text",
                strlen(".text"),
                string_hash(".text", strlen(".text"))
            );
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".data", directive_limit - scan_ptr) == 0) {
        current_section = find_section(
                &section_table,
                ".data",
                strlen(".data"),
                string_hash(".data", strlen(".data"))
            );
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".section", directive_limit - scan_ptr) == 0) {
        scan_ptr = pass_white(directive_limit, line);
        if (*scan_ptr == '\"') {
            ++scan_ptr;
            label_limit = strpbrk(scan_ptr, "\"");
        }
        else {
            label_limit = strpbrk(scan_ptr, ",\n\t");
        }
        current_section = find_section(
                &section_table,
                scan_ptr,
                label_limit - scan_ptr,
                string_hash(scan_ptr, label_limit - scan_ptr)
            );
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".ent", directive_limit - scan_ptr) == 0) {
        current_section->size = (current_section->size + alignment - 1) & -alignment;
        return strpbrk(scan_ptr, "\n");
    }
    else if (strncmp(scan_ptr, ".asciiz", directive_limit - scan_ptr) == 0 ||
             strncmp(scan_ptr, ".asciz", directive_limit - scan_ptr) == 0) {
        scan_ptr = parse_chr(directive_limit, '"', line);
        while (*scan_ptr != '"') {
            if (*scan_ptr == '\\') {
                switch (*++scan_ptr) {
                    case 'n':   *scan_ptr = '\n';   break;
                    case 't':   *scan_ptr = '\t';   break;
                    case 'v':   *scan_ptr = '\v';   break;
                }
            }
            *((char*)current_section->content + current_section->size++) = *scan_ptr++;
        }
        *((char*)current_section->content + current_section->size) = 0;
        current_section->size = (current_section->size + 4) & -4;
        ++scan_ptr;
    }
    else if (strncmp(scan_ptr, ".ascii", directive_limit - scan_ptr) == 0) {
        scan_ptr = parse_chr(directive_limit, '"', line);
        while (*scan_ptr != '"') {
            if (*scan_ptr == '\\') {
                ++scan_ptr;
            }
            *((char*)current_section->content + current_section->size++) = *scan_ptr++;
        }
        current_section->size = (current_section->size + 3) & -4;
        ++scan_ptr;
    }
    else if (
        strncmp(scan_ptr, ".abicalls",  directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".align",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".comm",      directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".end",       directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".file",      directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".fmask",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".frame",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".globl",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".ident",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".local",     directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".mask",      directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".nan",       directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".option",    directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".set",       directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".size",      directive_limit - scan_ptr) == 0 ||
        strncmp(scan_ptr, ".type",      directive_limit - scan_ptr) == 0 ||
        0
    ) {
        // known directive but do nothing
        return strpbrk(scan_ptr, "\n");     // jump to end of line
    }
    else {
        printf("WARNING: unknown directive `%.*s', line %d\n",
                directive_limit - scan_ptr,
                scan_ptr,
                *line
            );
        printf("%.*s\n",
                strpbrk(scan_ptr, "\n") - scan_ptr,
                scan_ptr
            );
        return strpbrk(scan_ptr, "\n");     // jump to end of line
    }

    return scan_ptr;
}

char *
trans_inst_label(char *scan_ptr)
{
    int _line;
    int *line = &_line;

    char *token_start = scan_ptr;
    char *token_limit = pass_token(scan_ptr, line);
    struct Symbol *sym;

    sym = lookup_symbol(
            inst_table,
            token_start,
            token_limit - token_start,
            string_hash(token_start, token_limit - token_start)
        );
    if (!sym) {
        return strpbrk(scan_ptr, ":") + 1;
    }

    unsigned int *inst = current_section->content + (current_section->size >> 2);
    unsigned int tmp;
    current_section->size += 4;

    scan_ptr = token_limit;
    *inst = 0;
    switch (sym->offset) {
        case I_LB:      case I_LBU:     case I_LH:      case I_LHU:
        case I_LW:      case I_LL:      case I_SB:      case I_SH:
        case I_SW:      case I_SC:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 16);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_imm(scan_ptr, &tmp);
                *inst |= (tmp & 0xFFFF);
                scan_ptr = parse_chr(scan_ptr, '(', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);
                scan_ptr = parse_chr(scan_ptr, ')', line);

                switch (sym->offset) {
                    case I_LB:      *inst |= 0x80000000;    break;
                    case I_LBU:     *inst |= 0x90000000;    break;
                    case I_LH:      *inst |= 0x84000000;    break;
                    case I_LHU:     *inst |= 0x94000000;    break;
                    case I_LW:      *inst |= 0x8C000000;    break;
                    case I_LL:      *inst |= 0xC0000000;    break;
                    case I_SB:      *inst |= 0xA0000000;    break;
                    case I_SH:      *inst |= 0xA4000000;    break;
                    case I_SW:      *inst |= 0xAC000000;    break;
                    case I_SC:      *inst |= 0xE0000000;    break;
                }
                break;
            }
        case I_ADDI:    case I_ADDIU:   case I_SLTI:    case I_SLTIU:
        case I_ANDI:    case I_ORI:     case I_XORI:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 16);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_imm(scan_ptr, &tmp);
                *inst |= (tmp & 0xFFFF);

                switch (sym->offset) {
                    case I_ADDI:    *inst |= 0x20000000;    break;
                    case I_ADDIU:   *inst |= 0x24000000;    break;
                    case I_SLTI:    *inst |= 0x28000000;    break;
                    case I_SLTIU:   *inst |= 0x2C000000;    break;
                    case I_ANDI:    *inst |= 0x30000000;    break;
                    case I_ORI:     *inst |= 0x34000000;    break;
                    case I_XORI:    *inst |= 0x38000000;    break;
                }
                break;
            }
        case I_SLL:     case I_SRL:     case I_SRA:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 11);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 16);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_imm(scan_ptr, &tmp);
                *inst |= (tmp & 0x1F) << 6;

                switch (sym->offset) {
                    case I_SLL:     *inst |= 0x00000000;    break;
                    case I_SRL:     *inst |= 0x00000002;    break;
                    case I_SRA:     *inst |= 0x00000003;    break;
                }
                break;
            }
        case I_BEQ:     case I_BNE:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 16);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_imm(scan_ptr, &tmp);
                tmp = (int)tmp - (int)(current_section->size + current_section->offset);
                *inst |= (tmp & 0x3FFFF) >> 2;

                switch (sym->offset) {
                    case I_BEQ:     *inst |= 0x10000000;    break;
                    case I_BNE:     *inst |= 0x14000000;    break;
                }
                break;
            }
        case I_LUI:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 16);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_imm(scan_ptr, &tmp);
                *inst |= (tmp & 0xFFFF);
                *inst |= 0x3C000000;
                break;
            }
        case I_ADD:     case I_ADDU:    case I_SUB:     case I_SUBU:
        case I_SLT:     case I_SLTU:    case I_AND:     case I_OR:
        case I_XOR:     case I_NOR:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 11);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 16);

                switch (sym->offset) {
                    case I_ADD:     *inst |= 0x00000020;    break;
                    case I_ADDU:    *inst |= 0x00000021;    break;
                    case I_SUB:     *inst |= 0x00000022;    break;
                    case I_SUBU:    *inst |= 0x00000023;    break;
                    case I_SLT:     *inst |= 0x0000002A;    break;
                    case I_SLTU:    *inst |= 0x0000002B;    break;
                    case I_AND:     *inst |= 0x00000024;    break;
                    case I_OR:      *inst |= 0x00000025;    break;
                    case I_XOR:     *inst |= 0x00000026;    break;
                    case I_NOR:     *inst |= 0x00000027;    break;
                }

                break;
            }
        case I_SLLV:    case I_SRLV:    case I_SRAV:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 11);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 16);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);

                switch (sym->offset) {
                    case I_SLLV:    *inst |= 0x00000004;    break;
                    case I_SRLV:    *inst |= 0x00000006;    break;
                    case I_SRAV:    *inst |= 0x00000007;    break;
                }
                break;
            }
        case I_MULT:    case I_MULTU:   case I_DIV:     case I_DIVU:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 16);

                switch (sym->offset) {
                    case I_MULT:    *inst |= 0x00000018;    break;
                    case I_MULTU:   *inst |= 0x00000019;    break;
                    /*
                    case I_DIV:     *inst |= 0x0000001A;    break;
                    case I_DIVU:    *inst |= 0x0000001B;    break;
                    */
                    default:
                        printf("ERROR: Division is not supported yet\n");
                        exit(-1);
                }
                break;
            }
        case I_MOVE:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 16);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);
                *inst |= 0x24000000;
                break;
            }
        case I_MFHI:    case I_MFLO:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 11);

                switch (sym->offset) {
                    case I_MFHI:    *inst |= 0x00000010;    break;
                    case I_MFLO:    *inst |= 0x00000012;    break;
                }
                break;
            }
        case I_MTHI:    case I_MTLO:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);

                switch (sym->offset) {
                    case I_MTHI:    *inst |= 0x00000011;    break;
                    case I_MTLO:    *inst |= 0x00000013;    break;
                }
                break;
            }
        case I_J:       case I_JAL:
            {
                scan_ptr = trans_imm(scan_ptr, &tmp);
                *inst |= (tmp & 0xFFFFFFF) >> 2;

                switch (sym->offset) {
                    case I_J:       *inst |= 0x08000000;    break;
                    case I_JAL:     *inst |= 0x0C000000;    break;
                }
                break;
            }
        case I_JR:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);
                *inst |= 0x00000008;
                break;
            }
        case I_JALR:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 11);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);
                *inst |= 0x00000009;
                break;
            }
        case I_BLTZ:    case I_BGEZ:    case I_BEQZ:    case I_BNEZ:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_imm(scan_ptr, &tmp);
                tmp = (int)tmp - (int)(current_section->size + current_section->offset);
                *inst |= (tmp & 0x3FFFF) >> 2;

                switch (sym->offset) {
                    case I_BLTZ:    *inst |= 0x04000000;    break;
                    case I_BGEZ:    *inst |= 0x04010000;    break;
                    case I_BEQZ:    *inst |= 0x10000000;    break;
                    case I_BNEZ:    *inst |= 0x14000000;    break;
                }
                break;
            }
        case I_SCALL:
            {
                *inst |= 0x0000000C;
                break;
            }
        case I_BREAK:
            {
                *inst |= 0x0000000A;
                break;
            }
        case I_ERET:
            {
                *inst |= 0x40000018;
                break;
            }
        case I_SYNC:
            {
                *inst |= 0x0000000F;
                break;
            }
        case I_NOP: break;
        case I_MFC0:    case I_MTC0:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 16);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_imm(scan_ptr, &tmp);
                *inst |= (tmp << 11);

                switch (sym->offset) {
                    case I_MFC0:    *inst |= 0x40000000;    break;
                    case I_MTC0:    *inst |= 0x40000004;    break;
                }
                break;
            }
        case I_NOT:
            {
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 11);
                scan_ptr = parse_chr(scan_ptr, ',', line);
                scan_ptr = trans_reg(scan_ptr, &tmp);
                *inst |= (tmp << 21);
                *inst |= 0x00000027;
                break;
            }
    }

    return scan_ptr;
}

int
main(int argc, char **argv)
{
    const char *output_path = NULL,
               *file_path = NULL,
               *map_path = NULL;
    char *src = NULL,
         *scan_ptr;
    size_t base = 0,
           limit = 0,
           total_size = 1,  // for terminating '\0'
           size, actual_read;
    int line = 0, alignment = 16;
    FILE *fp = NULL;

    --argc; ++argv;
    if (!argc) { print_help(); exit(-1); }

    while (argc && (*argv)[0] == '-') {
        switch ((*argv)[1]) {
            case 'o':
                output_path = *++argv; --argc;
                break;
            case 'O':
                base = strtoul(*++argv, NULL, 10); --argc;
                break;
            case 'a':
                alignment = strtod(*++argv, NULL); --argc;
                break;
            case 'l':
                limit = strtoul(*++argv, NULL, 10); --argc;
                break;
            case 'h':
                print_help();
                exit(0);
                break;
            case 'v':
                print_version();
                exit(0);
                break;
            case 'm':
                map_path = *++argv; --argc;
                break;
        }

        --argc; ++argv;
    }

    if (!output_path) { printf("No output file\n"); exit(-1); }

    // create section TEXT
    current_section = find_section(
            &section_table,
            ".text",
            strlen(".text"),
            string_hash(".text", strlen(".text"))
        );
    find_section(
            &section_table,
            ".rodata",
            strlen(".rodata"),
            string_hash(".rodata", strlen(".rodata"))
        );
    find_section(
            &section_table,
            ".data",
            strlen(".data"),
            string_hash(".data", strlen(".data"))
        );
    comm_section.name = ".comm";
    comm_section.length = 5;

    // parse insts
    src = (char*)INSTS_LITERIAL;
    line = I_LB;
    while ((scan_ptr = strpbrk(src, " "))) {
        find_symbol(
                &inst_table,
                src,
                scan_ptr - src,
                string_hash(src, scan_ptr - src)
            )->offset = line++;
        src = scan_ptr + 1;
    }

    // parse regs
    src = (char*)REG_LITERIAL_ALIAS;
    line = 0;
    while ((scan_ptr = strpbrk(src, " "))) {
        find_symbol(
                &reg_table,
                src,
                scan_ptr - src,
                string_hash(src, scan_ptr - src)
            )->offset = line++;
        src = scan_ptr + 1;
    }
    src = (char*)REG_LITERIAL_NUMBER;
    line = 0;
    while ((scan_ptr = strpbrk(src, " "))) {
        find_symbol(
                &reg_table,
                src,
                scan_ptr - src,
                string_hash(src, scan_ptr - src)
            )->offset = line++;
        src = scan_ptr + 1;
    }

    src = NULL;
    while (argc) {
        file_path = *argv;

        // openfile
        fp = fopen(file_path, "r");
        if (!fp) { printf("Cannot open file: %s\n", file_path); exit(-1); }

        // reallocation src buffer 
        total_size += (size = file_size(fp));
        src = realloc(src, total_size);
        scan_ptr = src + (total_size - size - 1);

        // read file & check size
        actual_read = fread(scan_ptr, 1, size, fp);
        if (actual_read != size) { printf("Read error: %d of %d char read\n", actual_read, size); exit(-1); }
        fclose(fp);
        scan_ptr[size] = 0;

        line = 1;
        while (*(scan_ptr = pass_white(scan_ptr, &line))) {
            // parse loop
            switch (*scan_ptr) {
                case '.':   scan_ptr = parse_directive(scan_ptr, &line, alignment);    break;
                case '#':   scan_ptr = strpbrk(scan_ptr, "\n");             break;
                default:    scan_ptr = parse_inst_label(scan_ptr, &line);   break;
            }
        }

        --argc; ++argv;
    }

    size_t offset = base;
    current_section = section_table;
    while (current_section) {
        offset = (offset + alignment - 1) & -alignment;
        current_section->offset = offset;
        offset += current_section->size;
        current_section->content = (unsigned int*)malloc(current_section->size);

        if (limit && current_section->size + current_section->offset - base >= limit) {
            printf("Limit error: when processing section %.*s, which requires up to %u byte\n",
                    current_section->length, current_section->name,
                    current_section->size + current_section->offset - base
                    );
            exit(-1);
        }

        current_section->size = 0;  // use as pointer
        current_section = current_section->next;
    }

    offset = (offset + alignment - 1) & -alignment;
    comm_section.offset = offset;

    current_section = find_section(
            &section_table,
            ".text",
            strlen(".text"),
            string_hash(".text", strlen(".text"))
        );

    scan_ptr = src;
    while (*(scan_ptr = pass_white(scan_ptr, &line))) {
        switch (*scan_ptr) {
            case '.':   scan_ptr = trans_directive(scan_ptr, alignment);   break;
            case '#':   scan_ptr = strpbrk(scan_ptr, "\n");     break;
            default:    scan_ptr = trans_inst_label(scan_ptr);  break;
        }
    }

    if (!(fp = fopen(output_path, "w"))) {
        printf("ERROR: Unable to open file `%s' to write\n", output_path);
        exit(-1);
    }
    offset = 0;
    current_section = section_table;
    while (current_section) {
        if (current_section->size) {
            printf("write section %.*s at offset %u, length %u\n", 
                    current_section->length,
                    current_section->name,
                    current_section->offset - base,
                    current_section->size
                );
            fseek(fp, current_section->offset - base, SEEK_SET);
            if (!fwrite(current_section->content, current_section->size, 1, fp)) {
                printf("ERROR: Unable to write file\n");
                printf("    when writing section %.*s\n",
                        current_section->length,
                        current_section->name
                    );
                exit(-1);
            }
            offset += current_section->size;
        }
        current_section = current_section->next;
    }
    if (comm_section.size) {
        printf("write section %.*s at offset %u, length %u\n", 
                comm_section.length,
                comm_section.name,
                comm_section.offset - base,
                comm_section.size
            );
        fseek(fp, comm_section.offset + comm_section.size - base - 1, SEEK_SET);
        if (EOF == fputc('\0', fp)) {
            printf("ERROR: Unable to write file\n");
            printf("    when writing section %.*s\n",
                    comm_section.length,
                    comm_section.name
                );
            exit(-1);
        }
    }
    fclose(fp);

    if (map_path) {
        if (!(fp = fopen(map_path, "w"))) {
            printf("ERROR: Unable to open map file `%s' to write\n", map_path);
            exit(-1);
        }

        fprintf(fp, "\tSECTION\tOFFSET\tABSOLUTE\n");
        while (label_table) {
            fprintf(fp, 
                    "%.*s\t%.*s\t%u\t%u\n",
                    label_table->length, label_table->name,
                    label_table->section->length, label_table->section->name,
                    label_table->offset,
                    label_table->offset + label_table->section->offset
                );
            label_table = label_table->next;
        }

        fclose(fp);
    }

    return 0;
}
