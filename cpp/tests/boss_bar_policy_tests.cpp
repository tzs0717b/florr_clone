#include "test.h"
#include "client/boss_bar_policy.h"
#include <limits>

using namespace flix;

TEST(full_health_super_crowds_do_not_fill_the_top_hud) {
    std::vector<RemoteEntity> mobs(100);
    std::vector<const RemoteEntity*> bars;
    for (auto& mob : mobs) {
        mob.rarity = Rarity::Super;
        mob.healthFraction = 1.0;
        bars.push_back(&mob);
    }
    compactBossBars(bars, false, 1920, 1080);
    CHECK(bars.empty());
}

TEST(injured_super_crowds_have_two_stable_top_rows_at_most) {
    std::vector<RemoteEntity> mobs(100);
    std::vector<const RemoteEntity*> bars;
    for (std::size_t i = 0; i < mobs.size(); ++i) {
        mobs[i].rarity = Rarity::Super;
        mobs[i].healthFraction = 0.5;
        mobs[i].netId = static_cast<std::uint32_t>(100 - i);
        bars.push_back(&mobs[i]);
    }
    compactBossBars(bars, false, 1920, 1080);
    CHECK_EQ(bars.size(), std::size_t(2));
    CHECK_EQ(bars[0]->netId, 1u);
    CHECK_EQ(bars[1]->netId, 2u);
    CHECK_NEAR(mobs[0].healthFraction, 0.5, 0.001);
}

TEST(rare_bosses_take_priority_even_at_full_health_and_small_screens_have_one_row) {
    RemoteEntity super, eternal, apex;
    super.rarity = Rarity::Super; super.healthFraction = 0.2; super.netId = 1;
    eternal.rarity = Rarity::Unique; eternal.healthFraction = 1.0; eternal.netId = 2;
    apex.rarity = Rarity::Apex; apex.healthFraction = 1.0; apex.netId = 3;
    std::vector<const RemoteEntity*> bars{&super, &eternal, &apex};
    compactBossBars(bars, false, 1920, 1080);
    CHECK_EQ(bars.size(), std::size_t(2));
    CHECK(bars[0] == &apex && bars[1] == &eternal);
    bars = {&super, &eternal, &apex};
    compactBossBars(bars, false, 390, 844);
    CHECK_EQ(bars.size(), std::size_t(1));
    CHECK(bars[0] == &apex);
    bars = {&super, &eternal};
    compactBossBars(bars, false, 1280, 600);
    CHECK_EQ(bars.size(), std::size_t(1));
    CHECK(bars[0] == &eternal);
}

TEST(dead_mobs_and_prestige_supers_do_not_claim_top_bars) {
    CHECK(!needsTopBossBar(Rarity::Super, 0.5, true));
    CHECK(!needsTopBossBar(Rarity::Unique, 0.0, false));
    CHECK(!needsTopBossBar(Rarity::Apex, std::numeric_limits<double>::quiet_NaN(), false));
    CHECK(!needsTopBossBar(Rarity::Ultra, 0.5, false));
    CHECK(needsTopBossBar(Rarity::Super, 0.5, false));
    CHECK(needsTopBossBar(Rarity::Unique, 1.0, false));
}
