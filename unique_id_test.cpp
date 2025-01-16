#include <gtest/gtest.h>
#include "unique_id.h"
#include <set>
#include <thread>
#include <unordered_set>

class UniqueIdTest : public testing::Test {
protected:
    using DefaultId = Int64Id;
    using SmallId = UniqueId<uint16_t>;
    using UUID = UniqueId<detail::uuid_storage>;
};

TEST_F(UniqueIdTest, Construction) {
    DefaultId defaultId(0x1234567890ABCDEF);
    EXPECT_EQ(defaultId.value(), 0x1234567890ABCDEF);

    SmallId smallId(0x1234);
    EXPECT_EQ(smallId.value(), 0x1234);

    UUID uuid(0x123456789ABCDEF0, 0xFEDCBA9876543210);
    EXPECT_EQ(uuid.high(), 0x123456789ABCDEF0);
    EXPECT_EQ(uuid.low(), 0xFEDCBA9876543210);
}

TEST_F(UniqueIdTest, ByteAccess) {
    DefaultId defaultId(0x1234567890ABCDEF);
    auto bytes = defaultId.bytes();
    EXPECT_EQ(bytes[0], 0xEF);
    EXPECT_EQ(bytes[1], 0xCD);
    EXPECT_EQ(bytes[2], 0xAB);
    EXPECT_EQ(bytes[3], 0x90);
    EXPECT_EQ(bytes[4], 0x78);
    EXPECT_EQ(bytes[5], 0x56);
    EXPECT_EQ(bytes[6], 0x34);
    EXPECT_EQ(bytes[7], 0x12);

    UUID uuid(0x123456789ABCDEF0, 0xFEDCBA9876543210);
    auto uuid_bytes = uuid.bytes();
    // High part
    EXPECT_EQ(uuid_bytes[0], 0xF0);
    EXPECT_EQ(uuid_bytes[1], 0xDE);
    EXPECT_EQ(uuid_bytes[2], 0xBC);
    EXPECT_EQ(uuid_bytes[3], 0x9A);
    EXPECT_EQ(uuid_bytes[4], 0x78);
    EXPECT_EQ(uuid_bytes[5], 0x56);
    EXPECT_EQ(uuid_bytes[6], 0x34);
    EXPECT_EQ(uuid_bytes[7], 0x12);
    // Low part
    EXPECT_EQ(uuid_bytes[8], 0x10);
    EXPECT_EQ(uuid_bytes[9], 0x32);
    EXPECT_EQ(uuid_bytes[10], 0x54);
    EXPECT_EQ(uuid_bytes[11], 0x76);
    EXPECT_EQ(uuid_bytes[12], 0x98);
    EXPECT_EQ(uuid_bytes[13], 0xBA);
    EXPECT_EQ(uuid_bytes[14], 0xDC);
    EXPECT_EQ(uuid_bytes[15], 0xFE);
}

TEST_F(UniqueIdTest, SetValue) {
    DefaultId defaultId;
    defaultId.setValue(0x1234567890ABCDEF);
    EXPECT_EQ(defaultId.value(), 0x1234567890ABCDEF);

    SmallId smallId;
    smallId.setValue(0x1234);
    EXPECT_EQ(smallId.value(), 0x1234);

    UUID uuid;
    uuid.setValue(0x123456789ABCDEF0, 0xFEDCBA9876543210);
    EXPECT_EQ(uuid.high(), 0x123456789ABCDEF0);
    EXPECT_EQ(uuid.low(), 0xFEDCBA9876543210);
}

