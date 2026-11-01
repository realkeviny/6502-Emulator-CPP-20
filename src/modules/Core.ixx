module;

export module cpu:core;

import std;
import :types;
import :memory;

export namespace MOS6502
{
struct CPU
{
    // Program Counter & Stack Pointer
    Word PC;
    Word SP;

    Byte A; // Accumulator
    Byte X, Y;

    // CPU Status Flag
    Byte C : 1; // Carry
    Byte Z : 1; // Zero Flag
    Byte I : 1; // Interrupt
    Byte D : 1; // Decimal
    Byte B : 1; // Break
    Byte V : 1; // OverFlow
    Byte N : 1; // Negative

    void Reset(Memory &Mbar);
    // Return the number of cycles that were used
    std::int32_t Execute(std::int32_t NumOfCycles, Memory &Mbar);

    Byte FetchByte(std::int32_t &NumofCycles, const Memory &Mbar);
    Word FetchWord(std::int32_t &NumofCycles, const Memory &Mbar);
    Byte ReadByte(std::int32_t &NumOfCycles, Word Address, const Memory &Mbar);
    Word ReadWord(std::int32_t &NumOfCycles, Word Address, const Memory &Mbar);
    void WriteByte(Byte Value, std::int32_t &NumOfCycles, Word Address, Memory &Mbar);

    // LDA
    static constexpr Byte INST_LDA_IMM{0xA9};  // LDA Immediate Addressing Mode
    static constexpr Byte INST_LDA_ZRP{0xA5};  // LDA Zero Page Addressing Mode
    static constexpr Byte INST_LDA_ZRPX{0xB5}; // LDA Zero Page X Addressing Mode
    static constexpr Byte INST_LDA_ABS{0xAD};  // LDA Absolute Addressing Mode
    static constexpr Byte INST_LDA_ABSX{0xBD}; // LDA Absolute X Addressing Mode
    static constexpr Byte INST_LDA_ABSY{0xB9}; // LDA Absolute Y Addressing Mode
    static constexpr Byte INST_LDA_INDX{0xA1}; // LDA Indexed Indirect X Addressing Mode
    static constexpr Byte INST_LDA_INDY{0xB1}; // LDA Indirect Indexed Y Addressing Mode

    // LDX
    static constexpr Byte INST_LDX_IMM{0xA2};  // LDX Immediate Addressing Mode
    static constexpr Byte INST_LDX_ZRP{0xA6};  // LDX Zero Page Addressing Mode
    static constexpr Byte INST_LDX_ZRPY{0xB6}; // LDX Zero Page Y Addressing Mode
    static constexpr Byte INST_LDX_ABS{0xAE};  // LDX Absolute Addressing Mode
    static constexpr Byte INST_LDX_ABSY{0xBE}; // LDX Absolute Y Addressing Mode

    // LDY
    static constexpr Byte INST_LDY_IMM{0xA0};  // LDY Immediate Addressing Mode
    static constexpr Byte INST_LDY_ZRP{0xA4};  // LDY Zero Page Addressing Mode
    static constexpr Byte INST_LDY_ZRPX{0xB4}; // LDY Zero Page X Addressing Mode
    static constexpr Byte INST_LDY_ABS{0xAC};  // LDY Absolute Addressing Mode
    static constexpr Byte INST_LDY_ABSX{0xBC}; // LDY Absolute X Addressing Mode

    // STA
    static constexpr Byte INST_STA_ZRP{0x85};  // STA Zero Page Addressing Mode
    static constexpr Byte INST_STA_ZRPX{0x95}; // STA Zero Page Addressing Mode
    static constexpr Byte INST_STA_ABS{0x8D};  // STA Absolute Addressing Mode
    static constexpr Byte INST_STA_ABSX{0x9D}; // STA Absolute X Addressing Mode
    static constexpr Byte INST_STA_ABSY{0x99}; // STA Absolute Y Addressing Mode
    static constexpr Byte INST_STA_INDX{0x81}; // STA Indexed Indirect X Addressing Mode
    static constexpr Byte INST_STA_INDY{0x91}; // STA Indirect Indexed Y Addressing Mode

    // STX
    static constexpr Byte INST_STX_ZRP{0x86};  // STX Zero Page Addressing Mode
    static constexpr Byte INST_STX_ZRPY{0x96}; // STX Zero Page Y Addressing Mode
    static constexpr Byte INST_STX_ABS{0x8E};  // STX Absolute Addressing Mode

    // STY
    static constexpr Byte INST_STY_ZRP{0x84};  // STY Zero Page Addressing Mode
    static constexpr Byte INST_STY_ZRPX{0x94}; // STY Zero Page Addressing Mode
    static constexpr Byte INST_STY_ABS{0x8C};  // STY Absolute Addressing Mode

    static constexpr Byte INST_JSR{0x20}; // JSR

    /**
     * @brief Set the correct Processor Status After a Load Register Instruction(LDA, LDX, LDY)
     * @param RegisterToSet The value of register A, X, or Y.
     */
    void LoadRegisterStatusSet(MOS6502::Byte RegisterToSet);

    /**
     * @brief Addressing Mode: Zero Page
     * @param NumOfCycles
     * @param Mbar
     * @return Word
     */
    Word AddrZeroPage(std::int32_t &NumOfCycles, const Memory &Mbar);

    /**
     * @brief Addressing Mode: Zero Page + X offset
     * @param NumOfCycles
     * @param Mbar
     * @return Word
     */
    Word AddrZeroPageX(std::int32_t &NumOfCycles, const Memory &Mbar);

    /**
     * @brief Addressing Mode: Zero Page + Y offset
     * @param NumOfCycles
     * @param Mbar
     * @return Word
     */
    Word AddrZeroPageY(std::int32_t &NumOfCycles, const Memory &Mbar);

    /**
     * @brief Addressing Mode: Absolute
     * @param NumOfCycles
     * @param Mbar
     * @return Word
     */
    Word AddrAbs(std::int32_t &NumOfCycles, const Memory &Mbar);

    /**
     * @brief Addressing Mode: Absolute + X offset
     *
     * @param NumOfCycles
     * @param Mbar
     * @return Word
     */
    Word AddrAbsX(std::int32_t &NumOfCycles, const Memory &Mbar);

    /**
     * @brief Addressing Mode: Absolute + Y offset
     * @param NumOfCycles
     * @param Mbar
     * @return Word
     */
    Word AddrAbsY(std::int32_t &NumOfCycles, const Memory &Mbar);

    /**
     * @brief Addressing Mode: Indexed Indirect X
     * @param NumOfCycles
     * @param Mbar
     * @return Word
     */
    Word AddrIndexedIndirectX(std::int32_t &NumOfCycles, const Memory &Mbar);

    /**
     * @brief Addressing Mode: Indirect Indexed Y
     *
     * @param NumOfCycles
     * @param Mbar
     * @return Word
     */
    Word AddrIndirectIndexedY(std::int32_t &NumOfCycles, const Memory &Mbar);
};
} // namespace MOS6502