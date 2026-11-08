#include "test.h"
#include "test_data.h"
#include <map>
#include "client/net_client.h"
#include "server/systems/map_pickups.h"

using namespace flix;

TEST(pyramid_maps_spawn_on_safe_ground_and_refill_without_growing_unbounded) {
    const auto& content = testsupport::shippedContent();
    World world;
    WorldMaps maps;
    Terrain terrain;
    std::string error;
    if (!maps.load(testsupport::dataDir(), &terrain, error)) { CHECK(false); return; }
    SpawnSystem spawning;
    spawning.worldMaps = &maps;
    LootSystem loot;
    PyramidMapPickups charts;
    Rng rng{1};
    CommandBuffer commands{world};
    spawning.run(world, terrain, content, {}, rng, 0.0, 0.0, commands);
    commands.flush();
    const auto chart = content.petalIndex("pyramid_map");
    CHECK(chart != kInvalidIndex);
    charts.run(world, maps, terrain, spawning, loot, content, 0.0);
    std::vector<Entity> drops;
    Query<DropItem, Transform, Lifetime> query{world};
    std::map<Realm, int> counts;
    query.each([&](Entity e, DropItem& item, Transform& at, Lifetime& lifetime) {
        CHECK_EQ(item.configIndex, chart);
        CHECK(item.singleCopy && item.humansOnly && item.eligible.empty());
        CHECK_EQ(item.rarity, Rarity::Rare);
        CHECK_NEAR(lifetime.remainingSeconds, 600.0, 0.001);
        const auto* map = maps.forRealm(at.realm);
        CHECK(map && map->templateId() == "pyramid" && !map->showsMinimap());
        CHECK(!terrain.blocked(at.position, at.realm));
        CHECK(distanceSq(at.position, terrain.resolveCircle(at.position, kDropWallRadius, at.realm)) <= 1.0);
        ++counts[at.realm];
        drops.push_back(e);
    });
    const auto pyramids = std::count_if(maps.maps().begin(), maps.maps().end(),
                                       [](const MapData& map) { return map.templateId() == "pyramid"; });
    CHECK_EQ(counts.size(), static_cast<std::size_t>(pyramids));
    for (const auto& count : counts) CHECK_EQ(count.second, PyramidMapPickups::kTarget);
    CHECK(!drops.empty());
    if (drops.empty()) return;
    const std::size_t total = drops.size();
    world.destroy(drops.front());
    charts.run(world, maps, terrain, spawning, loot, content, 29999.0);
    CHECK_EQ(query.count(), total - 1);
    charts.run(world, maps, terrain, spawning, loot, content, 30000.0);
    CHECK_EQ(query.count(), total);
    charts.run(world, maps, terrain, spawning, loot, content, 60000.0);
    CHECK_EQ(query.count(), total);
}

TEST(pyramid_map_pickups_ignore_bots_and_other_realms_and_pay_one_human_once) {
    const auto& content = testsupport::shippedContent();
    const auto chart = content.petalIndex("pyramid_map");
    CHECK(chart != kInvalidIndex);
    World world;
    LootSystem loot;
    Rng rng{1};
    SpatialGrid grid;
    EventQueue events;
    CommandBuffer commands{world};
    const Realm realm = worldRealm(3);
    const Vec2 at{2000, 2000};
    const Entity drop = loot.spawnDrop(world, chart, Rarity::Rare, at, realm, {});
    auto& item = world.get<DropItem>(drop);
    item.singleCopy = true;
    item.humansOnly = true;
    const Entity player = world.create();
    world.add<PlayerTag>(player);
    world.add<Transform>(player, Transform{at, 0.0, realm});
    world.add<PlayerModifiers>(player);
    grid.insert(drop, realm, at, kDropBodyRadius);
    auto tick = [&] {
        loot.run(world, grid, content, rng, 0.0, 0.0, commands, events);
        commands.flush();
    };
    tick();
    CHECK(loot.pickups().empty());
    PlayerAccount account;
    account.connection = 123;
    world.add<PlayerAccount>(player, account);
    world.get<Transform>(player).realm = Realm::Overworld;
    tick();
    CHECK(loot.pickups().empty());
    world.get<Transform>(player).realm = realm;
    tick();
    CHECK_EQ(loot.pickups().size(), std::size_t(1));
    CHECK(!world.isAlive(drop));
    tick();
    CHECK(loot.pickups().empty());
}

TEST(pyramid_map_is_owned_from_bag_or_bar_and_cannot_drop_from_random_mobs) {
    const auto& content = testsupport::shippedContent();
    const auto chart = content.petalIndex("pyramid_map");
    CHECK(chart != kInvalidIndex);
    Profile profile;
    CHECK(!profile.ownsPetal(chart));
    CHECK(!profile.ownsPetal(kInvalidIndex));
    profile.inventory.push_back({chart, Rarity::Rare, 1});
    CHECK(profile.ownsPetal(chart));
    profile.inventory[0].count = 0;
    CHECK(!profile.ownsPetal(chart));
    profile.loadout.push_back({chart, Rarity::Rare});
    CHECK(profile.ownsPetal(chart));
    CHECK(petalHasOwnSource(content.petal(chart)));
    DropTables tables;
    std::string error;
    CHECK(tables.load(content, testsupport::dataFile("mob_drops.json"), error));
    for (auto petal : tables.droppablePetals()) CHECK(petal != chart);
}
