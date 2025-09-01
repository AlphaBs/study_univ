#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define PARSER_STATE_BUFFER_SIZE 128

// -------------------- for debug -------------------- //

bool flagVerbose = true;
bool flagRunStepByStep = true;

// -------------------- dynamic storage -------------------- //

typedef struct
{
    int capacity;
    int size;
    void **first;
} DYNAMIC_STORAGE;

DYNAMIC_STORAGE *newDynamicStorage()
{
    int initialCapacity = 512;

    DYNAMIC_STORAGE *storage = malloc(sizeof(DYNAMIC_STORAGE));
    storage->capacity = initialCapacity;
    storage->size = 0;
    storage->first = malloc(sizeof(void *) * initialCapacity);
    return storage;
}

void addDynamicStorage(DYNAMIC_STORAGE *storage, void *item)
{
    if (storage->size == storage->capacity)
    {
        int newArrayCapacity = storage->capacity * 2;
        void **newArray = realloc(storage->first, sizeof(void *) * newArrayCapacity);
        if (newArray == NULL)
        {
            perror("memory fail: addDynamicStorage");
            exit(1);
        }
        storage->first = newArray;
        storage->capacity = newArrayCapacity;
    }

    storage->first[storage->size] = item;
    storage->size++;
}

void *getDynamicStorage(DYNAMIC_STORAGE *storage, int index)
{
    if (index < 0 || index >= storage->size)
        return NULL;

    return storage->first[index];
}

void deleteDynamicStorage(DYNAMIC_STORAGE *storage)
{
    free(storage->first);
    free(storage);
}

void deleteDynamicStorageWithItems(DYNAMIC_STORAGE *storage)
{
    for (int i = 0; i < storage->size; i++)
    {
        free(storage->first[i]);
    }

    deleteDynamicStorage(storage);
}

// -------------------- INST 상수 -------------------- //

typedef enum
{
    INST_OPTYPE_UNKNOWN = -1,
    INST_OPTYPE_R,
    INST_OPTYPE_I,
    INST_OPTYPE_I_MEM,
    INST_OPTYPE_S,
    INST_OPTYPE_SB,
    INST_OPTYPE_UJ
} INST_OPTYPE;

#define INST_COUNT 24
typedef enum
{
    INST_OP_UNKNOWN = -1,
    INST_OP_EXIT,
    INST_OP_ADD,
    INST_OP_SUB,
    INST_OP_AND,
    INST_OP_OR,
    INST_OP_XOR,
    INST_OP_SLL,
    INST_OP_SRL,
    INST_OP_SRA,
    INST_OP_ADDI,
    INST_OP_ANDI,
    INST_OP_ORI,
    INST_OP_XORI,
    INST_OP_SLLI,
    INST_OP_SRLI,
    INST_OP_SRAI,
    INST_OP_LW,
    INST_OP_JALR,
    INST_OP_SW,
    INST_OP_BEQ,
    INST_OP_BNE,
    INST_OP_BGE,
    INST_OP_BLT,
    INST_OP_JAL,
} INST_OP;

typedef struct
{
    INST_OP op;
    char rd;
    char rs1;
    char rs2;
    int imm; // 12-bit or 20-bit
} INST;

typedef struct
{
    INST_OP op;
    INST_OPTYPE type;
    char str[8];
} INST_OP_MAP_ITEM;

const INST_OP_MAP_ITEM INST_OP_MAP[INST_COUNT] =
    {
        {INST_OP_EXIT, INST_OPTYPE_UNKNOWN, "EXIT"},

        {INST_OP_ADD, INST_OPTYPE_R, "ADD"},
        {INST_OP_SUB, INST_OPTYPE_R, "SUB"},
        {INST_OP_AND, INST_OPTYPE_R, "AND"},
        {INST_OP_OR, INST_OPTYPE_R, "OR"},
        {INST_OP_XOR, INST_OPTYPE_R, "XOR"},
        {INST_OP_SLL, INST_OPTYPE_R, "SLL"},
        {INST_OP_SRL, INST_OPTYPE_R, "SRL"},
        {INST_OP_SRA, INST_OPTYPE_R, "SRA"},

        {INST_OP_ADDI, INST_OPTYPE_I, "ADDI"},
        {INST_OP_ANDI, INST_OPTYPE_I, "ANDI"},
        {INST_OP_ORI, INST_OPTYPE_I, "ORI"},
        {INST_OP_XORI, INST_OPTYPE_I, "XORI"},
        {INST_OP_SLLI, INST_OPTYPE_I, "SLLI"},
        {INST_OP_SRLI, INST_OPTYPE_I, "SRLI"},
        {INST_OP_SRAI, INST_OPTYPE_I, "SRAI"},

        {INST_OP_LW, INST_OPTYPE_I_MEM, "LW"},
        {INST_OP_JALR, INST_OPTYPE_I_MEM, "JALR"},

        {INST_OP_SW, INST_OPTYPE_S, "SW"},

        {INST_OP_BEQ, INST_OPTYPE_SB, "BEQ"},
        {INST_OP_BNE, INST_OPTYPE_SB, "BNE"},
        {INST_OP_BGE, INST_OPTYPE_SB, "BGE"},
        {INST_OP_BLT, INST_OPTYPE_SB, "BLT"},

        {INST_OP_JAL, INST_OPTYPE_UJ, "JAL"}
    };

INST_OP parseInstOpString(char *op)
{
    for (int i = 0; i < INST_COUNT; i++)
    {
        if (strcasecmp(INST_OP_MAP[i].str, op) == 0)
            return INST_OP_MAP[i].op;
    }

    return INST_OP_UNKNOWN;
}

void convertInstOpToString(INST_OP op, char *buffer, int n)
{
    for (int i = 0; i < INST_COUNT; i++)
    {
        if (INST_OP_MAP[i].op == op)
        {
            snprintf(buffer, n, "%s", INST_OP_MAP[i].str);
            return;
        }
    }
}

INST_OPTYPE getInstOpType(INST_OP op)
{
    for (int i = 0; i < INST_COUNT; i++)
    {
        if (INST_OP_MAP[i].op == op)
            return INST_OP_MAP[i].type;
    }

    return INST_OPTYPE_UNKNOWN;
}

INST *createInst()
{
    return malloc(sizeof(INST));
}

void addInstStorage(DYNAMIC_STORAGE *storage, INST *inst)
{
    addDynamicStorage(storage, inst);
}

INST *getInstStorage(DYNAMIC_STORAGE *storage, int index)
{
    return (INST *)getDynamicStorage(storage, index);
}

void printInst(int pc, INST *inst)
{
    char op[8];
    convertInstOpToString(inst->op, op, sizeof(op));
    printf("%d: %s %d %d %d %d\n", pc, op, inst->rd, inst->rs1, inst->rs2, inst->imm);
}

// -------------------- 파일 스트림에서 INST 명령어로 파싱 -------------------- //

