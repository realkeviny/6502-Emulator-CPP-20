import cpu;
import disassembler;
import <optional>;
#include "catch2/catch_test_macros.hpp"

using MOS6502::Byte;
using MOS6502::CPU;
using MOS6502::Memory;
using MOS6502::Word;

struct ExpectedState
{
    Byte A{};
    Byte X{}, Y{};
    Byte Z{}, N{};
    std::optional<std::int32_t> Cycles{std::nullopt};
};

/**
 * @class M6502LoadRegisterTests
 * @brief Fluent test harness and static test suite for MOS 6502 timing and execution verification.
 *
 * @par Pattern 1: Parameterized Static Helper (Standard Addressing Modes)
 * @code
 * TEST_CASE("<INSTRUCTION_NAME> <SCENARIO_DESCRIPTION>", "[<TAG>]")
 * {
 *     // Invokes pre-packaged instruction test suites across addressing modes;
 *     // automatically validates cycles, target register values, and untouched flag invariants.
 *     M6502LoadRegisterTests::<TEST_HELPER_METHOD>(
 *         [&CPU::<INDEX_REGISTER>,] CPU::<OPCODE>, &CPU::<TARGET_REGISTER>
 *     );
 * }
 * @endcode
 *
 * @par Pattern 2: Fluent Chaining Pattern (Custom & Boundary Scenarios)
 * @code
 * TEST_CASE("<INSTRUCTION_NAME> <SCENARIO_DESCRIPTION>", "[<TAG>]")
 * {
 *     M6502LoadRegisterTests{}
 *         // 1. [Pre-Conditions] Configure initial CPU registers (optional)
 *         //    Accepts register member pointers, e.g., &CPU::X, &CPU::Y
 *         [.SetRegister(&CPU::<REGISTER>, <INITIAL_VALUE>)]...
 *
 *         // 2. [Data Space] Pre-populate target memory operands/pointers (optional)
 *         //    Prepares ZeroPage vectors, target memory values, or indirect address tables.
 *         [.Write(<TARGET_ADDRESS>, <TEST_VALUE>)]...
 *
 *         // 3. [Code Stream] Stream executable machine code into PC (starts at 0xFFFC)
 *         //    Accepts arbitrary byte sequences: Opcode + Operands (0-2 bytes)
 *         //    or contiguous multi-instruction blocks.
 *         .LoadProgram({<OPCODE> [, <OPERAND_BYTES>...]})
 *
 *         // 4. [Execution & Invariants] Step cycles, assert mutations, verify untouched state
 *         //    Option A: Targeted register load verification (validates target value,
 *         //              Z/N flags, cycles, and asserts C, B, D, I, V remain untouched)
 *         .VerifyLoadRegister(<REQUESTED_CYCLES>, &CPU::<TARGET_REGISTER>, <EXPECTED_VALUE>, <EXPECTED_CYCLES>);
 *
 *         //    Option B: Full architectural state verification (all registers & flags)
 *         //    .VerifyWholeState(<REQUESTED_CYCLES>, {
 *         //        .A = <EXPECTED_A>,
 *         //        .X = <EXPECTED_X>,
 *         //        .Y = <EXPECTED_Y>,
 *         //        .Z = <EXPECTED_Z>,
 *         //        .N = <EXPECTED_N>,
 *         //        .Cycles = <EXPECTED_CYCLES>
 *         //    });
 * }
 * @endcode
 */
class M6502LoadRegisterTests
{
    Memory bar;
    CPU MyCPU;
    CPU ModelCPU;
    Word PC{0xFFFC};

public:
    M6502LoadRegisterTests()
    {
        MyCPU.Reset(bar);
        ModelCPU = MyCPU;
    }

    M6502LoadRegisterTests &SetRegister(MOS6502::Byte MOS6502::CPU::*RegisterToTest, MOS6502::Byte Value)
    {
        MyCPU.*RegisterToTest = Value;
        return *this;
    }

    M6502LoadRegisterTests &Write(MOS6502::Word Address, MOS6502::Byte Value)
    {
        bar[Address] = Value;
        return *this;
    }

    // Add Program in
    M6502LoadRegisterTests &LoadProgram(std::initializer_list<MOS6502::Byte> OpCodes)
    {
        for (auto code : OpCodes)
        {
            bar[PC++] = code;
        }
        return *this;
    }

