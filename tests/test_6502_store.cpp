import cpu;
import disassembler;
import <optional>;
#include "catch2/catch_test_macros.hpp"

using MOS6502::Byte;
using MOS6502::CPU;
using MOS6502::Memory;
using MOS6502::Word;

class M6502StoreRegisterTests
{
    Memory bar;
    CPU MyCPU;
    CPU ModelCPU;
    Word PC{0xFFFC};

public:
    M6502StoreRegisterTests()
    {
        MyCPU.Reset(bar);
        ModelCPU = MyCPU;
    }

    M6502StoreRegisterTests &SetRegister(MOS6502::Byte MOS6502::CPU::*RegisterToTest, MOS6502::Byte Value)
    {
        MyCPU.*RegisterToTest = Value;
        ModelCPU.*RegisterToTest = Value;
        return *this;
    }

    M6502StoreRegisterTests &Store(MOS6502::Word Address, MOS6502::Byte Value)
    {
        bar[Address] = Value;
        return *this;
    }

    // Add Program in
    M6502StoreRegisterTests &LoadProgram(std::initializer_list<MOS6502::Byte> OpCodes)
    {
        for (auto code : OpCodes)
        {
            bar[PC++] = code;
        }
        return *this;
    }

    void VerifyStates(std::int32_t RequestedCycles, Word Address, Byte Value, std::int32_t ExpectedCycles)
    {
        std::int32_t ActualCycles{MyCPU.Execute(RequestedCycles, bar)};
        REQUIRE(ActualCycles == ExpectedCycles);
        REQUIRE(bar[Address] == Value);

        REQUIRE(MyCPU.A == ModelCPU.A);
        REQUIRE(MyCPU.X == ModelCPU.X);
        REQUIRE(MyCPU.Y == ModelCPU.Y);
        REQUIRE(MyCPU.C == ModelCPU.C);
        REQUIRE(MyCPU.B == ModelCPU.B);
        REQUIRE(MyCPU.D == ModelCPU.D);
        REQUIRE(MyCPU.I == ModelCPU.I);
        REQUIRE(MyCPU.V == ModelCPU.V);
        REQUIRE(MyCPU.Z == ModelCPU.Z);
        REQUIRE(MyCPU.N == ModelCPU.N);
    }

    template <auto ValueRegister>
    static void TestStoreMemoryZeroPage(MOS6502::Byte OpcodeToTest)
    {
        M6502StoreRegisterTests{}
            .SetRegister(ValueRegister, 0x2F)
            .LoadProgram({OpcodeToTest, 0x32})
            .Store(0x32, 0x00)
            .VerifyStates(3, 0x32, 0x2F, 3);
    }

    template <auto ValueRegister>
    static void TestStoreMemoryAbsolute(MOS6502::Byte OpcodeToTest)
    {
        M6502StoreRegisterTests{}
            .SetRegister(ValueRegister, 0x2F)
            .LoadProgram({OpcodeToTest, 0x33, 0x22})
            .Store(0x2233, 0x00)
            .VerifyStates(4, 0x2233, 0x2F, 4);
    }

    template <auto OffsetRegister, auto ValueRegister>
    static void TestStoreMemoryZeroPageOffset(MOS6502::Byte OpcodeToTest)
    {
        M6502StoreRegisterTests{}
            .SetRegister(OffsetRegister, 0x0F)
            .SetRegister(ValueRegister, 0x65)
            .LoadProgram({OpcodeToTest, 0x32})
            .Store(0x41, 0x00)
            .VerifyStates(4, 0x41, 0x65, 4);
    }

    template <auto OffsetRegister, auto ValueRegister>
    static void TestStoreMemoryAbsoluteOffset(MOS6502::Byte OpcodeToTest)
    {
        M6502StoreRegisterTests{}
            .SetRegister(OffsetRegister, 0x0F)
            .SetRegister(ValueRegister, 0x39)
            .LoadProgram({OpcodeToTest, 0x33, 0x22})
            .Store(0x2242, 0x00)
            .VerifyStates(5, 0x2242, 0x39, 5);
    }