TEST_F(UniqueIdTest, SetBytes) {
    uint8_t bytes[] = {0xEF, 0xCD, 0xAB, 0x90, 0x78, 0x56, 0x34, 0x12};
    DefaultId defaultId;
    EXPECT_TRUE(defaultId.setBytes(bytes, sizeof(bytes)));
    EXPECT_EQ(defaultId.value(), 0x1234567890ABCDEF);

    uint8_t small_bytes[] = {0x34, 0x12};
    SmallId smallId;
    EXPECT_TRUE(smallId.setBytes(small_bytes, sizeof(small_bytes)));
    EXPECT_EQ(smallId.value(), 0x1234);

    uint8_t uuid_bytes[] = {
        0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12,
        0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE
    };
    UUID uuid;
    EXPECT_TRUE(uuid.setBytes(uuid_bytes, sizeof(uuid_bytes)));
    EXPECT_EQ(uuid.high(), 0x123456789ABCDEF0);
    EXPECT_EQ(uuid.low(), 0xFEDCBA9876543210);
}

TEST_F(UniqueIdTest, HexString) {
    DefaultId defaultId;
    EXPECT_TRUE(defaultId.setFromHex("1234567890ABCDEF"));
    EXPECT_EQ(defaultId.value(), 0x1234567890ABCDEF);
    EXPECT_EQ(defaultId.toString(), "1234567890ABCDEF");

    SmallId smallId;
    EXPECT_TRUE(smallId.setFromHex("1234"));
    EXPECT_EQ(smallId.value(), 0x1234);
    EXPECT_EQ(smallId.toString(), "1234");

    UUID uuid;
    EXPECT_TRUE(uuid.setFromHex("123456789ABCDEF0FEDCBA9876543210"));
    EXPECT_EQ(uuid.high(), 0x123456789ABCDEF0);
    EXPECT_EQ(uuid.low(), 0xFEDCBA9876543210);
    EXPECT_EQ(uuid.toString(), "123456789ABCDEF0-FEDC-BA98-7654-3210");

    // Test UUID with dashes
    EXPECT_TRUE(uuid.setFromHex("123456789ABCDEF0-FEDC-BA98-7654-3210"));
    EXPECT_EQ(uuid.high(), 0x123456789ABCDEF0);
    EXPECT_EQ(uuid.low(), 0xFEDCBA9876543210);
}

TEST_F(UniqueIdTest, Generation) {
    UUID uuid = UUID::generate();
    auto bytes = uuid.bytes();
    
    // Check version (4)
    EXPECT_EQ((bytes[6] & 0xF0), 0x40);
    
    // Check variant (2)
    EXPECT_EQ((bytes[8] & 0xC0), 0x80);

    // Generate multiple UUIDs and ensure they're different
    std::set<std::string> uuids;
    for (int i = 0; i < 1000; ++i) {
        uuids.insert(UUID::generate().toString());
    }
    EXPECT_EQ(uuids.size(), 1000);
}

TEST_F(UniqueIdTest, InvalidHexString) {
    DefaultId defaultId;
    EXPECT_FALSE(defaultId.setFromHex("123")); // Too short
    EXPECT_FALSE(defaultId.setFromHex("123456789")); // Too short
    EXPECT_FALSE(defaultId.setFromHex("123456789ABCDEFG")); // Invalid char
    EXPECT_FALSE(defaultId.setFromHex("GHIJKLMNOPQRSTUV")); // Invalid chars

    SmallId smallId;
    EXPECT_FALSE(smallId.setFromHex("123")); // Too short
    EXPECT_FALSE(smallId.setFromHex("12345")); // Too long
}

TEST_F(UniqueIdTest, Formatting) {
    // Test default formatting
    DefaultId defaultId(0x1234567890ABCDEF);
    EXPECT_EQ(std::format("{}", defaultId), "1234567890ABCDEF");
    EXPECT_EQ(std::format("{:x}", defaultId), "1234567890abcdef");
    EXPECT_EQ(std::format("{:X}", defaultId), "1234567890ABCDEF");
    
    // Test width and fill
    EXPECT_EQ(std::format("{:20}", defaultId), "  1234567890ABCDEF");
    EXPECT_EQ(std::format("{:<20}", defaultId), "1234567890ABCDEF  ");
    EXPECT_EQ(std::format("{:>20}", defaultId), "  1234567890ABCDEF");
    EXPECT_EQ(std::format("{:0>20}", defaultId), "001234567890ABCDEF");
    
    // Test small ID formatting
    SmallId smallId(0x1234);
    EXPECT_EQ(std::format("{}", smallId), "1234");
    EXPECT_EQ(std::format("{:x}", smallId), "1234");
    EXPECT_EQ(std::format("{:X}", smallId), "1234");
    EXPECT_EQ(std::format("{:08X}", smallId), "00001234");
}