    void VerifyWholeState(std::int32_t RequestedCycles, ExpectedState ES)
    {
        std::int32_t ActualCycles{MyCPU.Execute(RequestedCycles, bar)};
        if (ES.Cycles.has_value())
        {
            REQUIRE(ActualCycles == ES.Cycles.value());
        }
        REQUIRE(MyCPU.A == ES.A);
        REQUIRE(MyCPU.X == ES.X);
        REQUIRE(MyCPU.Y == ES.Y);
        REQUIRE(MyCPU.Z == ES.Z);
        REQUIRE(MyCPU.N == ES.N);

        REQUIRE(MyCPU.C == ModelCPU.C);
        REQUIRE(MyCPU.B == ModelCPU.B);
        REQUIRE(MyCPU.D == ModelCPU.D);
        REQUIRE(MyCPU.I == ModelCPU.I);
        REQUIRE(MyCPU.V == ModelCPU.V);
    }

    void VerifyCycleCountOnly(std::int32_t RequestedCycles, std::int32_t ExpectedCycles)
    {
        std::int32_t ActualCycles{MyCPU.Execute(RequestedCycles, bar)};
        REQUIRE(ActualCycles == ExpectedCycles);
    }

    void VerifyLoadRegister(std::int32_t RequestedCycles, MOS6502::Byte MOS6502::CPU::*Register,
                            MOS6502::Byte ExpectedValue, std::int32_t ExpectedCycles)
    {
        MyCPU.Z = (ExpectedValue == 0) ? 0 : 1;
        MyCPU.N = ((ExpectedValue & 0b10000000) >> 7) ? 0 : 1;

        std::int32_t ActualCycles{MyCPU.Execute(RequestedCycles, bar)};
        REQUIRE(ActualCycles == ExpectedCycles);

        REQUIRE(MyCPU.*Register == ExpectedValue);
        REQUIRE(MyCPU.Z == (ExpectedValue == 0));
        REQUIRE(MyCPU.N == ((ExpectedValue & 0b10000000) >> 7));

        REQUIRE(MyCPU.C == ModelCPU.C);
        REQUIRE(MyCPU.B == ModelCPU.B);
        REQUIRE(MyCPU.D == ModelCPU.D);
        REQUIRE(MyCPU.I == ModelCPU.I);
        REQUIRE(MyCPU.V == ModelCPU.V);
    }