bool isEmptyChar(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

typedef enum
{
    PARSER_RESULT_SUCCESS = 1,

    PARSER_RESULT_UNEXPECTED_EOF,
    PARSER_RESULT_UNEXPECTED_NEWLINE,
    PARSER_RESULT_UNEXPECTED_CHAR,

    PARSER_RESULT_BUFFER_OVERFLOW,
    PARSER_RESULT_INVALID_REGISTER,
    PARSER_RESULT_IMM_OVERFLOW,
    PARSER_RESULT_UNKNOWN_OP,
    PARSER_RESULT_INVALID_LABEL,
    PARSER_RESULT_INVALID_STATE
} PARSER_RESULT;

const int PARSER_RESULT_FLAG_EOF          = 1<<8;
const int PARSER_RESULT_FLAG_EMPTY        = 1<<9;
const int PARSER_RESULT_FLAG_WITH_LABEL   = 1<<10;
const int PARSER_RESULT_FLAG_DEFINE_LABEL = 1<<11;

typedef struct
{
    int index;
    char buffer[PARSER_STATE_BUFFER_SIZE];
    bool complete;
} PARSER_STATE;

bool isSuccessfulParserResult(PARSER_RESULT result)
{
    int type = result & 0b11111111;
    return type == PARSER_RESULT_SUCCESS;
}

void updateParserState(PARSER_STATE *state, int index, bool complete)
{
    state->index = index;
    state->complete = complete;
}

void initParserState(PARSER_STATE *state)
{
    updateParserState(state, 0, false);
}

bool isAlphabet(char ch)
{
    if ('A' <= ch && ch <= 'Z')
        return true;
    if ('a' <= ch && ch <= 'z')
        return true;
    return false;
}

bool isNumeric(char ch)
{
    return '0' <= ch && ch <= '9';
}

bool checkLabelValidation(char *buffer, int length)
{
    if (length == 0)
        return false;

    if (!isAlphabet(buffer[0]))
        return false;

    for (int i = 1; i < length; i++)
    {
        if (!isAlphabet(buffer[i]) && !isNumeric(buffer[i]))
            return false;
    }

    return true;
}

// ch 를 buffer 에 추가, 선행, 후행 공백은 무시
PARSER_RESULT appendToParserBuffer(PARSER_STATE *state, char ch)
{
    if (isEmptyChar(ch) && state->index == 0) // 선행 공백 무시
        return PARSER_RESULT_SUCCESS;
    if (!isEmptyChar(ch) && state->complete) // 후행 공백 이후 다시 문자가 오는 경우, 오류처리
        return PARSER_RESULT_UNEXPECTED_CHAR;
    if (isEmptyChar(ch) && state->index > 0) // 문자 이후 공백 오는 경우, 후행 공백으로 무시
    {
        state->complete = true;
        return PARSER_RESULT_SUCCESS;
    }

    if (state->index > PARSER_STATE_BUFFER_SIZE - 1)
        return PARSER_RESULT_BUFFER_OVERFLOW;

    state->buffer[state->index] = ch;
    state->index++;
    return PARSER_RESULT_SUCCESS;
}

// until 만날때까지 읽기
PARSER_RESULT readUntilUnsafe(FILE *file, PARSER_STATE *state, int *ends, int endCount, int *last)
{
    while (true)
    {
        *last = fgetc(file);

        bool end = false;
        for (int i = 0; i < endCount; i++)
        {
            if (ends[i] == *last)
            {
                end = true;
                break;
            }
        }

        if (end)
        {
            state->buffer[state->index] = '\0';
            return PARSER_RESULT_SUCCESS;
        }

        if (*last == EOF)
        {
            state->buffer[state->index] = '\0';
            return PARSER_RESULT_UNEXPECTED_EOF | PARSER_RESULT_FLAG_EOF;
        }

        PARSER_RESULT result = appendToParserBuffer(state, *last);
        if (result != PARSER_RESULT_SUCCESS)
            return result;
    }
}

// until 만날때까지 읽기
PARSER_RESULT readUntil(FILE *file, PARSER_STATE *state, int until)
{
    initParserState(state);
    int last;
    int untils[2] = { until, '\n' };
    PARSER_RESULT result = readUntilUnsafe(file, state, untils, 2, &last);
    if (last == '\n')
        return PARSER_RESULT_UNEXPECTED_NEWLINE;
    return result;
}

// EOF 혹은 \n 만날때까지 읽기
PARSER_RESULT readToEnd(FILE *file, PARSER_STATE *state)
{
    initParserState(state);
    PARSER_RESULT result;
    while (true)
    {
        char ch = fgetc(file);
        if (ch == EOF || ch == '\n')
        {
            state->buffer[state->index] = '\0';
            result = PARSER_RESULT_SUCCESS;
            if (ch == EOF)
                result |= PARSER_RESULT_FLAG_EOF;
            return result;
        }

        result = appendToParserBuffer(state, ch);
        if (!isSuccessfulParserResult(result))
            return result;
    }
}

// 명령어 첫부분 OP 혹은 Label 읽기
PARSER_RESULT readOpOrLabel(FILE *file, PARSER_STATE *state, INST *inst)
{
    PARSER_RESULT result;
    while (true)
    {
        int last;
        int ends[] = {' ', ':', '\n', EOF};
        initParserState(state);
        result = readUntilUnsafe(file, state, ends, sizeof(ends) / sizeof(int), &last);
        if (!isSuccessfulParserResult(result))
            return result;

        if (last == ':')
        {
            if (checkLabelValidation(state->buffer, state->index))
            {
                PARSER_STATE tempResult;
                initParserState(&tempResult);
                readToEnd(file, &tempResult);

                if (tempResult.index == 0)
                    return PARSER_RESULT_SUCCESS | PARSER_RESULT_FLAG_DEFINE_LABEL;
                else
                    return PARSER_RESULT_UNEXPECTED_CHAR;
            }
            else
                return PARSER_RESULT_INVALID_LABEL;
        }
        else if (last == '\n' && state->index > 0)
        {
            inst->op = parseInstOpString(state->buffer);
            if (inst->op == INST_OP_EXIT)
                return result;
            else
                return PARSER_RESULT_UNEXPECTED_NEWLINE;
        }
        else if (last == ' ' && state->index > 0)
        {
            inst->op = parseInstOpString(state->buffer);
            if (inst->op == INST_OP_UNKNOWN)
                return PARSER_RESULT_UNKNOWN_OP;
            if (inst->op == INST_OP_EXIT)
            {
                PARSER_STATE tempState;
                initParserState(&tempState);
                PARSER_RESULT tempResult = readToEnd(file, &tempState);
                if (!isSuccessfulParserResult(tempResult))
                    return tempResult;
                if (tempState.index > 0)
                    return PARSER_RESULT_UNEXPECTED_CHAR;
            }
            return result;
        }
        else if (last == EOF)
        {
            if (state->index == 0)
                return PARSER_RESULT_SUCCESS | PARSER_RESULT_FLAG_EMPTY | PARSER_RESULT_FLAG_EOF;
            else
                return PARSER_RESULT_UNEXPECTED_EOF | PARSER_RESULT_FLAG_EOF;
        }
    }
}

// x0 ~ x31 레지스터 파싱
PARSER_RESULT parseRegister(char *buffer, char *reg)
{
    if (strlen(buffer) < 2 || !(buffer[0] == 'x' || buffer[0] == 'X'))
        return PARSER_RESULT_INVALID_REGISTER;

    errno = 0;
    char *endptr;
    long value = strtol(&buffer[1], &endptr, 10);

    if (errno != 0 || *endptr != '\0' || value < 0 || value > 31)
        return PARSER_RESULT_INVALID_REGISTER;

    *reg = (int)value;
    return PARSER_RESULT_SUCCESS;
}

long getMaximumOfBit(int bit)
{
    return (long)((1UL << (bit - 1)) - 1);
}

long getMinimumOfBit(int bit)
{
    return -(long)(1UL << (bit - 1));
}

// 길이가 bit 인 상수 파싱
PARSER_RESULT parseImm(char *buffer, int *imm, int bit)
{
    char *endptr;
    long value;

    errno = 0;
    value = strtol(buffer, &endptr, 0);

    if (errno != 0 || *endptr != '\0')
        return PARSER_RESULT_IMM_OVERFLOW;

    if (value < getMinimumOfBit(bit) || value > getMaximumOfBit(bit))
        return PARSER_RESULT_IMM_OVERFLOW;

    *imm = (int)value;
    return PARSER_RESULT_SUCCESS;
}

// 완전한 명령어 한줄 파싱
PARSER_RESULT parseInst(FILE *file, PARSER_STATE *state, INST *inst)
{
    PARSER_RESULT readResult;
    PARSER_RESULT parseResult;
    readResult = readOpOrLabel(file, state, inst);

    if (readResult & PARSER_RESULT_FLAG_EOF)
        return readResult;

    if (readResult & PARSER_RESULT_FLAG_EMPTY)
        return readResult;

    if (readResult & PARSER_RESULT_FLAG_DEFINE_LABEL)
        return readResult;

    if (!isSuccessfulParserResult(readResult))
        return readResult;

    INST_OPTYPE optype = getInstOpType(inst->op);
    switch (optype)
    {
    case INST_OPTYPE_R:
        readResult = readUntil(file, state, ',');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rd);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readUntil(file, state, ',');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rs1);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readToEnd(file, state);
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rs2);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        return readResult;

    case INST_OPTYPE_I:
        readResult = readUntil(file, state, ',');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rd);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readUntil(file, state, ',');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rs1);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readToEnd(file, state);
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseImm(state->buffer, &inst->imm, 12);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        return readResult;

    case INST_OPTYPE_I_MEM:
        readResult = readUntil(file, state, ',');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rd);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readUntil(file, state, '(');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseImm(state->buffer, &inst->imm, 12);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readUntil(file, state, ')');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rs1);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readToEnd(file, state);
        if (!isSuccessfulParserResult(parseResult))
            return readResult;

        if (state->index > 0)
            return PARSER_RESULT_UNEXPECTED_CHAR;

        return readResult;

    case INST_OPTYPE_S:
        readResult = readUntil(file, state, ',');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rs2);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readUntil(file, state, '(');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseImm(state->buffer, &inst->imm, 12);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readUntil(file, state, ')');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rs1);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readToEnd(file, state);
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        if (state->index > 0)
            return PARSER_RESULT_UNEXPECTED_CHAR;

        return readResult;

    case INST_OPTYPE_SB:
        readResult = readUntil(file, state, ',');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rs1);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readUntil(file, state, ',');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rs2);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readToEnd(file, state);
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        if (checkLabelValidation(state->buffer, state->index))
            return readResult | PARSER_RESULT_FLAG_WITH_LABEL;

        parseResult = parseImm(state->buffer, &inst->imm, 13); // 명령어 정렬, 마지막 비트 무시
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        return readResult;

    case INST_OPTYPE_UJ:
        readResult = readUntil(file, state, ',');
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        parseResult = parseRegister(state->buffer, &inst->rd);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        readResult = readToEnd(file, state);
        if (!isSuccessfulParserResult(readResult))
            return readResult;

        if (checkLabelValidation(state->buffer, state->index))
            return PARSER_RESULT_SUCCESS | PARSER_RESULT_FLAG_WITH_LABEL;

        parseResult = parseImm(state->buffer, &inst->imm, 21);
        if (!isSuccessfulParserResult(parseResult))
            return parseResult;

        return readResult;

    case INST_OPTYPE_UNKNOWN:
        if (inst->op == INST_OP_EXIT)
            return readResult;
        return PARSER_RESULT_UNKNOWN_OP;
    }
}

