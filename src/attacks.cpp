#include "attacks.h"
#include "bitboardUtil.h"

// Libs
#include <cstdint>
#include <cstdio>
#include <functional>
#include <x86intrin.h>

namespace magicalBits {
MagicalBitboards::MagicalBitboards() { initSliderAttacks(); }

uint64_t MagicalBitboards::getMask(uint8_t square) {
    return m_bishopMasks[square];
}

uint64_t MagicalBitboards::maskBishopAttacks(uint8_t position) {
    uint64_t attacks = 0ULL;

    char r, f;

    const uint8_t tr = position >> 3;
    const uint8_t tf = position & 7;

    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
        attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
        attacks |= (1ULL << (r * 8 + f));
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
        attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
        attacks |= (1ULL << (r * 8 + f));

    return attacks;
}

uint64_t MagicalBitboards::maskRookAttacks(uint8_t position) {
    uint64_t attacks = 0ULL;

    char r, f;

    const uint8_t tr = position >> 3;
    const uint8_t tf = position & 7;

    for (r = tr + 1; r <= 6; r++)
        attacks |= (1ULL << (r * 8 + tf));
    for (r = tr - 1; r >= 1; r--)
        attacks |= (1ULL << (r * 8 + tf));
    for (f = tf + 1; f <= 6; f++)
        attacks |= (1ULL << (tr * 8 + f));
    for (f = tf - 1; f >= 1; f--)
        attacks |= (1ULL << (tr * 8 + f));

    return attacks;
}

uint64_t MagicalBitboards::maskBishopAttacksWithBlock(uint8_t  position,
                                                      uint64_t boardMask) {
    uint64_t attacks = 0ULL;
    uint64_t attack  = 0ULL;

    char r, f;

    const uint8_t tr = position >> 3;
    const uint8_t tf = position & 7;

    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
        attack = (1ULL << (r * 8 + f));
        attacks |= attack;
        if (attack & boardMask)
            break;
    }

    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
        attack = (1ULL << (r * 8 + f));
        attacks |= attack;
        if (attack & boardMask)
            break;
    }
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
        attack = (1ULL << (r * 8 + f));
        attacks |= attack;
        if (attack & boardMask)
            break;
    }
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        attack = (1ULL << (r * 8 + f));
        attacks |= attack;
        if (attack & boardMask)
            break;
    }

    return attacks;
}

uint64_t MagicalBitboards::maskRookAttacksWithBlock(uint8_t  position,
                                                    uint64_t boardMask) {
    uint64_t attacks = 0ULL;
    uint64_t attack  = 0ULL;
    char     r, f;

    const uint8_t tr = position >> 3;
    const uint8_t tf = position & 7;

    for (r = tr + 1; r <= 7; r++) {
        attack = (1ULL << (r * 8 + tf));
        attacks |= attack;
        if (attack & boardMask)
            break;
    }

    for (r = tr - 1; r >= 0; r--) {
        attack = (1ULL << (r * 8 + tf));
        attacks |= attack;
        if (attack & boardMask)
            break;
    }
    for (f = tf + 1; f <= 7; f++) {
        attack = (1ULL << (tr * 8 + f));
        attacks |= attack;
        if (attack & boardMask)
            break;
    }

    for (f = tf - 1; f >= 0; f--) {
        attack = (1ULL << (tr * 8 + f));
        attacks |= attack;
        if (attack & boardMask)
            break;
    }

    return attacks;
}
uint8_t MagicalBitboards::countBits(uint64_t board) {
    uint8_t count = 0;

    while (board) {
        board &= board - 1;
        count++;
    }
    return count;
}

uint64_t MagicalBitboards::setOccupancy(uint32_t position, uint8_t bitCount,
                                        uint64_t attackMask) {
    uint64_t occupancy = 0ULL;

    unsigned long square;
    for (uint32_t count = 0; count < bitCount; count++) {
        square = bitScan(attackMask);
        attackMask &= attackMask - 1;

        if (position & (1 << count)) {
            occupancy |= 1ULL << square;
        }
    }
    return occupancy;
}