TEST_F(UniqueIdTest, UUIDFormatting) {
    UUID uuid;
    uint8_t bytes[16] = {
        0x12, 0x3e, 0x45, 0x67,
        0xe8, 0x9b, 0x12, 0xd3,
        0xa4, 0x56, 0x42, 0x66,
        0x14, 0x17, 0x40, 0x00
    };
    uuid.setBytes(bytes, 16);

    // Test standard UUID format (8-4-4-4-12)
    EXPECT_EQ(std::format("{}", uuid), "123e4567-e89b-12d3-a456-426614174000");
    
    // Test uppercase
    EXPECT_EQ(std::format("{:X}", uuid), "123E4567-E89B-12D3-A456-426614174000");
    
    // Test without dashes
    EXPECT_EQ(std::format("{:-}", uuid), "123e4567e89b12d3a456426614174000");
    EXPECT_EQ(std::format("{:-X}", uuid), "123E4567E89B12D3A456426614174000");
    
    // Test width and alignment with dashes
    EXPECT_EQ(std::format("{:40}", uuid), "  123e4567-e89b-12d3-a456-426614174000");
    EXPECT_EQ(std::format("{:<40}", uuid), "123e4567-e89b-12d3-a456-426614174000  ");
    EXPECT_EQ(std::format("{:>40}", uuid), "  123e4567-e89b-12d3-a456-426614174000");
    
    // Test width and alignment without dashes
    EXPECT_EQ(std::format("{:-40}", uuid), "  123e4567e89b12d3a456426614174000");
    EXPECT_EQ(std::format("{:-<40}", uuid), "123e4567e89b12d3a456426614174000  ");
    EXPECT_EQ(std::format("{:->40}", uuid), "  123e4567e89b12d3a456426614174000");
    
    // Test zero padding
    EXPECT_EQ(std::format("{:0>40}", uuid), "00123e4567-e89b-12d3-a456-426614174000");
    EXPECT_EQ(std::format("{:-0>40}", uuid), "00123e4567e89b12d3a456426614174000");
}

TEST_F(UniqueIdTest, FormatInContainer) {
    std::vector<UUID> uuids;
    for (int i = 0; i < 3; ++i) {
        UUID uuid;
        uint8_t bytes[16] = {};
        bytes[0] = static_cast<uint8_t>(i + 1);
        uuid.setBytes(bytes, 16);
        uuids.push_back(uuid);
    }

    // Test formatting in containers
    std::string result;
    for (const auto& uuid : uuids) {
        if (!result.empty()) result += ", ";
        result += std::format("{:-<40X}", uuid);
    }

    EXPECT_FALSE(result.empty());
    for (const auto& uuid : uuids) {
        EXPECT_NE(result.find(std::format("{:-X}", uuid)), std::string::npos);
    }
}

TEST_F(UniqueIdTest, ComparisonOperators) {
    // Test integral IDs
    DefaultId id1(0x1234567890ABCDEF);
    DefaultId id2(0x1234567890ABCDEF);
    DefaultId id3(0x1234567890ABCDE0);

    EXPECT_EQ(id1, id2);
    EXPECT_NE(id1, id3);
    EXPECT_GT(id1, id3);
    EXPECT_GE(id1, id2);
    EXPECT_LT(id3, id1);
    EXPECT_LE(id2, id1);

    // Test UUIDs
    UUID uuid1(0x123456789ABCDEF0, 0xFEDCBA9876543210);
    UUID uuid2(0x123456789ABCDEF0, 0xFEDCBA9876543210);
    UUID uuid3(0x123456789ABCDEF0, 0xFEDCBA9876543211);
    UUID uuid4(0x123456789ABCDEF1, 0xFEDCBA9876543210);

    EXPECT_EQ(uuid1, uuid2);
    EXPECT_NE(uuid1, uuid3);
    EXPECT_NE(uuid1, uuid4);
    EXPECT_LT(uuid1, uuid3);
    EXPECT_LT(uuid1, uuid4);
    EXPECT_GT(uuid3, uuid1);
    EXPECT_GT(uuid4, uuid1);
    EXPECT_LE(uuid1, uuid2);
    EXPECT_GE(uuid2, uuid1);
}

