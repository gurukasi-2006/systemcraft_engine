/**
 * @file test_ecs.cpp
 * @brief Catch2 unit tests for the Core ECS subsystems (1-5).
 */

#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>
#include <string>

// Include our 5 Core Subsystems
#include "../core/entity_manager.hpp"
#include "../core/component_registry.hpp"
#include "../core/component_pool.hpp"
#include "../core/ecs_view.hpp"
#include "../core/archetype_manager.hpp"
#include "../core/component_handle.hpp"
#include "../core/tag_manager.hpp"
#include "../core/hierarchy_manager.hpp"
#include "../core/component_serializer.hpp"
#include "../core/ecs_world.hpp"

// --- Dummy Data for Testing ---
struct Position { float x, y; };
struct Health { int current_hp; };

TEST_CASE("Core ECS Integration Test", "[ecs]") {
    // 1. World Setup: Boot up the master registry and link the managers
    entt::registry master_registry;
    EntityManager entity_manager(master_registry);
    ArchetypeManager archetype_manager(entity_manager);

    ComponentPool<Position> position_pool(master_registry);
    ComponentPool<Health> health_pool(master_registry);

    SECTION("Subsystem 1: Entities can be created and destroyed") {
        EntityID e1 = entity_manager.createEntity();
        REQUIRE(entity_manager.isValid(e1) == true);

        entity_manager.destroyEntity(e1);
        REQUIRE(entity_manager.isValid(e1) == false);
    }

    SECTION("Subsystem 2: Component Registry assigns unique IDs") {
        uint32_t pos_id_1 = ComponentRegistry::getTypeID<Position>();
        uint32_t pos_id_2 = ComponentRegistry::getTypeID<Position>();
        uint32_t health_id = ComponentRegistry::getTypeID<Health>();

        // Type IDs should be persistent for the same type, but different across types
        REQUIRE(pos_id_1 == pos_id_2);
        REQUIRE(pos_id_1 != health_id);
    }

    SECTION("Subsystem 3: Component Pools attach, read, and remove data") {
        EntityID e2 = entity_manager.createEntity();

        // Insert and verify
        position_pool.insert(e2, Position{10.5f, 20.0f});
        REQUIRE(position_pool.has(e2) == true);
        REQUIRE(position_pool.get(e2).x == 10.5f);
        REQUIRE(position_pool.get(e2).y == 20.0f);

        // Remove and verify
        position_pool.remove(e2);
        REQUIRE(position_pool.has(e2) == false);
    }

    SECTION("Subsystem 4: ECS View iterates over matching components") {
        ECSView<Position, Health> view(master_registry);

        // Create two valid entities with both components
        EntityID e3 = entity_manager.createEntity();
        position_pool.insert(e3, Position{0.0f, 0.0f});
        health_pool.insert(e3, Health{100});

        EntityID e4 = entity_manager.createEntity();
        position_pool.insert(e4, Position{5.0f, 5.0f});
        health_pool.insert(e4, Health{50});

        // Create a dud entity with only one component (should be ignored by view)
        EntityID dud = entity_manager.createEntity();
        position_pool.insert(dud, Position{99.0f, 99.0f});

        int match_count = 0;

        // Run the view! It should only hit e3 and e4.
        view.each([&match_count](EntityID id, Position& pos, Health& hp) {
            match_count++;
            pos.x += 1.0f; // Modify data safely
        });

        REQUIRE(match_count == 2);
        REQUIRE(position_pool.get(e3).x == 1.0f);
        REQUIRE(position_pool.get(e4).x == 6.0f);
    }

    SECTION("Subsystem 5: Archetype Manager registers and spawns prefabs") {
        // Register a "Citizen" recipe that gets Health and Position automatically
        archetype_manager.registerArchetype("Citizen", [&](EntityID id) {
            position_pool.insert(id, Position{0.0f, 0.0f});
            health_pool.insert(id, Health{100});
        });

        // Spawn using the recipe
        EntityID citizen = archetype_manager.spawn("Citizen");

        // Verify the archetype manager properly attached the components
        REQUIRE(entity_manager.isValid(citizen) == true);
        REQUIRE(position_pool.has(citizen) == true);
        REQUIRE(health_pool.has(citizen) == true);
        REQUIRE(health_pool.get(citizen).current_hp == 100);
    }

    SECTION("Subsystem 6: Component Handle safely accesses data") {
        EntityID e5 = entity_manager.createEntity();
        position_pool.insert(e5, Position{50.0f, 60.0f});

        // Create a safe handle pointing to this entity's Position data
        ComponentHandle<Position> pos_handle(e5, master_registry);

        // 1. Verify it knows the data exists
        REQUIRE(pos_handle.isValid() == true);

        // 2. Verify we can read data using .get()
        REQUIRE(pos_handle.get().x == 50.0f);

        // 3. Verify we can write data safely using the -> pointer override
        pos_handle->y = 100.0f;
        REQUIRE(position_pool.get(e5).y == 100.0f);

        // 4. Verify it safely detects when the underlying data is destroyed!
        position_pool.remove(e5);
        REQUIRE(pos_handle.isValid() == false);
    }

    SECTION("Subsystem 7: Tag Manager assigns and retrieves unique tags") {
        TagManager tag_manager;
        EntityID player_id = entity_manager.createEntity();
        EntityID enemy_id = entity_manager.createEntity();

        // 1. Assign a tag and verify we can retrieve the exact entity
        tag_manager.tagEntity(player_id, "Player");
        REQUIRE(tag_manager.hasTag(player_id) == true);
        REQUIRE(tag_manager.getEntityByTag("Player").raw_id == player_id.raw_id);

        // 2. Test "Stealing" the tag (Safety Check)
        // If we tag the enemy as "Player", the original player should lose it automatically
        tag_manager.tagEntity(enemy_id, "Player");
        REQUIRE(tag_manager.hasTag(player_id) == false);
        REQUIRE(tag_manager.hasTag(enemy_id) == true);
        REQUIRE(tag_manager.getEntityByTag("Player").raw_id == enemy_id.raw_id);

        // 3. Test safe removal
        tag_manager.removeTag(enemy_id);
        REQUIRE(tag_manager.hasTag(enemy_id) == false);

        // When searching for a deleted tag, it should return EnTT's null identifier
        REQUIRE(tag_manager.getEntityByTag("Player").raw_id == static_cast<uint32_t>(entt::null));
    }

    SECTION("Subsystem 8: Hierarchy Manager links parents and children") {
        HierarchyManager hierarchy_manager(master_registry);

        EntityID parent = entity_manager.createEntity();
        EntityID child1 = entity_manager.createEntity();
        EntityID child2 = entity_manager.createEntity();

        // 1. Link children to the parent
        hierarchy_manager.addChild(parent, child1);
        hierarchy_manager.addChild(parent, child2);

        // Verify the children know who their parent is
        REQUIRE(hierarchy_manager.getParent(child1).raw_id == parent.raw_id);
        REQUIRE(hierarchy_manager.getParent(child2).raw_id == parent.raw_id);

        // Verify the parent knows it has exactly 2 children
        auto children = hierarchy_manager.getChildren(parent);
        REQUIRE(children.size() == 2);

        // 2. Safely remove a child and test the Erase-Remove math
        hierarchy_manager.removeParent(child1);

        // Child1 should now return an invalid/null ID for its parent
        REQUIRE(hierarchy_manager.getParent(child1).raw_id == static_cast<uint32_t>(entt::null));

        // The parent should now only have 1 child left in its vector
        children = hierarchy_manager.getChildren(parent);
        REQUIRE(children.size() == 1);
        REQUIRE(children[0].raw_id == child2.raw_id);
    }

    SECTION("Subsystem 9: Component Serializer saves and loads data") {
        ComponentSerializer serializer;

        // 1. Teach the manager how to save and load a "Position"
        serializer.registerComponent("Position",
            // SAVE LOGIC: Convert floats to string "x,y"
            [](entt::registry& reg, EntityID e) -> std::string {
                auto ent = static_cast<entt::entity>(e.raw_id);
                if (reg.all_of<Position>(ent)) {
                    auto& pos = reg.get<Position>(ent);
                    return std::to_string(pos.x) + "," + std::to_string(pos.y);
                }
                return ""; // Return empty if the entity doesn't have a Position
            },
            // LOAD LOGIC: Parse string "x,y" back into floats
            [](entt::registry& reg, EntityID e, const std::string& data) {
                auto ent = static_cast<entt::entity>(e.raw_id);
                size_t comma = data.find(',');
                if (comma != std::string::npos) {
                    float x = std::stof(data.substr(0, comma));
                    float y = std::stof(data.substr(comma + 1));
                    reg.emplace_or_replace<Position>(ent, x, y);
                }
            }
        );

        // 2. Create Entity A with specific data
        EntityID e_save = entity_manager.createEntity();
        master_registry.emplace<Position>(static_cast<entt::entity>(e_save.raw_id), 15.5f, 42.0f);

        // 3. Serialize Entity A into a string
        std::string save_data = serializer.serializeEntity(master_registry, e_save);

        // Ensure the string formatted correctly (std::to_string usually adds decimals)
        REQUIRE(save_data.find("[Position]:") != std::string::npos);
        REQUIRE(save_data.find("15.5") != std::string::npos);

        // 4. Create an empty Entity B and deserialize the string into it
        EntityID e_load = entity_manager.createEntity();
        serializer.deserializeEntity(master_registry, e_load, save_data);

        // 5. Verify Entity B perfectly absorbed the data!
        REQUIRE(master_registry.all_of<Position>(static_cast<entt::entity>(e_load.raw_id)) == true);
        REQUIRE(master_registry.get<Position>(static_cast<entt::entity>(e_load.raw_id)).x == 15.5f);
        REQUIRE(master_registry.get<Position>(static_cast<entt::entity>(e_load.raw_id)).y == 42.0f);
    }

    SECTION("Subsystem 10: ECS World acts as the supreme container") {
        // Instantiate the God Object
        ECSWorld world;

        // 1. Test that the internal entity manager works
        EntityID parent = world.entity_manager.createEntity();
        REQUIRE(world.entity_manager.isValid(parent) == true);

        // 2. Test that we can interact directly with the world's registry
        world.registry.emplace<Position>(static_cast<entt::entity>(parent.raw_id), 10.0f, 20.0f);
        REQUIRE(world.registry.all_of<Position>(static_cast<entt::entity>(parent.raw_id)) == true);

        // 3. Test that the hierarchy manager is properly linked to the same registry
        EntityID child = world.entity_manager.createEntity();
        world.hierarchy_manager.addChild(parent, child);
        REQUIRE(world.hierarchy_manager.getParent(child).raw_id == parent.raw_id);
    }
}