// This is only used for generating magical numbers once :)
uint64_t MagicalBitboards::findMagicNumber(
    uint32_t position, uint8_t bitCount,
    std::function<uint64_t(uint8_t)>           attackFunction,
    std::function<uint64_t(uint8_t, uint64_t)> attacksWithBlockFunction) {
    uint64_t occupancies[4096];

    uint64_t attacks[4096];

    uint64_t usedAttacks[4096];

    uint64_t attackMask = attackFunction(position);

    int occupancyIndicies = 1 << bitCount;

    for (int index = 0; index < occupancyIndicies; index++) {
        occupancies[index] = setOccupancy(index, bitCount, attackMask);

        attacks[index] = attacksWithBlockFunction(position, occupancies[index]);
    }

    for (int random_count = 0; random_count < 100000000; random_count++) {
        uint64_t magicNumber = randomNumber();

        if (countBits((attackMask * magicNumber) & 0xFF00000000000000) < 6)
            continue;

        memset(usedAttacks, 0ULL, sizeof(usedAttacks));

        int index, fail;

        for (index = 0, fail = 0; !fail && index < occupancyIndicies; index++) {
            int magicIndex =
                (int)((occupancies[index] * magicNumber) >> (64 - bitCount));

            if (usedAttacks[magicIndex] == 0ULL)
                usedAttacks[magicIndex] = attacks[index];

            else if (usedAttacks[magicIndex] != attacks[index])
                fail = 1;
        }

        if (!fail)
            return magicNumber;
    }

    // if magic number doesn't work
    printf("Magic number fails!\n");
    return 0ULL;
}

void MagicalBitboards::initMagicNumbers() {
    for (uint32_t square = 0; square < 64; square++) {
        std::printf(" 0x%lxULL,\n",
                    findMagicNumber(square, m_occupacyCountRook[square],
                                    maskRookAttacks, maskRookAttacksWithBlock));
    }
}

void MagicalBitboards::initSliderAttacks() {
    for (int square = 0; square < 64; square++) {
        m_bishopMasks[square] = maskBishopAttacks(square);

        uint64_t attackMask        = m_bishopMasks[square];
        int      relevantBitsCount = countBits(attackMask);
        int      occupancyIndicies = 1 << relevantBitsCount;

        for (int i = 0; i < occupancyIndicies; i++) {
            uint64_t occupancy = setOccupancy(i, relevantBitsCount, attackMask);
            int      magicIndex = (occupancy * m_bishopMagicBitboard[square]) >>
                             (64 - m_occupacyCountBishop[square]);
            m_bishopAttacks[square][magicIndex] =
                maskBishopAttacksWithBlock(square, occupancy);
        }

        m_rookMasks[square] = maskRookAttacks(square);

        attackMask        = m_rookMasks[square];
        relevantBitsCount = countBits(attackMask);
        occupancyIndicies = 1 << relevantBitsCount;

        for (int i = 0; i < occupancyIndicies; i++) {
            uint64_t occupancy = setOccupancy(i, relevantBitsCount, attackMask);
            int      magicIndex = (occupancy * m_rookMagicBitboard[square]) >>
                             (64 - m_occupacyCountRook[square]);
            m_rookAttacks[square][magicIndex] =
                maskRookAttacksWithBlock(square, occupancy);
        }
    }
}

uint64_t randomNumber() {
    uint64_t random1 = static_cast<uint64_t>(std::rand()) |
                       (static_cast<uint64_t>(std::rand()) << 16) |
                       (static_cast<uint64_t>(std::rand()) << 32) |
                       (static_cast<uint64_t>(std::rand()) << 48);
    uint64_t random2 = static_cast<uint64_t>(std::rand()) |
                       (static_cast<uint64_t>(std::rand()) << 16) |
                       (static_cast<uint64_t>(std::rand()) << 32) |
                       (static_cast<uint64_t>(std::rand()) << 48);
    // uint64_t random3 = static_cast<uint64_t>(std::rand()) |
    // (static_cast<uint64_t>(std::rand()) << 16) |
    // (static_cast<uint64_t>(std::rand()) << 32) |
    // (static_cast<uint64_t>(std::rand()) << 48);
    return random1 & random2; // & random3;
}

} // namespace magicalBits