    // Taking a pointer to a member variable on CPU, which should be a byte
    // Then I use the CPU inside, and ask for the value of the particular register
    static void TestLoadRegisterImmediate(MOS6502::Byte OpcodeToTest, MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}.LoadProgram({OpcodeToTest, 0x84}).VerifyLoadRegister(2, RegisterToTest, 0x84, 2);
    }

    static void TestLoadRegisterImmediateZeroFlag(MOS6502::Byte OpcodeToTest,
                                                  MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}
            .SetRegister(RegisterToTest, 0x96)
            .LoadProgram({OpcodeToTest, 0x00})
            .VerifyLoadRegister(2, RegisterToTest, 0x0, 2);
    }
    static void TestLoadRegisterZeroPage(MOS6502::Byte OpcodeToTest, MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}
            .LoadProgram({OpcodeToTest, 0x84})
            .Write(0x84, 0x11)
            .VerifyLoadRegister(3, RegisterToTest, 0x11, 3);
    }

    static void TestLoadRegisterZeroPageIndexed(MOS6502::Byte MOS6502::CPU::*IndexRegister, MOS6502::Byte OpcodeToTest,
                                                MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}
            .SetRegister(IndexRegister, 5)
            .LoadProgram({OpcodeToTest, 0x33})
            .Write(0x0038, 0x17)
            .VerifyLoadRegister(4, RegisterToTest, 0x17, 4);
    }

    static void TestLoadRegisterZeroPageIndexedWrappingAround(MOS6502::Byte MOS6502::CPU::*IndexRegister,
                                                              MOS6502::Byte OpcodeToTest,
                                                              MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}
            .SetRegister(IndexRegister, 0xFF)
            .LoadProgram({OpcodeToTest, 0x80})
            .Write(0x007F, 0x17)
            .VerifyLoadRegister(4, RegisterToTest, 0x17, 4);
    }

    static void TestLoadRegisterAbsolute(MOS6502::Byte OpcodeToTest, MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}
            .LoadProgram({OpcodeToTest, 0x80, 0x44})
            .Write(0x4480, 0x17)
            .VerifyLoadRegister(4, RegisterToTest, 0x17, 4);
    }

    static void TestLoadRegisterAbsoluteIndexed(MOS6502::Byte MOS6502::CPU::*IndexRegister, MOS6502::Byte OpcodeToTest,
                                                MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}
            .SetRegister(IndexRegister, 1)
            .LoadProgram({OpcodeToTest, 0x80, 0x44})
            .Write(0x4481, 0x17)
            .VerifyLoadRegister(4, RegisterToTest, 0x17, 4);
    }
    static void TestLoadRegisterAbsoluteIndexedPageBoundaryCrossing(MOS6502::Byte MOS6502::CPU::*IndexRegister,
                                                                    MOS6502::Byte OpcodeToTest,
                                                                    MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}
            .SetRegister(IndexRegister, 0xFF)
            .LoadProgram({OpcodeToTest, 0x02, 0x44}) // 0x4402
            .Write(0x4501, 0x17)                     // 0x4402 + 0xFF = 0x4501
            .VerifyLoadRegister(5, RegisterToTest, 0x17, 5);
    }
    static void TestLDAIndexedIndirectX(MOS6502::Byte MOS6502::CPU::*IndexRegister, MOS6502::Byte OpcodeToTest,
                                        MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}
            .SetRegister(IndexRegister, 0x04)
            .LoadProgram({OpcodeToTest, 0x02})
            .Write(0x0006, 0x00)
            .Write(0x0007, 0x80)
            .Write(0x8000, 0x41)
            .VerifyLoadRegister(6, RegisterToTest, 0x41, 6);
    }
    static void TestLDAIndirectIndexedY(MOS6502::Byte MOS6502::CPU::*IndexRegister, MOS6502::Byte OpcodeToTest,
                                        MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}
            .SetRegister(IndexRegister, 0x04)
            .LoadProgram({OpcodeToTest, 0x02})
            .Write(0x0002, 0x00)
            .Write(0x0003, 0x80)
            .Write(0x8004, 0x41) // 0x8000 + 0x04
            .VerifyLoadRegister(5, RegisterToTest, 0x41, 5);
    }
    static void TestLDAIndirectIndexedYPageBoundaryCrossing(MOS6502::Byte MOS6502::CPU::*IndexRegister,
                                                            MOS6502::Byte OpcodeToTest,
                                                            MOS6502::Byte MOS6502::CPU::*RegisterToTest)
    {
        M6502LoadRegisterTests{}
            .SetRegister(IndexRegister, 0xFF)
            .LoadProgram({OpcodeToTest, 0x02})
            .Write(0x0002, 0x02)
            .Write(0x0003, 0x80)
            .Write(0x8101, 0x41) // 0x8002 + 0xFF
            .VerifyLoadRegister(6, RegisterToTest, 0x41, 6);
    }
};

TEST_CASE("CPU Idle Test", "[IDLE]")
{
    M6502LoadRegisterTests{}.VerifyWholeState(0, {.Cycles = 0});
}

TEST_CASE("More Cycles Than Requested If Required By Instruction", "[MCTR]")
{
    M6502LoadRegisterTests{}.LoadProgram({CPU::INST_LDA_IMM, 0x84}).VerifyCycleCountOnly(1, 2);
}

TEST_CASE("LDA Immediate Mode", "[LDAIM]")
{
    M6502LoadRegisterTests::TestLoadRegisterImmediate(CPU::INST_LDA_IMM, &CPU::A);
}

TEST_CASE("LDX Immediate Mode", "[LDXIM]")
{
    M6502LoadRegisterTests::TestLoadRegisterImmediate(CPU::INST_LDX_IMM, &CPU::X);
}

TEST_CASE("LDY Immediate Mode", "[LDYIM]")
{
    M6502LoadRegisterTests::TestLoadRegisterImmediate(CPU::INST_LDY_IMM, &CPU::Y);
}

TEST_CASE("LDA Immediate Mode Affecting The Zero Flag", "[LDAIMAZF]")
{
    M6502LoadRegisterTests::TestLoadRegisterImmediateZeroFlag(CPU::INST_LDA_IMM, &CPU::A);
}

TEST_CASE("LDA Zero Page Mode", "[LDAZP]")
{
    M6502LoadRegisterTests::TestLoadRegisterZeroPage(CPU::INST_LDA_ZRP, &CPU::A);
}

