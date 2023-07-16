#pragma once

#include <stdint.h>
#include <functional>

namespace magicalBits
{

    class MagicalBitboards
    {
    public:
        MagicalBitboards();

        uint64_t getMask(uint8_t square);


        static uint64_t findMagicNumber(uint32_t position, uint8_t bitCount,
            std::function<uint64_t(uint8_t)> attackFunction,
            std::function<uint64_t(uint8_t, uint64_t)> attacksWithBlockFunction);
        void initMagicNumbers();
        // private:
        
        static uint64_t maskBishopAttacks(uint8_t position);
        static uint64_t maskRookAttacks(uint8_t position);
        static uint64_t maskBishopAttacksWithBlock(uint8_t position, uint64_t boardMask);
        static uint64_t maskRookAttacksWithBlock(uint8_t position, uint64_t boardMask);
        static uint8_t countBits(uint64_t board);
        static uint64_t setOccupancy(uint32_t position, uint8_t bitCount, uint64_t attackMask);
        

    protected:
        void initSliderAttacks();

        uint64_t m_bishopMasks[64];
        uint64_t m_rookMasks[64];

        uint64_t m_bishopAttacks[64][512];
        uint64_t m_rookAttacks[64][4096];
    protected:
        static constexpr uint8_t m_occupacyCountBishop[64] =
        { 6, 5, 5, 5, 5, 5, 5, 6,
         5, 5, 5, 5, 5, 5, 5, 5,
         5, 5, 7, 7, 7, 7, 5, 5,
         5, 5, 7, 9, 9, 7, 5, 5,
         5, 5, 7, 9, 9, 7, 5, 5,
         5, 5, 7, 7, 7, 7, 5, 5,
         5, 5, 5, 5, 5, 5, 5, 5,
         6, 5, 5, 5, 5, 5, 5, 6 };
        static constexpr uint8_t m_occupacyCountRook[64] =
        { 12, 11, 11, 11, 11, 11, 11, 12,
         11, 10, 10, 10, 10, 10, 10, 11,
         11, 10, 10, 10, 10, 10, 10, 11,
         11, 10, 10, 10, 10, 10, 10, 11,
         11, 10, 10, 10, 10, 10, 10, 11,
         11, 10, 10, 10, 10, 10, 10, 11,
         11, 10, 10, 10, 10, 10, 10, 11,
         12, 11, 11, 11, 11, 11, 11, 12 };

        static constexpr uint64_t m_bishopMagicBitboard[64] =
        { 0x1104043802430608ULL,
         0x48600381421c3088ULL,
         0x1004011206050048ULL,
         0x2008048118581261ULL,
         0xac5040030810a4ULL,
         0x22010521200200ULL,
         0x401e061220450926ULL,
         0x10e020710821030ULL,
         0x486301010093060ULL,
         0x1442280807540da0ULL,
         0x421a500c20802201ULL,
         0x281080841000140ULL,
         0x2241c0504140227ULL,
         0x38a1250452405085ULL,
         0x1058008801282081ULL,
         0x2c14338841301000ULL,
         0x1204556044703220ULL,
         0x420e381a180a08ULL,
         0x106d1628060400b1ULL,
         0x8006220a04082ULL,
         0xa0c041201210302ULL,
         0x94a022342304400ULL,
         0x200101c94f082048ULL,
         0x21a24840221e1014ULL,
         0x5040004a381108ULL,
         0x4764108360814100ULL,
         0x44180a1029020204ULL,
         0x1004044040002ULL,
         0x60020041405000ULL,
         0x1118084018806007ULL,
         0x219c2bc060980420ULL,
         0x40810826424c0401ULL,
         0x1118044004314202ULL,
         0x121284200783048ULL,
         0xe005000610300ULL,
         0x3c13200800090810ULL,
         0x29020484008a0020ULL,
         0x19010700aa0341ULL,
         0x2821080204e90f38ULL,
         0x3098010020c04a21ULL,
         0x206e101024800801ULL,
         0x2000482450003488ULL,
         0x80f1c1850081804ULL,
         0x240220420281180cULL,
         0x5826084104002441ULL,
         0x3208050802030220ULL,
         0x7812621454044100ULL,
         0x4830588e00821042ULL,
         0x2204110119201102ULL,
         0x19032a090c200404ULL,
         0x520018404c82240ULL,
         0x408e314842020000ULL,
         0x2080200425040240ULL,
         0x4a0200a4a8a0003ULL,
         0x4060606965031023ULL,
         0x8820c45060204a9ULL,
         0x4005450405a00400ULL,
         0x2077114228140a25ULL,
         0x22102941014b1073ULL,
         0x1cc0421402420205ULL,
         0x6c8b2a4042104109ULL,
         0x41a2401950304188ULL,
         0x3b06700c5848304aULL,
         0x2160381011132121ULL };

        static constexpr uint64_t m_rookMagicBitboard[64] =
        { 0x280018940015060ULL,
         0x4c0006008405005ULL,
         0xe000a8022001040ULL,
         0xd00100048646100ULL,
         0x680180044004680ULL,
         0x5a00012804160050ULL,
         0x880208021000200ULL,
         0x4600020b02224184ULL,
         0x10860022058300c0ULL,
         0x202400220100048ULL,
         0x2822001200402088ULL,
         0x401a000a10220042ULL,
         0x92a0020040a0010ULL,
         0x4cb00480c001700ULL,
         0x83c004204082150ULL,
         0x41100055e960100ULL,
         0xd08218001c00884ULL,
         0x817020028820040ULL,
         0x3010370020004100ULL,
         0x2e630039001000ULL,
         0x40b80280082c0080ULL,
         0xc2010100080400ULL,
         0x10a00c0010021807ULL,
         0x10200010348a4ULL,
         0x2480025140002005ULL,
         0x4008470200220281ULL,
         0x5620030100241040ULL,
         0x120601c200200812ULL,
         0x1001001100080084ULL,
         0x4909001300081400ULL,
         0x46511400103278ULL,
         0x2000269200130054ULL,
         0x60244006800081ULL,
         0x810002000400051ULL,
         0x491704103002000ULL,
         0x102420112000820ULL,
         0x3022010812002004ULL,
         0x113c001002020008ULL,
         0x410308304001842ULL,
         0x31035830e00224cULL,
         0x1280094660084005ULL,
         0xc3000201040400bULL,
         0x4930004020010100ULL,
         0x490030108110020ULL,
         0x2920010040a0020ULL,
         0x4bc409060080104ULL,
         0x1401802391400b0ULL,
         0x109240044860027ULL,
         0x1403148008e04100ULL,
         0x4920208108400d00ULL,
         0x820021020470100ULL,
         0x70052008110100ULL,
         0x2166041100080100ULL,
         0x5000204000900ULL,
         0x52c6110210e80400ULL,
         0x4a011409488b0200ULL,
         0x2043321048000c1ULL,
         0x407a400102201481ULL,
         0x12010400a0282ULL,
         0x4002000840102106ULL,
         0x27200082015501eULL,
         0x420200106c130846ULL,
         0x86a12821028250cULL,
         0x104202104840646ULL };
    };

    uint64_t randomNumber();
}  // namespace magicalBits