// -------------------- INST Encoder -------------------- //

typedef uint32_t ENCODED_INST;

// imm 에서 start 부터 count 개의 비트를 가져오기
int getBits(uint32_t imm, int start, int count)
{
    return (imm >> start) & ((1 << count) - 1);
}

// value 를 from 에다가 삽입한 값을 반환, start 부터 count 개 만큼
ENCODED_INST encodeBits(ENCODED_INST from, int start, int count, ENCODED_INST value)
{
    // value 의 하위 count 비트만 가져오기
    ENCODED_INST mask = (1 << count) - 1;
    value &= mask;

    // value 를 start 만큼 이동
    value <<= start;

    // from 에서 기존에 있던 비트를 0으로 초기화
    ENCODED_INST clearMask = ~(mask << start);
    from &= clearMask;

    // value 를 from 에 삽입
    return from | value;
}

ENCODED_INST encodeInstR(INST *inst, int opcode, int funct3, int funct7)
{
    ENCODED_INST encoded = 0;

    encoded = encodeBits(encoded, 0, 7, opcode);
    encoded = encodeBits(encoded, 7, 5, inst->rd);
    encoded = encodeBits(encoded, 12, 3, funct3);
    encoded = encodeBits(encoded, 15, 5, inst->rs1);
    encoded = encodeBits(encoded, 20, 5, inst->rs2);
    encoded = encodeBits(encoded, 25, 7, funct7);

    return encoded;
}

ENCODED_INST encodeInstI(INST *inst, int opcode, int funct3)
{
    ENCODED_INST encoded = 0;

    encoded = encodeBits(encoded, 0, 7, opcode);
    encoded = encodeBits(encoded, 7, 5, inst->rd);
    encoded = encodeBits(encoded, 12, 3, funct3);
    encoded = encodeBits(encoded, 15, 5, inst->rs1);
    encoded = encodeBits(encoded, 20, 12, inst->imm);

    return encoded;
}

ENCODED_INST encodeInstShamt(INST *inst, int opcode, int funct3, int imm)
{
    ENCODED_INST encoded = 0;

    encoded = encodeBits(encoded, 0, 7, opcode);
    encoded = encodeBits(encoded, 7, 5, inst->rd);
    encoded = encodeBits(encoded, 12, 3, funct3);
    encoded = encodeBits(encoded, 15, 5, inst->rs1);
    encoded = encodeBits(encoded, 20, 5, inst->imm);
    encoded = encodeBits(encoded, 25, 7, imm);

    return encoded;
}

