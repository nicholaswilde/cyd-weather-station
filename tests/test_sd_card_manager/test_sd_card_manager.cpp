#include <unity.h>
#include "sd_card_manager.h"
#include "../mocks/mocks.cpp"
#include "../../src/sd_card_manager.cpp"

void setUp(void) {
    mock_sd_card_present = true;
    mock_sd_card_mounted = false;
    mock_files.clear();
}

void tearDown(void) {
    SdCardManager::end();
}

void test_sd_card_manager_mount_success(void) {
    // With card present, begin() should return true and set mounted state
    mock_sd_card_present = true;
    TEST_ASSERT_TRUE(SdCardManager::begin());
    TEST_ASSERT_TRUE(SdCardManager::isMounted());
    TEST_ASSERT_TRUE(SdCardManager::isCardPresent());
}

void test_sd_card_manager_mount_no_card(void) {
    // Without card present, begin() should return false and not be mounted
    mock_sd_card_present = false;
    TEST_ASSERT_FALSE(SdCardManager::begin());
    TEST_ASSERT_FALSE(SdCardManager::isMounted());
    TEST_ASSERT_FALSE(SdCardManager::isCardPresent());
}

void test_sd_card_manager_reinitialization(void) {
    // Test that if we fail to mount first (no card), then insert card, it mounts
    mock_sd_card_present = false;
    TEST_ASSERT_FALSE(SdCardManager::begin());
    TEST_ASSERT_FALSE(SdCardManager::isCardPresent());
    
    mock_sd_card_present = true;
    TEST_ASSERT_TRUE(SdCardManager::begin());
    TEST_ASSERT_TRUE(SdCardManager::isMounted());
    TEST_ASSERT_TRUE(SdCardManager::isCardPresent());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_sd_card_manager_mount_success);
    RUN_TEST(test_sd_card_manager_mount_no_card);
    RUN_TEST(test_sd_card_manager_reinitialization);
    return UNITY_END();
}