    template <auto OffsetRegister, auto ValueRegister>
    static void TestSTAIndexedIndirectX(MOS6502::Byte OpcodeToTest)
    {
        M6502StoreRegisterTests{}
            .SetRegister(OffsetRegister, 0x0F)
            .SetRegister(ValueRegister, 0x39)
            .LoadProgram({OpcodeToTest, 0x33})
            .Store(0x42, 0x00)
            .Store(0x43, 0x78)
            .Store(0x7800, 0x00)
            .VerifyStates(6, 0x7800, 0x39, 6);
    }

    template <auto OffsetRegister, auto ValueRegister>
    static void TestSTAIndirectIndexedY(MOS6502::Byte OpcodeToTest)
    {
        M6502StoreRegisterTests{}
            .SetRegister(OffsetRegister, 0x0F)
            .SetRegister(ValueRegister, 0x39)
            .LoadProgram({OpcodeToTest, 0x33})
            .Store(0x33, 0x00)
            .Store(0x34, 0x78)
            .Store(0x7800 + 0x0F, 0x00)
            .VerifyStates(6, 0x7800 + 0x0F, 0x39, 6);
    }
};

TEST_CASE("STA Zero Page Mode", "[STAZP]")
{
    M6502StoreRegisterTests::TestStoreMemoryZeroPage<&CPU::A>(CPU::INST_STA_ZRP);
}

TEST_CASE("STX Zero Page Mode", "[STXZP]")
{
    M6502StoreRegisterTests::TestStoreMemoryZeroPage<&CPU::X>(CPU::INST_STX_ZRP);
}

TEST_CASE("STY Zero Page Mode", "[STYZP]")
{
    M6502StoreRegisterTests::TestStoreMemoryZeroPage<&CPU::Y>(CPU::INST_STY_ZRP);
}

TEST_CASE("STA Absolute Mode", "[STAABS]")
{
    M6502StoreRegisterTests::TestStoreMemoryAbsolute<&CPU::A>(CPU::INST_STA_ABS);
}

TEST_CASE("STX Absolute Mode", "[STXABS]")
{
    M6502StoreRegisterTests::TestStoreMemoryAbsolute<&CPU::X>(CPU::INST_STX_ABS);
}

TEST_CASE("STY Absolute Mode", "[STYABS]")
{
    M6502StoreRegisterTests::TestStoreMemoryAbsolute<&CPU::Y>(CPU::INST_STY_ABS);
}

TEST_CASE("STA Zero Page X Mode", "[STAZRPX]")
{
    M6502StoreRegisterTests::TestStoreMemoryZeroPageOffset<&CPU::X, &CPU::A>(CPU::INST_STA_ZRPX);
}

TEST_CASE("STX Zero Page Y Mode", "[STXZRPY]")
{
    M6502StoreRegisterTests::TestStoreMemoryZeroPageOffset<&CPU::Y, &CPU::X>(CPU::INST_STX_ZRPY);
}

TEST_CASE("STY Zero Page X Mode", "[STYZRPX]")
{
    M6502StoreRegisterTests::TestStoreMemoryZeroPageOffset<&CPU::X, &CPU::Y>(CPU::INST_STY_ZRPX);
}

TEST_CASE("STA Absolute X Mode", "[STAABSX]")
{
    M6502StoreRegisterTests::TestStoreMemoryAbsoluteOffset<&CPU::X, &CPU::A>(CPU::INST_STA_ABSX);
}

TEST_CASE("STA Absolute Y Mode", "[STAABSY]")
{
    M6502StoreRegisterTests::TestStoreMemoryAbsoluteOffset<&CPU::Y, &CPU::A>(CPU::INST_STA_ABSY);
}

TEST_CASE("STA Indexed Indirect X Mode", "[STAINDINDIX]")
{
    M6502StoreRegisterTests::TestSTAIndexedIndirectX<&CPU::X, &CPU::A>(CPU::INST_STA_INDX);
}

TEST_CASE("STA Indirect Indexed Y Mode", "[STAINDIINDY]")
{
    M6502StoreRegisterTests::TestSTAIndirectIndexedY<&CPU::Y, &CPU::A>(CPU::INST_STA_INDY);
}