ENCODED_INST encodeInstS(INST *inst, int opcode, int funct3)
{
    ENCODED_INST encoded = 0;

    encoded = encodeBits(encoded, 0, 7, opcode);
    encoded = encodeBits(encoded, 7, 5, getBits(inst->imm, 0, 5));
    encoded = encodeBits(encoded, 12, 3, funct3);
    encoded = encodeBits(encoded, 15, 5, inst->rs1);
    encoded = encodeBits(encoded, 20, 5, inst->rs2);
    encoded = encodeBits(encoded, 25, 7, getBits(inst->imm, 5, 7));

    return encoded;
}

ENCODED_INST encodeInstSB(INST *inst, int opcode, int funct3)
{
    ENCODED_INST encoded = 0;

    encoded = encodeBits(encoded, 0, 7, opcode);
    encoded = encodeBits(encoded, 7, 1, getBits(inst->imm, 11, 1));
    encoded = encodeBits(encoded, 8, 4, getBits(inst->imm, 1, 4));
    encoded = encodeBits(encoded, 12, 3, funct3);
    encoded = encodeBits(encoded, 15, 5, inst->rs1);
    encoded = encodeBits(encoded, 20, 5, inst->rs2);
    encoded = encodeBits(encoded, 25, 6, getBits(inst->imm, 5, 6));
    encoded = encodeBits(encoded, 31, 1, getBits(inst->imm, 12, 1));

    return encoded;
}

ENCODED_INST encodeInstUJ(INST *inst, int opcode)
{
    ENCODED_INST encoded = 0;

    encoded = encodeBits(encoded, 0, 7, opcode);
    encoded = encodeBits(encoded, 7, 5, inst->rd);
    encoded = encodeBits(encoded, 12, 8, getBits(inst->imm, 12, 8));
    encoded = encodeBits(encoded, 20, 1, getBits(inst->imm, 11, 1));
    encoded = encodeBits(encoded, 21, 10, getBits(inst->imm, 1, 10));
    encoded = encodeBits(encoded, 31, 1, getBits(inst->imm, 20, 1));

    return encoded;
}

ENCODED_INST encodeInst(INST *inst)
{
    switch (inst->op)
    {
    case INST_OP_EXIT:
        return 0xffffffff;

    case INST_OP_ADD:
        return encodeInstR(inst, 0b0110011, 0b000, 0b0000000);
    case INST_OP_SUB:
        return encodeInstR(inst, 0b0110011, 0b000, 0b0100000);
    case INST_OP_AND:
        return encodeInstR(inst, 0b0110011, 0b111, 0b0000000);
    case INST_OP_OR:
        return encodeInstR(inst, 0b0110011, 0b110, 0b0000000);
    case INST_OP_XOR:
        return encodeInstR(inst, 0b0110011, 0b100, 0b0000000);
    case INST_OP_SLL:
        return encodeInstR(inst, 0b0110011, 0b001, 0b0000000);
    case INST_OP_SRL:
        return encodeInstR(inst, 0b0110011, 0b101, 0b0000000);
    case INST_OP_SRA:
        return encodeInstR(inst, 0b0110011, 0b101, 0b0100000);

    case INST_OP_ADDI:
        return encodeInstI(inst, 0b0010011, 0b000);
    case INST_OP_ANDI:
        return encodeInstI(inst, 0b0010011, 0b111);
    case INST_OP_ORI:
        return encodeInstI(inst, 0b0010011, 0b110);
    case INST_OP_XORI:
        return encodeInstI(inst, 0b0010011, 0b100);
    case INST_OP_SLLI:
        return encodeInstShamt(inst, 0b0010011, 0b001, 0b0000000);
    case INST_OP_SRLI:
        return encodeInstShamt(inst, 0b0010011, 0b101, 0b0000000);
    case INST_OP_SRAI:
        return encodeInstShamt(inst, 0b0010011, 0b101, 0b0100000);
    case INST_OP_LW:
        return encodeInstI(inst, 0b0000011, 0b010);
    case INST_OP_JALR:
        return encodeInstI(inst, 0b1100111, 0b000);

    case INST_OP_SW:
        return encodeInstS(inst, 0b0100011, 0b010);

    case INST_OP_BEQ:
        return encodeInstSB(inst, 0b1100011, 0b000);
    case INST_OP_BGE:
        return encodeInstSB(inst, 0b1100011, 0b101);
    case INST_OP_BLT:
        return encodeInstSB(inst, 0b1100011, 0b100);

    case INST_OP_JAL:
        return encodeInstUJ(inst, 0b1101111);

    default:
        perror("encodeInst: unknown inst type");
        exit(1);
    }
}

// -------------------- Label Map -------------------- //

// 중복된 이름의 LABEL 가 있다면 마지막으로 정의된 LABEL 를 사용합니다.
// LABEL 의 이름의 첫 글자에는 알파벳 대소문자만 올 수 있습니다. `[A-Za-z]`
// LABEL 의 이름의 나머지 글자에는 알파벳 대소문자와 숫자만 올 수 있습니다. `[A-Za-z0-9]`
// LABEL 의 이름은 최대 127글자입니다.

typedef struct
{
    char buffer[PARSER_STATE_BUFFER_SIZE];
    int pc;
} LABEL;

void addLabelStorage(DYNAMIC_STORAGE *storage, LABEL *label)
{
    addDynamicStorage(storage, label);
}

LABEL *getLabelStorage(DYNAMIC_STORAGE *storage, int index)
{
    return (LABEL *)getDynamicStorage(storage, index);
}

LABEL *findLabelByName(DYNAMIC_STORAGE *storage, char *name)
{
    for (int i = 0; i < storage->size; i++)
    {
        LABEL *item = getLabelStorage(storage, i);
        if (strncasecmp(item->buffer, name, PARSER_STATE_BUFFER_SIZE) == 0)
            return item;
    }

    return NULL;
}

LABEL *findLabelByPos(DYNAMIC_STORAGE *storage, int pc)
{
    for (int i = 0; i < storage->size; i++)
    {
        LABEL *item = getLabelStorage(storage, i);
        if (item->pc == pc)
            return item;
    }

    return NULL;
}

LABEL *addNewLabelStorage(DYNAMIC_STORAGE *storage, char *name, int pos)
{
    LABEL *label = malloc(sizeof(LABEL));
    label->pc = pos;
    snprintf(label->buffer, PARSER_STATE_BUFFER_SIZE, "%s", name);
    addDynamicStorage(storage, label);
    return label;
}

// -------------------- Emulator -------------------- //

const int PROCESS_REGISTER_COUNT = 32;
typedef uint64_t PROCESS_REGISTER;
typedef int64_t PROCESS_REGISTER_SIGNED;
typedef uint8_t PROCESS_MEM;

typedef struct
{
    PROCESS_REGISTER *reg;
    PROCESS_REGISTER pc;
    int memSize;
    PROCESS_MEM *mem;
} PROCESS;

