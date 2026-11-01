module;

module cpu;

import std;

namespace MOS6502
{
void CPU::Reset(Memory &Mbar)
{
    PC = 0xFFFC;
    SP = 0x0100;
    C = Z = I = D = B = V = N = 0;
    A = X = Y = 0;
    Mbar.init();
}

Byte CPU::FetchByte(std::int32_t &NumofCycles, const Memory &Mbar)
{
    Byte Data{Mbar[PC]};
    PC++;
    NumofCycles--;
    return Data;
}

Word CPU::FetchWord(std::int32_t &NumofCycles, const Memory &Mbar)
{
    Word Data{Mbar[PC]};
    PC++;

    Data |= (Mbar[PC] << 8);
    PC++;
    NumofCycles -= 2;

    // Handle endianness
    // Swap Bytes
    // if(IS_BIG_ENDIAN)
    // then SwapByteInWord(Data)
    return Data;
}

Byte CPU::ReadByte(std::int32_t &NumOfCycles, Word Address, const Memory &Mbar)
{
    Byte Data{Mbar[Address]};
    NumOfCycles--;
    return Data;
}

Word CPU::ReadWord(std::int32_t &NumOfCycles, Word Address, const Memory &Mbar)
{
    Byte LSB{ReadByte(NumOfCycles, Address, Mbar)};     // Lower Byte
    Byte HSB{ReadByte(NumOfCycles, Address + 1, Mbar)}; // Higher Byte

    return LSB | (HSB << 8);
}

void CPU::WriteByte(Byte Value, std::int32_t &NumOfCycles, Word Address, Memory &Mbar)
{
    Mbar[Address] = Value;
    NumOfCycles--;
}

void CPU::LoadRegisterStatusSet(Byte RegisterToSet)
{
    Z = (RegisterToSet == 0);
    N = (RegisterToSet & 0b10000000) >> 7;
}

Word CPU::AddrZeroPage(std::int32_t &NumOfCycles, const Memory &Mbar)
{
    Byte ZeroPageAddress{FetchByte(NumOfCycles, Mbar)};
    return ZeroPageAddress;
}

Word CPU::AddrZeroPageX(std::int32_t &NumOfCycles, const Memory &Mbar)
{
    Byte ZeroPageAddress{FetchByte(NumOfCycles, Mbar)};
    ZeroPageAddress += X;
    NumOfCycles--;
    return ZeroPageAddress;
}

Word CPU::AddrZeroPageY(std::int32_t &NumOfCycles, const Memory &Mbar)
{
    Byte ZeroPageAddress{FetchByte(NumOfCycles, Mbar)};
    ZeroPageAddress += Y;
    NumOfCycles--;
    return ZeroPageAddress;
}

Word CPU::AddrAbs(std::int32_t &NumOfCycles, const Memory &Mbar)
{
    Word AbsoluteAddress{FetchWord(NumOfCycles, Mbar)};
    return AbsoluteAddress;
}

Word CPU::AddrAbsX(std::int32_t &NumOfCycles, const Memory &Mbar)
{
    Word AbsoluteAddress{FetchWord(NumOfCycles, Mbar)};
    Word AbsoluteAddressX = AbsoluteAddress + X;
    if (AbsoluteAddressX - AbsoluteAddress >= 0xFF)
    {
        NumOfCycles--;
    }
    return AbsoluteAddressX;
}

Word CPU::AddrAbsY(std::int32_t &NumOfCycles, const Memory &Mbar)
{
    Word AbsoluteAddress{FetchWord(NumOfCycles, Mbar)};
    Word AbsoluteAddressY = AbsoluteAddress + Y;
    if (AbsoluteAddressY - AbsoluteAddress >= 0xFF)
    {
        NumOfCycles--;
    }
    return AbsoluteAddressY;
}

Word CPU::AddrIndexedIndirectX(std::int32_t &NumOfCycles, const Memory &Mbar)
{
    Byte ZeroPageAddress{FetchByte(NumOfCycles, Mbar)};
    ZeroPageAddress += X;
    NumOfCycles--;
    Word ActualAddress{ReadWord(NumOfCycles, ZeroPageAddress, Mbar)};
    return ActualAddress;
}

Word CPU::AddrIndirectIndexedY(std::int32_t &NumOfCycles, const Memory &Mbar)
{
    Byte ZeroPageAddress{FetchByte(NumOfCycles, Mbar)};
    Word ActualAddress{ReadWord(NumOfCycles, ZeroPageAddress, Mbar)};
    Word ActualAddressY = ActualAddress + Y;
    if (ActualAddressY - ActualAddress >= 0xFF)
    {
        NumOfCycles--;
    }
    return ActualAddressY;
}

// Return the number of cycles that were used
std::int32_t CPU::Execute(std::int32_t NumOfCycles, Memory &Mbar)
{
    // Load a Register using the value from the memory address.
    auto LoadRegister = [&NumOfCycles, &Mbar, this](Word Address, std::same_as<Byte> auto &Register) noexcept {
        Register = ReadByte(NumOfCycles, Address, Mbar);
        LoadRegisterStatusSet(Register);
    };

    const std::int32_t CyclesRequested(NumOfCycles);
    while (NumOfCycles > 0)
    {
        Byte Instruction{FetchByte(NumOfCycles, Mbar)};
        switch (Instruction)
        {
        case INST_LDA_IMM: {
            A = FetchByte(NumOfCycles, Mbar);
            LoadRegisterStatusSet(A);
        }
        break;

        case INST_LDX_IMM: {
            X = FetchByte(NumOfCycles, Mbar);
            LoadRegisterStatusSet(X);
        }
        break;

        case INST_LDY_IMM: {
            Y = FetchByte(NumOfCycles, Mbar);
            LoadRegisterStatusSet(Y);
        }
        break;

        case INST_LDA_ZRP: {
            Word Address{AddrZeroPage(NumOfCycles, Mbar)};
            LoadRegister(Address, A);
        }
        break;

        case INST_LDX_ZRP: {
            Word Address{AddrZeroPage(NumOfCycles, Mbar)};
            LoadRegister(Address, X);
        }
        break;

        case INST_LDY_ZRP: {
            Word Address{AddrZeroPage(NumOfCycles, Mbar)};
            LoadRegister(Address, Y);
        }
        break;

        case INST_LDA_ZRPX: {
            Word Address{AddrZeroPageX(NumOfCycles, Mbar)};
            LoadRegister(Address, A);
        }
        break;

        case INST_LDY_ZRPX: {
            Word Address{AddrZeroPageX(NumOfCycles, Mbar)};
            LoadRegister(Address, Y);
        }
        break;

        case INST_LDX_ZRPY: {
            Word Address{AddrZeroPageY(NumOfCycles, Mbar)};
            LoadRegister(Address, X);
        }
        break;

        case INST_LDA_ABS: {
            Word Address{AddrAbs(NumOfCycles, Mbar)};
            LoadRegister(Address, A);
        }
        break;

        case INST_LDX_ABS: {
            Word Address{AddrAbs(NumOfCycles, Mbar)};
            LoadRegister(Address, X);
        }
        break;

        case INST_LDY_ABS: {
            Word Address{AddrAbs(NumOfCycles, Mbar)};
            LoadRegister(Address, Y);
        }
        break;

        case INST_LDA_ABSX: {
            Word Address{AddrAbsX(NumOfCycles, Mbar)};
            LoadRegister(Address, A);
        }
        break;

        case INST_LDY_ABSX: {
            Word Address{AddrAbsX(NumOfCycles, Mbar)};
            LoadRegister(Address, Y);
        }
        break;

        case INST_LDA_ABSY: {
            Word Address{AddrAbsY(NumOfCycles, Mbar)};
            LoadRegister(Address, A);
        }
        break;

        case INST_LDX_ABSY: {
            Word Address{AddrAbsY(NumOfCycles, Mbar)};
            LoadRegister(Address, X);
        }
        break;

        case INST_LDA_INDX: {
            Word Address{AddrIndexedIndirectX(NumOfCycles, Mbar)};
            LoadRegister(Address, A);
        }
        break;

        case INST_LDA_INDY: {
            Word Address{AddrIndirectIndexedY(NumOfCycles, Mbar)};
            LoadRegister(Address, A);
        }
        break;

        case INST_STA_ZRP: {
            Word Address{AddrZeroPage(NumOfCycles, Mbar)};
            WriteByte(A, NumOfCycles, Address, Mbar);
        }
        break;

        case INST_STA_ZRPX: {
            Word Address{AddrZeroPageX(NumOfCycles, Mbar)};
            WriteByte(A, NumOfCycles, Address, Mbar);
        }
        break;

        case INST_STA_ABS: {
            Word Address{AddrAbs(NumOfCycles, Mbar)};
            WriteByte(A, NumOfCycles, Address, Mbar);
        }
        break;

        case INST_STA_ABSX: {
            Word Address{AddrAbsX(NumOfCycles, Mbar)};
            WriteByte(A, NumOfCycles, Address, Mbar);
            // AddrAbsoulteX can consume an extra cycle on boundaries?
            NumOfCycles--; // TODO: 2nd Try(sourcing the consumption of this cycle)
        }
        break;

        case INST_STA_ABSY: {
            Word Address{AddrAbsY(NumOfCycles, Mbar)};
            WriteByte(A, NumOfCycles, Address, Mbar);
            // AddrAbsoulteY can consume an extra cycle on boundaries?
            NumOfCycles--; // TODO: 2nd Try(sourcing the consumption of this cycle)
        }
        break;

        case INST_STA_INDX: {
            Word Address{AddrIndexedIndirectX(NumOfCycles, Mbar)};
            WriteByte(A, NumOfCycles, Address, Mbar);
        }
        break;

        case INST_STA_INDY: {
            Word Address{AddrIndirectIndexedY(NumOfCycles, Mbar)};
            WriteByte(A, NumOfCycles, Address, Mbar);
            // IndirectIndexedY can consume an extra cycle on boundaries?
            NumOfCycles--; // TODO: 2nd Try(sourcing the consumption of this cycle)
        }
        break;

        case INST_STX_ZRP: {
            Word Address{AddrZeroPage(NumOfCycles, Mbar)};
            WriteByte(X, NumOfCycles, Address, Mbar);
        }
        break;

        case INST_STX_ZRPY: {
            Word Address{AddrZeroPageY(NumOfCycles, Mbar)};
            WriteByte(X, NumOfCycles, Address, Mbar);
        }
        break;

        case INST_STX_ABS: {
            Word Address{AddrAbs(NumOfCycles, Mbar)};
            WriteByte(X, NumOfCycles, Address, Mbar);
        }
        break;

        case INST_STY_ZRP: {
            Word Address{AddrZeroPage(NumOfCycles, Mbar)};
            WriteByte(Y, NumOfCycles, Address, Mbar);
        }
        break;

        case INST_STY_ZRPX: {
            Word Address{AddrZeroPageX(NumOfCycles, Mbar)};
            WriteByte(Y, NumOfCycles, Address, Mbar);
        }
        break;

        case INST_STY_ABS: {
            Word Address{AddrAbs(NumOfCycles, Mbar)};
            WriteByte(Y, NumOfCycles, Address, Mbar);
        }
        break;

        case INST_JSR: {
            Word SubroutineAddress{FetchWord(NumOfCycles, Mbar)};
            Mbar.WriteWord(PC - 1, SP, NumOfCycles);
            SP += 2;
            PC = SubroutineAddress;
            NumOfCycles--;
        }
        break;

        default:
            [[unlikely]]
            {
                std::cout << "Unable to handle instruction: " << std::format("{:#04X}\n", Instruction);
                throw std::runtime_error(std::format("Illegal Instruction {:#04X} Detected!", Instruction));
            }
            break;
        }
    }
    const std::int32_t NumOfCyclesUsed(CyclesRequested - NumOfCycles);
    return NumOfCyclesUsed;
}

} // namespace MOS6502