TEST_F(UniqueIdTest, CopyAndMoveSemantics) {
    // Test copy construction and assignment
    DefaultId original(0x1234567890ABCDEF);
    DefaultId copied(original);
    EXPECT_EQ(original, copied);

    DefaultId assigned;
    assigned = original;
    EXPECT_EQ(original, assigned);

    // Test move construction and assignment
    DefaultId moved(std::move(copied));
    EXPECT_EQ(original, moved);

    DefaultId moveAssigned;
    moveAssigned = std::move(moved);
    EXPECT_EQ(original, moveAssigned);

    // Test with UUID
    UUID uuidOriginal(0x123456789ABCDEF0, 0xFEDCBA9876543210);
    UUID uuidCopied(uuidOriginal);
    EXPECT_EQ(uuidOriginal, uuidCopied);

    UUID uuidAssigned;
    uuidAssigned = uuidOriginal;
    EXPECT_EQ(uuidOriginal, uuidAssigned);

    UUID uuidMoved(std::move(uuidCopied));
    EXPECT_EQ(uuidOriginal, uuidMoved);

    UUID uuidMoveAssigned;
    uuidMoveAssigned = std::move(uuidMoved);
    EXPECT_EQ(uuidOriginal, uuidMoveAssigned);
}

TEST_F(UniqueIdTest, UUIDVersionAndVariant) {
    // Test multiple generated UUIDs for correct version and variant
    for (int i = 0; i < 100; ++i) {
        UUID uuid = UUID::generate();
        auto bytes = uuid.bytes();

        // Version should be 4 (0b0100xxxx)
        EXPECT_EQ(bytes[6] & 0xF0, 0x40) << "UUID version should be 4";

        // Variant should be 2 (0b10xxxxxx)
        EXPECT_EQ(bytes[8] & 0xC0, 0x80) << "UUID variant should be 2";

        // Test that reserved bits are properly set
        EXPECT_EQ(bytes[6] & 0xF0, 0x40) << "Version bits should be exactly 0x40";
        EXPECT_EQ(bytes[8] & 0xC0, 0x80) << "Variant bits should be exactly 0x80";
    }
}

TEST_F(UniqueIdTest, UUIDDistribution) {
    // Test the distribution of generated UUIDs
    constexpr int numTests = 10000;
    std::map<uint8_t, int> firstByteCount;
    std::map<uint8_t, int> lastByteCount;
    std::set<std::string> uniqueUUIDs;

    for (int i = 0; i < numTests; ++i) {
        UUID uuid = UUID::generate();
        auto bytes = uuid.bytes();
        firstByteCount[bytes[0]]++;
        lastByteCount[bytes[15]]++;
        uniqueUUIDs.insert(uuid.toString());
    }

    // Test uniqueness
    EXPECT_EQ(uniqueUUIDs.size(), numTests) << "All generated UUIDs should be unique";

    // Test distribution of first and last bytes
    const int expectedAvg = numTests / 256;
    const int allowedDeviation = expectedAvg / 2;

    for (const auto& [byte, count] : firstByteCount) {
        EXPECT_NEAR(count, expectedAvg, allowedDeviation)
            << "Byte distribution should be roughly uniform";
    }

    for (const auto& [byte, count] : lastByteCount) {
        EXPECT_NEAR(count, expectedAvg, allowedDeviation)
            << "Byte distribution should be roughly uniform";
    }
}