PROCESS *newProcess()
{
    PROCESS *process = malloc(sizeof(PROCESS));

    process->reg = malloc(sizeof(PROCESS_REGISTER) * PROCESS_REGISTER_COUNT);
    for (int i = 0; i < PROCESS_REGISTER_COUNT; i++)
    {
        process->reg[i] = 0;
    }

    // x1, x2, x3, x4, x5, x6 register의 초기값은 각각 1, 2, 3, 4, 5, 6으로 가정하고, 
    // 나머지 register들의 초기값은 0으로 가정함
    process->reg[1] = 1;
    process->reg[2] = 2;
    process->reg[3] = 3;
    process->reg[4] = 4;
    process->reg[5] = 5;
    process->reg[6] = 6;

    process->memSize = 1024 * 1024; // 1MB
    process->mem = malloc(sizeof(PROCESS_MEM) * process->memSize);
    process->pc = 1000;

    return process;
}

PROCESS_REGISTER getProcessReg(PROCESS *process, int x)
{
    if (x == 0)
        return 0;
    if (x < 0 || x >= PROCESS_REGISTER_COUNT)
    {
        perror("invalid register");
        exit(1);
    }
    return process->reg[x];
}

void setProcessReg(PROCESS *process, int x, PROCESS_REGISTER value)
{
    if (x == 0)
        return;
    if (x < 0 || x >= PROCESS_REGISTER_COUNT)
    {
        perror("invalid register");
        exit(1);
    }
    process->reg[x] = value;
}

PROCESS_MEM getProcessMem(PROCESS *process, PROCESS_REGISTER addr)
{
    if (addr >= process->memSize)
        return 0; // for now just ignore memory fault
    return process->mem[addr];
}

void setProcessMem(PROCESS *process, PROCESS_REGISTER addr, PROCESS_MEM value)
{
    if (addr >= process->memSize)
        return; // for now just ignore memory fault
    process->mem[addr] = value;
}

int32_t getProcessMem32(PROCESS *process, PROCESS_REGISTER addr)
{
    return (int32_t)getProcessMem(process, addr + 0) << 0 |
           (int32_t)getProcessMem(process, addr + 1) << 8 |
           (int32_t)getProcessMem(process, addr + 2) << 16 |
           (int32_t)getProcessMem(process, addr + 3) << 24;
}

void setProcessMem32(PROCESS *process, PROCESS_REGISTER addr, int32_t value)
{
    setProcessMem(process, addr + 0, (PROCESS_MEM)(value >> 0));
    setProcessMem(process, addr + 1, (PROCESS_MEM)(value >> 8));
    setProcessMem(process, addr + 2, (PROCESS_MEM)(value >> 16));
    setProcessMem(process, addr + 3, (PROCESS_MEM)(value >> 24));
}

void deleteProcess(PROCESS *process)
{
    free(process->reg);
    free(process->mem);
    free(process);
}

void printProcess(PROCESS *process)
{
    printf("PC: %lld\n", process->pc);
    for (int i = 0; i < PROCESS_REGISTER_COUNT; i++)
    {
        printf("x%d %lld\n", i, getProcessReg(process, i));
    }
}

PROCESS_REGISTER extendSign(int imm, int bits)
{
    PROCESS_REGISTER sign = 0xFFFFFFFFFFFFFFFF;
    PROCESS_REGISTER result = imm;
    if (imm & (1 << (bits - 1)))
        result |= (sign << bits);
    return result;
}

PROCESS_REGISTER extendZero(int imm)
{
    return (PROCESS_REGISTER)imm;
}

bool executeInstProcess(PROCESS *process, INST *inst)
{
    PROCESS_REGISTER result;
    process->pc += 4;

    switch (inst->op)
    {
    case INST_OP_EXIT:
        return false;

    case INST_OP_ADD:
        result = (PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs1) + (PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs2);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_SUB:
        result = (PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs1) - (PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs2);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_AND:
        result = getProcessReg(process, inst->rs1) & getProcessReg(process, inst->rs2);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_OR:
        result = getProcessReg(process, inst->rs1) | getProcessReg(process, inst->rs2);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_XOR:
        result = getProcessReg(process, inst->rs1) ^ getProcessReg(process, inst->rs2);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_SLL:
        result = getProcessReg(process, inst->rs1) << getProcessReg(process, inst->rs2);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_SRL:
        result = getProcessReg(process, inst->rs1) >> getProcessReg(process, inst->rs2);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_SRA:
        result = (PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs1) >> getProcessReg(process, inst->rs2);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_ADDI:
        result = (PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs1) + extendSign(inst->imm, 12);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_ANDI:
        result = getProcessReg(process, inst->rs1) & extendSign(inst->imm, 12);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_ORI:
        result = getProcessReg(process, inst->rs1) | extendSign(inst->imm, 12);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_XORI:
        result = getProcessReg(process, inst->rs1) ^ extendSign(inst->imm, 12);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_SLLI:
        result = getProcessReg(process, inst->rs1) << extendSign(inst->imm, 12);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_SRLI:
        result = getProcessReg(process, inst->rs1) >> extendSign(inst->imm, 12);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_SRAI:
        result = (PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs1) >> extendSign(inst->imm, 12);
        setProcessReg(process, inst->rd, result);
        return true;

    case INST_OP_LW:
        result = getProcessReg(process, inst->rs1) + extendSign(inst->imm, 12);
        int32_t lwMem = getProcessMem32(process, result);
        setProcessReg(process, inst->rd, (PROCESS_REGISTER)lwMem);
        return true;

    case INST_OP_JALR:
        setProcessReg(process, inst->rd, process->pc);
        process->pc = (getProcessReg(process, inst->rs1) + extendSign(inst->imm, 12)) & ~1;
        return true;

    case INST_OP_SW:
        result = getProcessReg(process, inst->rs1) + extendSign(inst->imm, 12);
        setProcessMem32(process, result, getProcessReg(process, inst->rs2));
        return true;

    case INST_OP_BEQ:
        if (getProcessReg(process, inst->rs1) == getProcessReg(process, inst->rs2))
            process->pc = process->pc - 4 + extendSign(inst->imm, 13);
        return true;

    case INST_OP_BNE:
        if (getProcessReg(process, inst->rs1) != getProcessReg(process, inst->rs2))
            process->pc = process->pc - 4 + extendSign(inst->imm, 13);
        return true;

    case INST_OP_BGE:
        if ((PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs1) >= (PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs2))
            process->pc = process->pc - 4 + extendSign(inst->imm, 13);
        return true;

    case INST_OP_BLT:
        if ((PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs1) < (PROCESS_REGISTER_SIGNED)getProcessReg(process, inst->rs2))
            process->pc = process->pc - 4 + extendSign(inst->imm, 13);
        return true;

    case INST_OP_JAL:
        setProcessReg(process, inst->rd, process->pc);
        process->pc = process->pc - 4 + extendSign(inst->imm, 21);
        return true;
    }
}

INST *getInstStorageByPc(DYNAMIC_STORAGE *storage, int pc)
{
    int index = (pc - 1000) / 4;
    return getInstStorage(storage, index);
}

void runProcess(PROCESS *process, DYNAMIC_STORAGE *instStorage, FILE *log)
{
    if (flagVerbose)
        printf("Running instructions...\n");

    while (true)
    {
        INST *inst;
        inst = getInstStorageByPc(instStorage, process->pc);
        if (inst == NULL)
            return;

        if (flagVerbose)
            printInst(process->pc, inst);

        if (flagRunStepByStep)
            getchar();

        fprintf(log, "%lld\n", process->pc);
        bool continueResult = executeInstProcess(process, inst);
        if (!continueResult)
            return;
    }
}

