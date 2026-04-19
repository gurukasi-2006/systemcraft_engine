#pragma once

#include <entt/entt.hpp>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../08_Citizencomponent/daily_schedule_component.hpp"
#include "../08_Citizencomponent/employment_component.hpp"
#include "../08_Citizencomponent/housing_component.hpp"
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file daily_routine_executor.hpp
 * @brief Subsystem 108: Executes schedule offsets and triggers physical location changes.
 */

namespace Population {

    /**
     * @enum ActivityState
     * @brief The current spatial and logical action the citizen is performing.
     */
    enum class ActivityState : uint8_t {
        Home = 0,
        CommutingToWork,
        Working,
        Shopping,
        CommutingHome
    };

    /**
     * @struct RoutineStateComponent
     * @brief Holds the real-time execution state of the citizen's daily routine.
     */
    struct RoutineStateComponent {
        ActivityState current_state{ActivityState::Home};
        uint8_t current_need_score{100}; // Drives the post-work shopping trigger
    };
}

class DailyRoutineExecutor {
public:
    /**
     * @brief Steps the daily schedule for all citizens, updating their states and locations.
     * @param world The master ECS world.
     * @param current_tick The absolute simulation time.
     */
    void update(ECSWorld& world, uint64_t current_tick) {
        // Assume a standard 24-hour tick cycle for schedule execution
        uint8_t tick_of_day = static_cast<uint8_t>(current_tick % 24);

        // Query all citizens with a schedule, a routine state, and a physical body
        auto view = world.registry.view<
            Population::DailyScheduleComponent,
            Population::RoutineStateComponent,
            PositionComponent,
            Population::EmploymentComponent,
            Population::HousingComponent
        >();

        for (auto raw_id : view) {
            auto& schedule = view.get<Population::DailyScheduleComponent>(raw_id);
            auto& routine = view.get<Population::RoutineStateComponent>(raw_id);
            auto& pos = view.get<PositionComponent>(raw_id);

            auto& emp = view.get<Population::EmploymentComponent>(raw_id);
            auto& housing = view.get<Population::HousingComponent>(raw_id);

            // --- STATE MACHINE ---

            if (tick_of_day == schedule.wake_tick) {
                // Wake up and head out
                routine.current_state = Population::ActivityState::CommutingToWork;
            }
            else if (tick_of_day == schedule.work_start_tick) {
                // Arrive at work
                routine.current_state = Population::ActivityState::Working;

                // Move Entity to Employer Tile
                entt::entity emp_ent = static_cast<entt::entity>(emp.employer.raw_id);
                if (world.registry.valid(emp_ent) && world.registry.all_of<PositionComponent>(emp_ent)) {
                    pos.coord = world.registry.get<PositionComponent>(emp_ent).coord;
                    // Note: In a full engine, you would also call spatial_index.move(raw_id, old_pos, new_pos) here!
                }
            }
            else if (tick_of_day == schedule.work_end_tick) {
                // Clock out: Go shopping or go home?
                if (routine.current_need_score < 40) {
                    routine.current_state = Population::ActivityState::Shopping;
                } else {
                    routine.current_state = Population::ActivityState::CommutingHome;
                }
            }
            else if (tick_of_day == schedule.sleep_tick) {
                // Arrive back home to sleep
                routine.current_state = Population::ActivityState::Home;

                // Move Entity to Housing Tile
                entt::entity home_ent = static_cast<entt::entity>(housing.housing_entity.raw_id); // Assuming HousingComponent uses `home_building`
                if (world.registry.valid(home_ent) && world.registry.all_of<PositionComponent>(home_ent)) {
                    pos.coord = world.registry.get<PositionComponent>(home_ent).coord;
                }
            }
        }
    }
};