TEST_F(UniqueIdTest, EdgeCases) {
    // Test minimum and maximum values
    DefaultId minId(std::numeric_limits<uint64_t>::min());
    DefaultId maxId(std::numeric_limits<uint64_t>::max());
    EXPECT_LT(minId, maxId);
    EXPECT_EQ(minId.toString(), "0000000000000000");
    EXPECT_EQ(maxId.toString(), "FFFFFFFFFFFFFFFF");

    SmallId minSmallId(std::numeric_limits<uint16_t>::min());
    SmallId maxSmallId(std::numeric_limits<uint16_t>::max());
    EXPECT_LT(minSmallId, maxSmallId);
    EXPECT_EQ(minSmallId.toString(), "0000");
    EXPECT_EQ(maxSmallId.toString(), "FFFF");

    // Test zero UUID
    UUID zeroUuid(0, 0);
    EXPECT_EQ(zeroUuid.toString(), "00000000-0000-0000-0000-000000000000");

    // Test max UUID
    UUID maxUuid(UINT64_MAX, UINT64_MAX);
    EXPECT_EQ(maxUuid.toString(), "FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF");
}

TEST_F(UniqueIdTest, StringFormats) {
    UUID uuid(0x123456789ABCDEF0, 0xFEDCBA9876543210);

    // Test various string format combinations
    std::vector<std::pair<std::string, std::string>> formatTests = {
        {"{}", "123456789abcdef0-fedc-ba98-7654-3210"},
        {"{:X}", "123456789ABCDEF0-FEDC-BA98-7654-3210"},
        {"{:-}", "123456789abcdef0fedcba9876543210"},
        {"{:-X}", "123456789ABCDEF0FEDCBA9876543210"},
        {"{:40}", "  123456789abcdef0-fedc-ba98-7654-3210"},
        {"{:<40}", "123456789abcdef0-fedc-ba98-7654-3210  "},
        {"{:>40}", "  123456789abcdef0-fedc-ba98-7654-3210"},
        {"{:-40}", "  123456789abcdef0fedcba9876543210"},
        {"{:-<40}", "123456789abcdef0fedcba9876543210  "},
        {"{:->40}", "  123456789abcdef0fedcba9876543210"},
        {"{:0>40}", "00123456789abcdef0-fedc-ba98-7654-3210"},
        {"{:-0>40}", "00123456789abcdef0fedcba9876543210"}
    };

    for (const auto& [format, expected] : formatTests) {
        EXPECT_EQ(std::format(format, uuid), expected)
            << "Format '" << format << "' failed";
    }
}

TEST_F(UniqueIdTest, InvalidByteOperations) {
    DefaultId id;
    SmallId smallId;
    UUID uuid;

    // Test invalid byte array sizes
    uint8_t tooSmall[1] = {0x12};
    uint8_t tooLarge[32] = {};

    EXPECT_FALSE(id.setBytes(tooSmall, sizeof(tooSmall)));
    EXPECT_FALSE(id.setBytes(tooLarge, sizeof(tooLarge)));
    EXPECT_FALSE(smallId.setBytes(tooSmall, sizeof(tooSmall)));
    EXPECT_FALSE(smallId.setBytes(tooLarge, sizeof(tooLarge)));
    EXPECT_FALSE(uuid.setBytes(tooSmall, sizeof(tooSmall)));
    EXPECT_FALSE(uuid.setBytes(tooLarge, sizeof(tooLarge)));

    // Test null pointer
    EXPECT_FALSE(id.setBytes(nullptr, 8));
    EXPECT_FALSE(smallId.setBytes(nullptr, 2));
    EXPECT_FALSE(uuid.setBytes(nullptr, 16));
}