// -------------------- TESTCASE ---------------------- //

void testInstR(INST_OP op, PROCESS_REGISTER rs1, PROCESS_REGISTER rs2, PROCESS_REGISTER expected)
{
    INST inst = {.op = op, .rs1 = 1, .rs2 = 2, .rd = 3};
    PROCESS *process = newProcess();
    setProcessReg(process, 1, rs1);
    setProcessReg(process, 2, rs2);
    executeInstProcess(process, &inst);
    PROCESS_REGISTER actual = getProcessReg(process, 3);

    char buffer[128];
    convertInstOpToString(op, buffer, sizeof(buffer));
    printf("%s 0x%llX 0x%llX = 0x%llX (0x%llX)\n", buffer, rs1, rs2, actual, expected);

    deleteProcess(process);

    if (expected != actual)
        exit(1);
}

void testInstI(INST_OP op, PROCESS_REGISTER rs1, int imm, PROCESS_REGISTER expected)
{
    INST inst = {.op = op, .rs1 = 1, .rs2 = 2, .rd = 3, .imm = imm};
    PROCESS *process = newProcess();
    setProcessReg(process, 1, rs1);
    executeInstProcess(process, &inst);
    PROCESS_REGISTER actual = getProcessReg(process, 3);

    char buffer[128];
    convertInstOpToString(op, buffer, sizeof(buffer));
    printf("%s 0x%llX 0x%llX = 0x%llX (0x%llX)\n", buffer, rs1, imm, actual, expected);

    deleteProcess(process);

    if (expected != actual)
        exit(1);
}

void testInstLW(PROCESS *process, PROCESS_REGISTER base, int offset, PROCESS_REGISTER expected)
{
    INST inst = {
        .op = INST_OP_LW,
        .rs1 = 30,
        .rs2 = 31,
        .rd = 31,
        .imm = offset,
    };
    setProcessReg(process, 30, base);
    executeInstProcess(process, &inst);
    PROCESS_REGISTER actual = getProcessReg(process, 31);

    char buffer[128];
    convertInstOpToString(inst.op, buffer, sizeof(buffer));
    printf("%s MEM[0x%llX + %lld] : 0x%llX (0x%llX)\n", buffer, base, offset, actual, expected);

    if (actual != expected)
        exit(1);
}

void testInstSW(PROCESS *process, PROCESS_REGISTER base, int offset, PROCESS_REGISTER value)
{
    INST inst = {
        .op = INST_OP_SW,
        .rs1 = 30,
        .rs2 = 31,
        .rd = 3,
        .imm = offset,
    };
    setProcessReg(process, 30, base);
    setProcessReg(process, 31, value);
    executeInstProcess(process, &inst);

    char buffer[128];
    convertInstOpToString(inst.op, buffer, sizeof(buffer));
    printf("%s MEM[0x%llX + %lld] <- 0x%llX\n", buffer, base, offset, value);
}

void testMem(PROCESS *process, PROCESS_REGISTER addr, int expected)
{
    PROCESS_MEM actual = getProcessMem(process, addr);
    printf("0x%llX : 0x%llX (0x%llX)\n", addr, actual, expected);
}

void testInstSB(INST_OP op, PROCESS_REGISTER rs1, PROCESS_REGISTER rs2, bool expected)
{
    PROCESS *process = newProcess();
    INST inst = {.op = op, .rs1 = 1, .rs2 = 2, .imm = 1000};
    setProcessReg(process, 1, rs1);
    setProcessReg(process, 2, rs2);
    executeInstProcess(process, &inst);

    bool isTaken = process->pc == 2000;

    char buffer[128];
    convertInstOpToString(inst.op, buffer, sizeof(buffer));
    printf("%s 0x%llX 0x%llX %s\n", buffer, rs1, rs2, isTaken ? "O" : "X");

    deleteProcess(process);

    if (isTaken != expected)
        exit(1);
}

int testInstSBJump(int imm, int expected)
{
    PROCESS *process = newProcess();
    INST inst = {.op = INST_OP_BEQ, .rs1 = 1, .rs2 = 2, .imm = imm};
    setProcessReg(process, 1, 0);
    setProcessReg(process, 2, 0);
    executeInstProcess(process, &inst);
    PROCESS_REGISTER actual = process->pc;

    printf("BEQ JUMP %d (%d)\n", actual, expected);
    deleteProcess(process);

    if (expected != actual)
        exit(1);
}

void testJALR(PROCESS_REGISTER base, int offset, PROCESS_REGISTER expected)
{
    PROCESS *process = newProcess();
    INST inst = {.op = INST_OP_JALR, .rd = 1, .rs1 = 2, .imm = offset};
    setProcessReg(process, 2, base);
    executeInstProcess(process, &inst);
    PROCESS_REGISTER rd = getProcessReg(process, 1);
    PROCESS_REGISTER pc = process->pc;
    printf("JALR %d (%d)\n", pc, expected);
    deleteProcess(process);
    if (rd != 1000 + 4)
        exit(1);
    if (pc != expected)
        exit(1);
}

void testJAL(int offset, PROCESS_REGISTER expected)
{
    PROCESS *process = newProcess();
    INST inst = {.op = INST_OP_JAL, .rd = 1, .imm = offset};
    executeInstProcess(process, &inst);
    PROCESS_REGISTER actual = process->pc;
    PROCESS_REGISTER rd = getProcessReg(process, 1);
    printf("JAL %d (%d)\n", actual, expected);
    deleteProcess(process);
    if (rd != 1004)
        exit(1);
    if (actual != expected)
        exit(1);
}

