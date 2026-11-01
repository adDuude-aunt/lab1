#include "journal.hpp"
#include <gtest/gtest.h>
#include <filesystem>
using namespace warehouse;

TEST(EventJournalTest, LogAndClear) {
    EventJournal j;
    j.log(1, "Order", "Created");
    EXPECT_EQ(j.events().size(), 1);
    j.clear();
    EXPECT_TRUE(j.events().empty());
}

TEST(EventJournalTest, SaveLoad) {
    EventJournal j1;
    j1.log(1, "Move", "Robot 1 moved");
    j1.log(2, "Load", "Robot 1 loaded 5");
    j1.save("test_journal.txt");

    EventJournal j2;
    j2.load("test_journal.txt");
    EXPECT_EQ(j2.events().size(), 2);
    EXPECT_EQ(j2.events()[0].description, "Robot 1 moved");
    std::filesystem::remove("test_journal.txt");
}