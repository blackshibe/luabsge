#include "lua_instance.h"
#include "util/output.h"
#include <stdexcept>
#include <string>
#include <vector>

static Output output;

namespace Lua::instance {

	Scene::ecs::Instance &get_instance(entt::registry *registry, entt::entity entity) {
		if (!registry->any_of<Scene::ecs::Instance>(entity)) {
			throw std::runtime_error("ECS entity has no Instance?");
		}

		return registry->get<Scene::ecs::Instance>(entity);
	}

	/*
	 * @namespace Instance
	 * This is an autodoc tool test
	 */
	void init(EngineInstance &engine, sol::state &lua) {
		registry = &engine.registry;

		lua.new_usertype<Instance>("Instance",
		                           /*
		                            * @field new
		                            * @type function
		                            * @param name string
		                            * Creates a new blank instance with the provided name.
		                            */
		                           sol::constructors<Instance(std::string)>(),

		                           /*
		                            * @field parent
		                            * @type field
		                            * Parent of entity.
		                            */
		                           "parent",
		                           sol::property(
		                               [](Instance &i) -> entt::entity {
			                               return get_instance(registry, i.entity).parent;
		                               },
		                               [](Instance &i, Instance parent) {
			                               get_instance(registry, i.entity).parent = parent.entity;
		                               }),

		                           /*
		                            * @field name
		                            * @type field
		                            * Name of entity.
		                            */
		                           "name",
		                           sol::property(
		                               [](Instance &i) -> std::string {
			                               return get_instance(registry, i.entity).name;
		                               },
		                               [](Instance &i, std::string name) {
			                               get_instance(registry, i.entity).name = name;
		                               }),

		                           /*
		                            * @field children
		                            * @type field Instance[]
		                            * Instances parented to this one.
		                            */
		                           "children",
		                           sol::property([](Instance &i) -> std::vector<Instance> {
			                           std::vector<Instance> children;

			                           for (entt::entity entity : registry->view<Scene::ecs::Instance>()) {
				                           if (registry->get<Scene::ecs::Instance>(entity).parent == i.entity) {
					                           children.emplace_back(entity);
				                           }
			                           }

			                           return children;
		                           })

		);
	}

	Lua::instance::Instance::Instance(std::string name) {
		output.mark();

		entt::entity entity = registry->create();
		registry->emplace<Scene::ecs::Instance>(entity);
		this->entity = entity;
	};

	Lua::instance::Instance::Instance(entt::entity entity) {
		this->entity = entity;
	};
}