void testALU()
{
    // ADD
    testInstR(INST_OP_ADD, 1, 2, 3);
    testInstR(INST_OP_ADD, 1, -1, 0);
    testInstR(INST_OP_ADD, -10, 4, -6);
    testInstR(INST_OP_ADD, 0x7FFFFFFFFFFFFFFF, 1, 0x8000000000000000);

    // ADDI
    testInstI(INST_OP_ADDI, 1, 2, 3);
    testInstI(INST_OP_ADDI, 1, -1, 0);
    testInstI(INST_OP_ADDI, -10, 4, -6);
    testInstI(INST_OP_ADDI, 0x7FFFFFFFFFFFFFFF, 1, 0x8000000000000000);

    // SUB
    testInstR(INST_OP_SUB, 1, 2, -1);
    testInstR(INST_OP_SUB, 1, -1, 2);
    testInstR(INST_OP_SUB, -10, 6, -16);
    testInstR(INST_OP_SUB, 0, 1, 0xFFFFFFFFFFFFFFFF);

    // ADD
    testInstR(INST_OP_AND, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x0000000000000000);
    testInstR(INST_OP_AND, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000);
    testInstR(INST_OP_AND, 0b11001100, 0b10101010, 0b10001000);

    // ANDI
    testInstI(INST_OP_ANDI, 0xAAAAAAAAAAAAAAAA, 0b111111111111, 0xAAAAAAAAAAAAAAAA);
    testInstI(INST_OP_ANDI, 0xFFFFFFFFFFFFFFFF, 0b111111111111, 0xFFFFFFFFFFFFFFFF);
    testInstI(INST_OP_ANDI, 0xFFFFFFFFFFFFFFFF, 0b011111111111, 0b011111111111);
    testInstI(INST_OP_ANDI, 0b11001100, 0b10101010, 0b10001000);

    // OR
    testInstR(INST_OP_OR, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0xFFFFFFFFFFFFFFFF);
    testInstR(INST_OP_OR, 0x8000000000000000, 0x0000000000000000, 0x8000000000000000);
    testInstR(INST_OP_OR, 0b11001100, 0b10101010, 0b11101110);

    // ORI
    testInstI(INST_OP_ORI, 0xAAAAAAAAAAAAAAAA, 0b111111111111, 0xFFFFFFFFFFFFFFFF);
    testInstI(INST_OP_ORI, 0x0, 0b111111111111, 0xFFFFFFFFFFFFFFFF);
    testInstI(INST_OP_ORI, 0x0, 0b011111111111, 0b011111111111);
    testInstI(INST_OP_ORI, 0b11001100, 0b10101010, 0b11101110);

    // XOR
    testInstR(INST_OP_XOR, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0xFFFFFFFFFFFFFFFF);
    testInstR(INST_OP_XOR, 0, 0, 0);
    testInstR(INST_OP_XOR, 0b11001100, 0b10101010, 0b01100110);

    // XORI
    testInstI(INST_OP_XORI, 0xAAAAAAAAAAAAAAAA, 0b010101010101, 0xAAAAAAAAAAAAAFFF);
    testInstI(INST_OP_XORI, 0xFFFFFFFFFFFFFFFF, 0b111111111111, 0);
    testInstI(INST_OP_XORI, 0, 0, 0);
    testInstI(INST_OP_XORI, 0b11001100, 0b10101010, 0b01100110);

    // SLL
    testInstR(INST_OP_SLL, 0x0123456789ABCDEF, 8, 0x23456789ABCDEF00);
    testInstR(INST_OP_SLL, 0xABCD000000000000, 8, 0xCD00000000000000);
    testInstR(INST_OP_SLL, 0xF000000000000000, 1, 0xE000000000000000);

    // SLLI
    testInstI(INST_OP_SLLI, 0x0123456789ABCDEF, 8, 0x23456789ABCDEF00);
    testInstI(INST_OP_SLLI, 0xABCD000000000000, 8, 0xCD00000000000000);
    testInstI(INST_OP_SLLI, 0xF000000000000000, 1, 0xE000000000000000);

    // SRL
    testInstR(INST_OP_SRL, 0x0123456789ABCDEF, 8, 0x000123456789ABCD);
    testInstR(INST_OP_SRL, 0xABCD000000000000, 8, 0x00ABCD0000000000);
    testInstR(INST_OP_SRL, 0xF000000000000000, 1, 0x7800000000000000);

    // SRLI
    testInstI(INST_OP_SRLI, 0x0123456789ABCDEF, 8, 0x000123456789ABCD);
    testInstI(INST_OP_SRLI, 0xABCD000000000000, 8, 0x00ABCD0000000000);
    testInstI(INST_OP_SRLI, 0xF000000000000000, 1, 0x7800000000000000);

    // SRA
    testInstR(INST_OP_SRA, 0x0123456789ABCDEF, 8, 0x000123456789ABCD);
    testInstR(INST_OP_SRA, 0xABCD000000000000, 8, 0xFFABCD0000000000);
    testInstR(INST_OP_SRA, 0xF000000000000000, 12, 0xFFFF000000000000);

    // SRAI
    testInstI(INST_OP_SRAI, 0x0123456789ABCDEF, 8, 0x000123456789ABCD);
    testInstI(INST_OP_SRAI, 0xABCD000000000000, 8, 0xFFABCD0000000000);
    testInstI(INST_OP_SRAI, 0xF000000000000000, 12, 0xFFFF000000000000);
}

void testLoadStore()
{
    PROCESS *process = newProcess();

    testInstLW(process, 0, 0, 0);

    testInstSW(process, 0, 0, 0x12345678);
    testInstLW(process, 0, 0, 0x0000000012345678);
    testInstLW(process, 0, 1, 0x0000000000123456);
    testInstLW(process, 1, 1, 0x0000000000001234);
    testInstLW(process, 1, 2, 0x0000000000000012);

    testInstSW(process, 0, 0, 0xFFEEDDCC);
    testInstLW(process, 0, 0, 0xFFFFFFFFFFEEDDCC);
    testInstLW(process, 0, 1, 0x0000000000FFEEDD);
    testInstLW(process, 1, 1, 0x000000000000FFEE);
    testInstLW(process, 1, 2, 0x00000000000000FF);

    testInstSW(process, 12, 1, 0x12345678);
    testInstLW(process, 12, 1, 0x12345678);
    testInstLW(process, 12, 2, 0x00123456);
    testInstLW(process, 13, 2, 0x00001234);
    testInstLW(process, 13, 3, 0x00000012);

    testInstSW(process, 1024, 1024, -12345678);
    testInstLW(process, 1024, 1024, -12345678);

    deleteProcess(process);
}

void testBranch()
{
    testInstSB(INST_OP_BEQ, 0, 0, true);
    testInstSB(INST_OP_BEQ, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, true);
    testInstSB(INST_OP_BEQ, 0x1234, 0x9876, false);

    testInstSB(INST_OP_BGE, 0, 0, true);
    testInstSB(INST_OP_BGE, 0, 1, false);
    testInstSB(INST_OP_BGE, 1, 0, true);
    testInstSB(INST_OP_BGE, 1, -1, true);
    testInstSB(INST_OP_BGE, -1, -1000, true);
    testInstSB(INST_OP_BGE, 0x0, 0xFFFFFFFFFFFFFFFF, true);
    testInstSB(INST_OP_BGE, 0xFFFFFFFFFFFFFFFF, 0x0, false);

    testInstSB(INST_OP_BLT, 0, 1, true);
    testInstSB(INST_OP_BLT, 1, 1, false);
    testInstSB(INST_OP_BLT, -1, 1, true);
    testInstSB(INST_OP_BLT, 1, -1, false);
    testInstSB(INST_OP_BLT, 0x0, 0xFFFFFFFFFFFFFFFF, false);
    testInstSB(INST_OP_BLT, 0xFFFFFFFFFFFFFFFF, 0x0, true);

    // 13 비트에서 마지막 비트는 0
    testInstSBJump(0, 1000);
    testInstSBJump(-1, 999);
    testInstSBJump(0b0111111111110, 1000 + 0b0111111111110);
    testInstSBJump(0b1000000000000, 1000 - 4096);
}

void testJump()
{
    testJAL(-2, 1000 + -2);
    testJAL(0, 1000 + 0);
    testJAL(2, 1000 + 2);

    testJALR(0, 0, 0);
}

