#include "chip8.hpp"
#include <string.h>
#include <stdlib.h>

static inline int get_nibble(int val, int bits, int val_to_binary_and = 0xFFFF)
{
    return ((val & val_to_binary_and) >> bits);
}

Chip8_CPU::Chip8_CPU()
{
    Reset();
}

Chip8_CPU::~Chip8_CPU()
{
}

void Chip8_CPU::Reset()
{
    I = 0x0000;
    pc = 0x200;
    sp = 0;
    memset(V, 0, sizeof(V));
    memset(stack, 0, sizeof(stack));
    rand_count = 0;
    srand(0x11223344);
}

int Chip8_CPU::Tick(IFramebufferOut *fb_out, uint8_t *mem, uint8_t *keypad, uint8_t *timer, uint8_t *stimer)
{
    uint16_t opcode = (mem[pc] << 8) | (mem[pc + 1]);
    uint32_t opcode_msb_nibble = get_nibble(opcode, 12, 0xf000);

    int reg_idx1, reg_idx2, k_value, res;

    switch (opcode_msb_nibble) {
    case 0:
        switch (opcode) {
        case 0x00E0:
            fb_out->Clear();
            pc += 2;
            break;
        case 0x00EE:
            sp--;
            pc = stack[sp];
            pc += 2;
            break;
        }
        break;
    case 1: /* 1NNN jump */
        pc = opcode & 0x0fff;
        break;
    case 2: /* 2NNN call */
        stack[sp++] = pc;
        pc = opcode & 0x0fff;
        break;
    case 3: //3xkk -> SE %Vx, $0xkk (Skip if Vx Equals kk)
        k_value = get_nibble(opcode, 0, 0x00ff);
        reg_idx1 = get_nibble(opcode, 8, 0x0f00);

        pc += (V[reg_idx1] == k_value) ? 4 : 2;
        break;
    case 4: //4xkk -> SNE %Vx, $0xkk (Skip if Vx not equal kk)
        k_value = get_nibble(opcode, 0, 0x00ff);
        reg_idx1 = get_nibble(opcode, 8, 0x0f00);

        pc += (V[reg_idx1] != k_value) ? 4 : 2;
        break;
    case 5: //5xy0 -> SE %Vx, %Vy (Skip if Vx equals Vy)
        if (opcode & 0xf) { // last 4 bits SHOULD be zero
            return -1;
        }
        pc += (V[get_nibble(opcode, 8, 0x0f00)] == V[get_nibble(opcode, 4, 0x00f0)]) ?
            4 : 2;
        break;
    case 6: //6xnn -> ld %Vx, $0xnn (Load nn into Vx)
        k_value = get_nibble(opcode, 0, 0x00ff);
        reg_idx1 = get_nibble(opcode, 8, 0x0f00);
        V[reg_idx1] = k_value;
        pc += 2;
        break;
    case 7: //7xnn -> add %Vx, $0xnn (add nn into Vx)
        k_value = get_nibble(opcode, 0, 0x00ff);
        reg_idx1 = get_nibble(opcode, 8, 0x0f00);
        V[reg_idx1] += k_value;
        pc += 2;
        break;
    case 8: //multiple variations (8XYN)
        reg_idx1 = get_nibble(opcode, 8, 0x0f00);
        reg_idx2 = get_nibble(opcode, 4, 0xf0);
        switch(get_nibble(opcode, 0, 0x000f)) { // last 4 bits
        case 0: //8xy0 -> ld %vx, %vy (Set Vx from Vy)
            V[reg_idx1] = V[reg_idx2];
            pc += 2;
            break;
        case 1: //8xy1 -> or %vx, %vy
            V[reg_idx1] |= V[reg_idx2];
            pc += 2;
            break;
        case 2: //8xy2 -> and %vx, %vy
            V[reg_idx1] &= V[reg_idx2];
            pc += 2;
            break;
        case 3: //8xy3 -> xor %vx, %vy
            V[reg_idx1] ^= V[reg_idx2];
            pc += 2;
            break;
        case 4: //8xy4 -> add %vx, %vy ; VF if carry
            res = V[reg_idx1] + V[reg_idx2];
            V[0xF] = (res > 0xff) ? 1 : 0;
            V[reg_idx1] = (uint8_t) res & 0xff;
            pc += 2;
            break;
        case 5: //8xy5 -> sub %vx, %vy (Substract Vy from Vx, store on Vx)
            V[0xF] = (V[reg_idx1] < V[reg_idx2]) ? 0 : 1;
            V[reg_idx1] -= V[reg_idx2];
            pc += 2;
            break;
        case 6: //8xy6 -> shr %vx (shift Vx right by 1)
            V[0xf] = V[reg_idx1] & 1;
            V[reg_idx1] >>= 1;
            pc += 2;
            break;
        case 7: //8xy7 -> subn %vx, %vy (Subsctract Vx from Vt, store on Vx)
            V[0xF] = (V[reg_idx1] > V[reg_idx2]) ? 0 : 1;
            V[reg_idx1] = V[reg_idx2] - V[reg_idx1];
            pc += 2;
            break;
        case 0xE:
            V[0xF] = V[reg_idx1] >> 7;
            V[reg_idx1] <<= 1;
            pc += 2;
            break;
        default:
            return -1;
        }
        break;
    case 9: //9XY0 -> sne %vx, %vy (skip if vx doesnt equal vy)
        reg_idx1 = get_nibble(opcode, 8, 0x0f00);
        reg_idx2 = get_nibble(opcode, 4, 0xf0);
        pc += (V[reg_idx1] != V[reg_idx2]) ? 4 : 2;
        break;
    case 0xA: //ANNN -> ld %I, $0xNNN
        I = opcode & 0x0fff;
        pc += 2;
        break;
    case 0xB: //BNNN -> jmp %v0, $0xNNN
        pc = opcode & 0x0fff;
        pc += V[0];
        break;
    case 0xC: //CXNN -> rnd %vx, $0xNN (Vx = rand() & 0xNN)
        reg_idx1 = get_nibble(opcode, 8, 0x0f00);
        k_value = opcode & 0xff;

        V[reg_idx1] = ((uint8_t)rand()) & k_value;
        pc += 2;
        break;
    case 0xD: //DXYN -> drw %vx, %vy, $0xN (draw sprite at (Vx, Vy) with N pixels of height)
        reg_idx1 = get_nibble(opcode, 8, 0x0f00);
        reg_idx2 = get_nibble(opcode, 4, 0xf0);
        k_value = get_nibble(opcode, 0, 0xf);
        V[0xF] = 0;

        for (int y = 0; y < k_value; ++y) {
            uint8_t pix = mem[I + y];
            for (int x=0; x < 8; ++x) {
                if (!(pix & (0x80 >> x))) {
                    continue;
                }
                V[0xF] |= fb_out->Put(
                    V[reg_idx1] + x,
                    V[reg_idx2] + y,
                    0
                );
            }
        }

        pc += 2;
        break;
    case 0xE: // input instructions
        reg_idx1 = get_nibble(opcode, 8, 0x0f00);
        switch(get_nibble(opcode, 0, 0xff)) {
        case 0x9E: //EX9E -> skp %vx (skip if key of value Vx is pressed)
            pc += (keypad[V[reg_idx1] & 0xf]) ? 4 : 2;
            break;
        case 0xA1:
            pc += (!keypad[V[reg_idx1] & 0xf]) ? 4 : 2;
            break;
        default:
            return -1;
        }
        break;
    case 0xF: // various instructions
        reg_idx1 = get_nibble(opcode, 8, 0x0f00);
        switch(get_nibble(opcode, 0, 0xff)) {
        case 0x07: //FX07 -> ld %vx, %DT (load delay timer value into Vx)
            V[reg_idx1] = *timer;
            pc += 2;
            break;
        case 0x0A: // FX0A -> wait key press and store on %vx
            for (int i = 0; i < 16; ++i) {
                if (keypad[i]) {
                    pc += 2;
                    break;
                }
            }
            break;
        case 0x15: //FX15 -> ld %DT, %vx (set delay timer to the value of Vx)
            *timer = V[reg_idx1];
            pc += 2;
            break;
        case 0x18: //FX18 -> ld %ST, %vx (set sound timer to the value of Vx)
            *stimer = V[reg_idx1];
            pc += 2;
            break;
        case 0x1E: //FX1E -> add %I, %Vx (Add Vx to I)
            res = I + V[reg_idx1];
            V[0xF] = (res > 0xfff) ? 1 : 0;
            I = res;
            pc += 2;
            break;
        case 0x29: //FX29 -> set to text sprite (not implemented yet)
            I = V[reg_idx1] * 5;
            pc += 2;
            break;
        case 0x33: //FX33 ->
            mem[I] = (uint8_t) ((uint8_t) V[reg_idx1] / 100);
            mem[I + 1] = (uint8_t) ((uint8_t) (V[reg_idx1] / 10) % 10);
            mem[I + 2] = (uint8_t) ((uint8_t) (V[reg_idx1] % 100) % 10);
            pc += 2;
            break;
        case 0x55: //FX55
            for (int i = 0; i <= reg_idx1; ++i) {
                mem[I++] = V[i];
            }
            pc += 2;
            break;
        case 0x65: //FX65
            for (int i = 0; i <= reg_idx1; ++i) {
                V[i] = mem[I++];
            }
            pc += 2;
            break;
        default:
            return -1;
        }
    }

    return 0;
}