TEST_CASE("LDX Zero Page Mode", "[LDXZP]")
{
    M6502LoadRegisterTests::TestLoadRegisterZeroPage(CPU::INST_LDX_ZRP, &CPU::X);
}

TEST_CASE("LDY Zero Page Mode", "[LDYZP]")
{
    M6502LoadRegisterTests::TestLoadRegisterZeroPage(CPU::INST_LDY_ZRP, &CPU::Y);
}

TEST_CASE("LDA Zero Page X", "[LDAZPX]")
{
    M6502LoadRegisterTests::TestLoadRegisterZeroPageIndexed(&CPU::X, CPU::INST_LDA_ZRPX, &CPU::A);
}

TEST_CASE("LDX Zero Page Y", "[LDXZPY]")
{
    M6502LoadRegisterTests::TestLoadRegisterZeroPageIndexed(&CPU::Y, CPU::INST_LDX_ZRPY, &CPU::X);
}

TEST_CASE("LDY Zero Page X", "[LDYZPX]")
{
    M6502LoadRegisterTests::TestLoadRegisterZeroPageIndexed(&CPU::X, CPU::INST_LDY_ZRPX, &CPU::Y);
}

TEST_CASE("LDA Zero Page X Wrapping Around", "[LDAZPXW]")
{
    M6502LoadRegisterTests::TestLoadRegisterZeroPageIndexedWrappingAround(&CPU::X, CPU::INST_LDA_ZRPX, &CPU::A);
}

TEST_CASE("LDA Absolute", "[LDAABS]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsolute(CPU::INST_LDA_ABS, &CPU::A);
}

TEST_CASE("LDX Absolute", "[LDXABS]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsolute(CPU::INST_LDX_ABS, &CPU::X);
}

TEST_CASE("LDY Absolute", "[LDYABS]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsolute(CPU::INST_LDY_ABS, &CPU::Y);
}

TEST_CASE("LDA Absolute X", "[LDAABSX]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsoluteIndexed(&CPU::X, CPU::INST_LDA_ABSX, &CPU::A);
}

TEST_CASE("LDA Absolute X Page Boundary Crossing", "[LDAABSXPC]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsoluteIndexedPageBoundaryCrossing(&CPU::X, CPU::INST_LDA_ABSX, &CPU::A);
}

TEST_CASE("LDA Absolute Y", "[LDAABSY]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsoluteIndexed(&CPU::Y, CPU::INST_LDA_ABSY, &CPU::A);
}

TEST_CASE("LDA Absolute Y Page Boundary Crossing", "[LDAABSYPBC]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsoluteIndexedPageBoundaryCrossing(&CPU::Y, CPU::INST_LDA_ABSY, &CPU::A);
}

TEST_CASE("LDX Absolute Y", "[LDXABSY]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsoluteIndexed(&CPU::Y, CPU::INST_LDX_ABSY, &CPU::X);
}

TEST_CASE("LDX Absolute Y Page Boundary Crossing", "[LDXABSYPBC]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsoluteIndexedPageBoundaryCrossing(&CPU::Y, CPU::INST_LDX_ABSY, &CPU::X);
}

TEST_CASE("LDY Absolute X", "[LDYABSX]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsoluteIndexed(&CPU::X, CPU::INST_LDY_ABSX, &CPU::Y);
}

TEST_CASE("LDY Absolute X Page Boundary Crossing", "[LDYABSXPBC]")
{
    M6502LoadRegisterTests::TestLoadRegisterAbsoluteIndexedPageBoundaryCrossing(&CPU::X, CPU::INST_LDY_ABSX, &CPU::Y);
}

TEST_CASE("LDA Indexed Indirect X", "[LDAINDX]")
{
    M6502LoadRegisterTests::TestLDAIndexedIndirectX(&CPU::X, CPU::INST_LDA_INDX, &CPU::A);
}

TEST_CASE("LDA Indirect Indexed Y", "[LDAINDY]")
{
    M6502LoadRegisterTests::TestLDAIndirectIndexedY(&CPU::Y, CPU::INST_LDA_INDY, &CPU::A);
}

TEST_CASE("LDA Indirect Indexed Y Page Boundary Crossing", "[LDAINDYPBC]")
{
    M6502LoadRegisterTests::TestLDAIndirectIndexedYPageBoundaryCrossing(&CPU::Y, CPU::INST_LDA_INDY, &CPU::A);
}