void testAll()
{
    testALU();
    testLoadStore();
    testBranch();
    testJump();
}

// -------------------- Interface --------------------- //

void printBits(FILE *file, void *ptr, int size)
{
    unsigned char *b = (unsigned char *)ptr;
    for (int i = size - 1; i >= 0; i--)
    {
        for (int j = 7; j >= 0; j--)
        {
            unsigned char byte = (b[i] >> j) & 1;
            fprintf(file, "%u", byte);
        }
    }
}

PARSER_RESULT buildLabelStorage(FILE *file, PARSER_STATE *state, DYNAMIC_STORAGE *labelDefinitionStorage)
{
    if (flagVerbose)
        printf("Building label storage...\n");

    int pc = 1000;
    PARSER_RESULT result = 0;
    while (!(result & PARSER_RESULT_FLAG_EOF))
    {
        INST inst;
        result = parseInst(file, state, &inst);

        if (isSuccessfulParserResult(result))
        {
            if (result & PARSER_RESULT_FLAG_DEFINE_LABEL)
            {
                if (flagVerbose)
                    printf("%s -> %d\n", state->buffer, pc);
                addNewLabelStorage(labelDefinitionStorage, state->buffer, pc);
            }
            else if (result & PARSER_RESULT_FLAG_EMPTY)
            {
                continue;
            }
            else
            {
                if (flagVerbose)
                    printInst(pc, &inst);
                pc += 4;
            }
        }
        else
        {
            return result;
        }
    }

    return result;
}

bool checkValidJumpRange(int d, int bit)
{
    bool inRange = getMinimumOfBit(bit + 1) <= d && d <= getMaximumOfBit(bit + 1);
    bool isEven = (d & 1) == 0;
    return inRange && isEven;
}

PARSER_RESULT parseFileWithLabel(
    FILE *file,
    PARSER_STATE *state,
    DYNAMIC_STORAGE *instStorage,
    DYNAMIC_STORAGE *labelDefinitionStorage)
{
    if (flagVerbose)
        printf("Parsing instructions with labels...\n");

    int pc = 1000;
    INST inst;
    PARSER_RESULT result = 0;
    while (!(result & PARSER_RESULT_FLAG_EOF))
    {
        result = parseInst(file, state, &inst);

        if (isSuccessfulParserResult(result))
        {
            if (result & PARSER_RESULT_FLAG_DEFINE_LABEL)
            {
                if (flagVerbose)
                    printf("%s:\n", state->buffer);
                continue;
            }
            if (result & PARSER_RESULT_FLAG_EMPTY)
            {
                continue;
            }
            if (result & PARSER_RESULT_FLAG_WITH_LABEL)
            {
                LABEL *def = findLabelByName(labelDefinitionStorage, state->buffer);
                if (def == NULL)
                    return PARSER_RESULT_INVALID_LABEL;

                int d = def->pc - pc;
                inst.imm = d;

                if (getInstOpType(inst.op) == INST_OPTYPE_SB && 
                    !checkValidJumpRange(d, 12))
                    return PARSER_RESULT_INVALID_LABEL;

                if (getInstOpType(inst.op) == INST_OPTYPE_UJ && 
                    !checkValidJumpRange(d, 20))
                    return PARSER_RESULT_INVALID_LABEL;

                if (flagVerbose)
                    printf("(%s) ", state->buffer);
            }

            INST *copy = createInst();
            *copy = inst;
            addInstStorage(instStorage, copy);

            if (flagVerbose)
                printInst(pc, copy);

            pc += 4;
        }
        else
        {
            return result;
        }
    }

    return result;
}

PARSER_RESULT parseFile(FILE *file, PARSER_STATE *state, DYNAMIC_STORAGE *instStorage)
{
    DYNAMIC_STORAGE *labelDefinitionStorage = newDynamicStorage();

    PARSER_RESULT result;
    result = buildLabelStorage(file, state, labelDefinitionStorage);

    if (isSuccessfulParserResult(result))
    {
        rewind(file);
        result = parseFileWithLabel(file, state, instStorage, labelDefinitionStorage);
    }

    deleteDynamicStorageWithItems(labelDefinitionStorage);
    return result;
}

int startFile(char *filename)
{
    char filenameWithoutExt[246];
    char* dot = strrchr(filename, '.');
    if (dot)
    {
        int dotIndex = dot - filename;
        strncpy(filenameWithoutExt, filename, dotIndex);
        filenameWithoutExt[dotIndex] = '\0';
    }
    else
    {
        filenameWithoutExt[0] = '\0';
    }

    FILE *file;
    file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Input file does not exist!!\n");
        return EXIT_FAILURE;
    }

    DYNAMIC_STORAGE *instStorage = newDynamicStorage();
    PARSER_STATE state;
    initParserState(&state);
    PARSER_RESULT parserResult = parseFile(file, &state, instStorage);

    if (!isSuccessfulParserResult(parserResult))
    {
        printf("Syntax error!!\n");

        if (flagVerbose)
        {
            printf("result: %d\n", parserResult);
            printf("index: %d\n", state.index);
            printf("buffer: %s\n", state.buffer);
        }

        return EXIT_FAILURE;
    }

    char oFilename[256];
    snprintf(oFilename, sizeof(oFilename), "%s.o", filenameWithoutExt);

    FILE *out;
    out = fopen(oFilename, "w");
    if (out == NULL)
    {
        printf("Cannot open %s!!", "out.o");
        return EXIT_FAILURE;
    }

    if (flagVerbose)
        printf("Encoding instructions...\n");
    for (int i = 0; i < instStorage->size; i++)
    {
        INST *inst = getInstStorage(instStorage, i);
        ENCODED_INST encoded = encodeInst(inst);

        printBits(out, &encoded, sizeof(ENCODED_INST));
        fputc('\n', out);
    }

    char traceFilename[256];
    snprintf(traceFilename, sizeof(traceFilename), "%s.trace", filenameWithoutExt);

    FILE *trace;
    trace = fopen(traceFilename, "w");
    if (trace == NULL)
    {
        printf("Cannot open %s!!", "trace.txt");
        return EXIT_FAILURE;
    }

    PROCESS *process = newProcess();
    runProcess(process, instStorage, trace);
    deleteProcess(process);
    
    deleteDynamicStorageWithItems(instStorage);
    fclose(file);
    fclose(out);
    fclose(trace);

    return EXIT_SUCCESS;
}

int main()
{
    // 1: 테스트케이스 실행
    // 2: input.s 처리
    // 3: 명세서대로 실행
    int mode = 2;

    switch (mode)
    {
    case 1:
        testAll();
        printf("TESTCASE PASS\n");
        return EXIT_SUCCESS;
    case 2:
        flagVerbose = true;
        flagRunStepByStep = false;

        return startFile("tc5.s");
    case 3:
        flagVerbose = false;
        flagRunStepByStep = false;

        char filename[256];
        while (true)
        {
            printf(">> Enter Input File Name: ");
            scanf("%s", filename);
            if (strncmp(filename, "terminate", sizeof(filename)) == 0)
                return EXIT_SUCCESS;
            startFile(filename);
        }
    }
}