TEST_F(UniqueIdTest, ThreadSafety) {
    constexpr int numThreads = 4;
    constexpr int numIterations = 1000;
    std::vector<std::thread> threads;
    std::vector<std::set<std::string>> threadResults(numThreads);

    // Generate UUIDs in multiple threads
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, &threadResults]() {
            for (int j = 0; j < numIterations; ++j) {
                UUID uuid = UUID::generate();
                threadResults[i].insert(uuid.toString());
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify that all UUIDs are unique across all threads
    std::set<std::string> allUUIDs;
    for (const auto& threadSet : threadResults) {
        allUUIDs.insert(threadSet.begin(), threadSet.end());
    }

    EXPECT_EQ(allUUIDs.size(), numThreads * numIterations)
        << "All generated UUIDs should be unique across threads";
}

TEST_F(UniqueIdTest, Uniqueness) {
    constexpr size_t numIds = 1000;
    std::set<UUID> uuids;
    std::set<DefaultId> defaultIds;
    
    for (size_t i = 0; i < numIds; ++i) {
        auto uuid = UUID::generate();
        auto defaultId = DefaultId::generate();
        
        EXPECT_TRUE(uuids.insert(uuid).second) << "Generated UUID was not unique";
        EXPECT_TRUE(defaultIds.insert(defaultId).second) << "Generated DefaultId was not unique";
    }
}

TEST_F(UniqueIdTest, Comparison) {
    UUIDType raw1{}, raw2{};
    raw1[0] = 1;
    raw2[0] = 2;
    
    UUID id1(raw1);
    UUID id2(raw2);
    UUID id1_copy(raw1);

    EXPECT_LT(id1, id2);
    EXPECT_GT(id2, id1);
    EXPECT_EQ(id1, id1_copy);
    EXPECT_NE(id1, id2);
}

TEST_F(UniqueIdTest, SetValue) {
    // Test integral type
    DefaultId defaultId;
    defaultId.setValue(0x1234567890ABCDEF);
    EXPECT_EQ(defaultId.value(), 0x1234567890ABCDEF);

    // Test byte array type
    UUID uuid;
    UUIDType bytes{};
    bytes[0] = 0x12;
    bytes[1] = 0x34;
    uuid.setValue(bytes);
    EXPECT_EQ(uuid.value(), bytes);
}

TEST_F(UniqueIdTest, SetBytes) {
    UUID uuid;
    uint8_t bytes[16] = {
        0x12, 0x34, 0x56, 0x78,
        0x90, 0xAB, 0xCD, 0xEF,
        0x12, 0x34, 0x56, 0x78,
        0x90, 0xAB, 0xCD, 0xEF
    };

    EXPECT_TRUE(uuid.setBytes(bytes, 16));
    auto result = uuid.bytes();
    EXPECT_TRUE(std::equal(std::begin(bytes), std::end(bytes), result.begin()));

    // Test with wrong size
    EXPECT_FALSE(uuid.setBytes(bytes, 15));
}

TEST_F(UniqueIdTest, SetFromHex) {
    // Test UUID format
    UUID uuid;
    EXPECT_TRUE(uuid.setFromHex("123e4567-e89b-12d3-a456-426614174000"));
    EXPECT_TRUE(uuid.setFromHex("123E4567E89B12D3A456426614174000")); // Without dashes

    auto str = uuid.toString();
    EXPECT_EQ(str.length(), 36); // Including dashes

    // Test invalid formats
    EXPECT_FALSE(uuid.setFromHex("invalid"));
    EXPECT_FALSE(uuid.setFromHex("123E4567E89B12D3A456")); // Too short
    EXPECT_FALSE(uuid.setFromHex("123E4567E89B12D3A45642661417400000")); // Too long
    EXPECT_FALSE(uuid.setFromHex("123E4567E89B12D3A456426614174ZZZ")); // Invalid hex

    // Test small ID
    SmallId smallId;
    EXPECT_TRUE(smallId.setFromHex("1234"));
    EXPECT_EQ(smallId.toString(), "1234");
    EXPECT_FALSE(smallId.setFromHex("123")); // Too short
    EXPECT_FALSE(smallId.setFromHex("12345")); // Too long
}
